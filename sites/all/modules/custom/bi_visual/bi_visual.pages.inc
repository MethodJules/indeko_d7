<?php
/**
 * Created by PhpStorm.
 * User: mende
 * Date: 2/9/2018
 * Time: 11:10 AM
 */

/**
 * @file
 * User page callbacks for the bi_visual module.
 */


/**
 * Page callback: Displays analysis of a taxonomy vocabulary.
 *
 * @param $vid
 *   The ID of the taxonomy vocabulary to analyse.
 *
 * @return string
 *   The rendered HTML code of the analysis.
 */
function bi_visual($vid) {
  $output = '';
  $fields = get_taxonomy_fields_on_content_type();

  // display detailed analysis of a vocabulary
  if ($fields[$vid]) {

    // get all taxonomy terms of the given vocabulary
    $tids = entity_load('taxonomy_term', FALSE, array('vid' => $vid));

    $chartData = array();
    $tableRows = array();

    foreach($tids as $tid => $info) {
      $rowArray = array();
      $noNodes = count(taxonomy_select_nodes($tid, FALSE));

      // show link in table for further drilldown only if there are > 0 results
      if($noNodes > 0) {
        $rowArray['name'] = l($info->name, "d3/bi/" . $tid) ;
      } else {
        $rowArray['name'] = check_plain($info->name) ;
      }
      $rowArray['count'] = $noNodes;
      $tableRows[] = $rowArray;

      // don't display 0% entries on pie D3 chart
      if ($noNodes > 0) {
        $chartData[] = array(check_plain($info->name), $noNodes, $tid);
      }
    }

    // list of all vocabularies available for analysis
    $links = array();
    foreach ($fields as $vocId => $info) {
      $links[] = l(check_plain($info->name), 'd3/bi_visual/'.$vocId);
    }

    $type_of_visualization = array(
      '#theme' => 'item_list',
      '#title' => 'Analysen',
      '#items' => $links,
      '#attributes' => array('class' => array('bi')),
    );

    // TODO platzsparenderes Design bis Aussehen geklärt
    drupal_add_css('ul.bi {display:flex; flex-wrap: wrap;}', 'inline');

    $title = array(
      '#type' => 'item',
      '#title' => $fields[$vid]->name,
    );

    // render html output
    $output = render($type_of_visualization);
    $output .= render($title);
    $output .= _bi_visual_render_chart($chartData);
    $output .= _bi_visual_render_table($tableRows);
  }

  return $output;
}


/**
 * Page callback: Displays analysis of a taxonomy term.
 *
 * @param $tid
 *   The ID of the taxonomy term to analyse.
 *
 * @return string
 *   The rendered HTML code of the analysis.
 */
function bi_drilldown($tid) {
  $output = '';

  if (is_int((int)$tid)) {

    // use term tid instead of term name to get the correct results; e.g. term 'Sonstige' or 'Keine Angabe' are part of
    // several vocabularies.
    $sql_query = "SELECT title, node.nid FROM ({node} INNER JOIN {taxonomy_index} ON node.nid = taxonomy_index.nid)
                INNER JOIN {taxonomy_term_data} ON taxonomy_index.tid = taxonomy_term_data.tid
                WHERE taxonomy_term_data.tid = :tid
                ORDER BY title";
    $result = db_query($sql_query, array(':tid' => $tid));
    $result = $result->fetchAll();

    $data = array();
    foreach ($result as $line) {
      $data[] = array(check_plain($line->title), 1, $line->nid,
      );
    }

    $tableRows = array();

    foreach ($data as $entry) {
      $rowArray = array();
      $rowArray['name'] = l($entry[0], drupal_get_path_alias('node/' . (int) $entry[2]));
      $rowArray['count'] = $entry[1];
      $tableRows[] = $rowArray;
    }

    $term = taxonomy_term_load($tid);
    $vocabulary = taxonomy_vocabulary_load($term->vid);

    $title = array(
      '#type' => 'item',
      '#title' => $vocabulary->name . ' - ' . $term->name,
    );

    // render html output
    $output = render($title);
    $output .= _bi_visual_render_chart($data);
    $output .= _bi_visual_render_table($tableRows);
  }
  return $output;
}


/**
 * Returns all taxonomy term reference fields on the given content type (or on all content types if no $types is specified).
 * The db field name, associated taxonomy vocabulary, db field table name and db field tid column are returned.
 *
 * @param $type
 *   The content type machine name.
 *
 * @return array
 *   Array of taxonomy term reference fields.
 */
function get_taxonomy_fields_on_content_type($type = array()) {
  $fieldsTaxonomy = array();

  // array of all fields and content types they are associated with
  $fieldMap = field_info_field_map();

  // all available vocabularies sorted by weight first, alphabetically second
  $vocabularies = taxonomy_get_vocabularies();

  // filters all taxonomy reference fields on nodes of content type $type
  $bundle = 'node';
  foreach ($fieldMap as $field => $info) {
    if ($info['type'] === 'taxonomy_term_reference' && array_key_exists($bundle, $info['bundles']) &&
      (in_array($type, $info['bundles'][$bundle]) || $type === array())) {
      $fieldInfo = field_info_field($field);
      $fieldTableName = key($fieldInfo['storage']['details']['sql']['FIELD_LOAD_CURRENT']);
      $vocabulary = taxonomy_vocabulary_machine_name_load($fieldInfo['settings']['allowed_values'][0]['vocabulary']);
      $vid = $vocabulary->vid;

      $vocabularies[$vid]->field_name = $field;
      $vocabularies[$vid]->table_name = $fieldTableName;
      $vocabularies[$vid]->table_tid_column_name = $fieldInfo['storage']['details']['sql']['FIELD_LOAD_CURRENT'][$fieldTableName]['tid'];
      $vocabularies[$vid]->bundles = $fieldInfo['bundles'];

      $fieldsTaxonomy[$vid] = $vid;
    }
  }

  // remove vocabularies that are not used on any content type
  foreach ($vocabularies as $vid => $values) {
    if (!in_array($vid, $fieldsTaxonomy, FALSE)) {
      unset($vocabularies[$vid]);
    }
  }

  return $vocabularies;
}


/**
 * Renders the table visualization.
 *
 * @param $rows
 *   An Array of table row data.
 *
 * @return string
 *   The rendered HTML table.
 */
function _bi_visual_render_table($rows) {
  $html = '';

  if (!empty($rows)) {
    $tableHeader = array(
      'name'  => t('Name'),
      'count' => t('Count'),
    );

    try {
      $html .= theme('table', array('header' => $tableHeader,
                                    'rows' => $rows,
                                    'attributes' => array('class' => array('bi'))));
    } catch (Exception $e){
      // exception in cae theme() gets called before all modules are loaded
    }
  }

  return $html;
}


/**
 * Renders the chart visualization.
 *
 * @param $data
 *   Array of data to draw.
 *
 * @return string
 *   The rendered HTML chart.
 */
function _bi_visual_render_chart($data) {
  $html = '';

  if (!empty($data)) {
    $chart = array(
      'id'   => 'bi-piechart',
      'type' => 'piechart',
      'rows' => $data,
    );

    $html = d3_draw($chart);
  }

  return $html;
}