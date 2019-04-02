<?php

/**
 * @file
 * Morphbox page callbacks for the morphsearch module.
 */


/**
 * Form builder to list vocabularies as a table.
 *
 * @ingroup forms
 * @see morphsearch_morphbox_submit()
 * @see theme_morphsearch_morphbox()
 */
function morphsearch_morphbox($form) {
  $form['#tree'] = TRUE;

  $all = array();
  $all[0] = array();
  $noTermsMax = 0;

  // get all vocabularies
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $vocabulary) {

    // filter morphological box taxonomy vocabularies (prefix 'tax_morph_')
    if (0 === strpos($vocabulary->machine_name, 'tax_morph_')) {
      $all[0][$vocabulary->vid] = array('data' => check_plain($vocabulary->name), 'id' => $vocabulary->vid);

      $termsTree = taxonomy_get_tree($vocabulary->vid);

      $noTermsMax = count($termsTree) > $noTermsMax ? count($termsTree) : $noTermsMax;
    }
  }


  // build matrix with one column per vocabulary
  foreach ($all[0] as $vid => $vocName) {
    for ($i = 1; $i <= $noTermsMax; $i++) {
      $termsTree = taxonomy_get_tree($vid);

      if (isset($termsTree[$i - 1]->name)) {
        $name = check_plain($termsTree[$i - 1]->name);
        $depthClass = 'term-depth-' . $termsTree[$i - 1]->depth;
        $all[$i][] = '<div  class=' . $depthClass . '><span>' . $name . '</span></div>';
      } else {
        $all[$i][] = '';
      }
    }
  }

  $header = $all[0];
  unset($all[0]);
  $rows = $all;
  $tableHtml = theme('table', array('header' => $header, 'rows' => $all));



  $form['table'] = array(
    '#markup' => $tableHtml,
    '#prefix' => '<div class="morphbox-table">',
    '#suffix' => '</div>',
  );

  // Attach morphbox JavaScript.
  $form['content']['#attached']['js'][] = array(
    'data' => drupal_get_path('module', 'morphsearch') . "/js/morphbox.js",
    'scope' => 'footer',
  );

  // Attach morphbox CSS.
  $form['content']['#attached']['css'][] = array(
    'data' => drupal_get_path('module', 'morphsearch') . "/css/morphbox.css",
    'type' => 'file',
    'scope' => 'footer',
    'group' => CSS_THEME,
  );

  return $form;
}

/**
 * Submit handler for morphsearch morphbox. Updates changed vocabularies and adds new vocabularies.
 *
 * @see morphsearch_morphbox()
 */
function morphsearch_morphbox_submit($form, &$form_state) {
  foreach ($form_state['values'] as $vid => $vocabulary) {
    if (is_numeric($vid) && $form[$vid]['#vocabulary']->weight != $form_state['values'][$vid]['weight']) {
      $form[$vid]['#vocabulary']->weight = $form_state['values'][$vid]['weight'];
      taxonomy_vocabulary_save($form[$vid]['#vocabulary']);
    }
  }
  drupal_set_message(t('The vocabulary has been saved.'));
}


/**
 * Form buildes to create and update whole vocabularies with terms.
 *
 * @param $form
 * @param $form_state
 * @param $vocabulary_id
 *
 * @return array
 *   finished $form
 */
function morphsearch_get_vocabulary_edit($form, &$form_state, $vocabulary_id) {

  if (isset($vocabulary_id)) {
    $voc = taxonomy_vocabulary_load($vocabulary_id);
  }

  $form['vocabulary_name'] = [
    '#type' => 'textfield',
    '#title' => t('Dimension'),
    '#weight' => 0,
    '#default_value' => $voc !== FALSE ? check_plain($voc->name) : '',
    '#element_validate' => array('morphsearch_morphbox_validate_not_empty'),
    '#attributes' => [
      'class' => ['field-edit-voc'],
    ],
  ];

  $form['vocabulary_vid'] = [
    '#type' => 'hidden',
    '#value' => $voc !== FALSE ? $voc->vid : -1,
  ];

  $form['terms_container'] = [
    '#type' => 'container',
    '#title' => t('Options'),
    '#weight' => 80,
    '#tree' => TRUE,
    // Set up the wrapper so that AJAX will be able to replace the fieldset.
    '#prefix' => '<div id="js-ajax-elements-wrapper">',
    '#suffix' => '</div>',
  ];

  $form['terms_container']['title'] = [
    '#type' => 'item',
    '#title' => t('Values'),
    '#prefix' => '<span class="terms-container-title">',
    '#suffix' => '</span>',
  ];

  if (isset($form_state['no_terms'])) {
    // user changed number of terms (add or delete)
    $form_state['no_terms'] = $form_state['no_terms'];
    $termTree = taxonomy_get_tree($voc->vid);
  } else if ($voc !== FALSE) {
    // first call to edit a vocabulary, load all existing terms for editing
    $termTree = taxonomy_get_tree($voc->vid);
    $form_state['no_terms'] = empty($termTree) ? range(0,2) : range(0, count($termTree) - 1);
  } else {
    // add a new vocabulary, start with 3 blank term fields
    $form_state['no_terms'] = range(0, 2);
  }

  $term_count = $form_state['no_terms'];
  foreach ($term_count as $key => $delta) {

    $form['terms_container'][$delta] = [
      '#type' => 'container',
      '#attributes' => [
        'class' => ['container-inline'],
      ],
      '#tree' => TRUE,
    ];

    $form['terms_container'][$delta]['name'] = [
      '#type' => 'textfield',
      '#size' => 60,
      '#default_value' => isset($termTree[$key]) ? $termTree[$key]->name : '',
      '#attributes' => [
        'class' => ['field-edit-term'],
      ],
    ];

    $form['terms_container'][$delta]['tid'] = [
      '#type' => 'hidden',
      '#value' => isset($termTree[$key]) ? $termTree[$key]->tid : -1,
    ];

    $form['terms_container'][$delta]['weight'] = array(
      '#type' => 'hidden',
      '#value' => isset($termTree[$key]) ? $termTree[$key]->weight : $key,
    );

    $form['terms_container'][$delta]['remove_term'] = [
      '#type' => 'submit',
      '#value' => t('-'),
      '#submit' => ['morphsearch_ajax_edit_vocabulary_remove'],
      '#ajax' => [
        'callback' => 'morphsearch_ajax_edit_vocabulary_remove_callback',
        'wrapper' => 'js-ajax-elements-wrapper',
      ],
      '#weight' => -1,
      '#attributes' => [
        'class' => ['button-term-remove'],
      ],
      '#name' => 'remove_term_' . $delta,
    ];
  }

  $form['terms_container']['add_term'] = [
    '#type' => 'submit',
    '#value' => t('Add term'),
    '#submit' => ['morphsearch_ajax_edit_vocabulary_add_term'],
    '#ajax' => [
      'callback' => 'morphsearch_ajax_edit_vocabulary_add_term_callback',
      'wrapper' => 'js-ajax-elements-wrapper',
    ],
    '#attributes' => [
      'class' => ['button-term-add'],
    ],
    '#weight' => 100,
  ];

  // edit term order and hierarchy using standard taxonomy form
  if ($voc !== FALSE) {
    $form['terms_container']['change_order'] = [
      '#markup' => l('Edit order / hierarchy', 'admin/structure/taxonomy/' . $voc->machine_name,
        array('attributes' => array('class' => 'link-voc-edit'))),
      '#weight' => 110,
    ];
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 999,
  );

  $form['actions']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save dimension'),
    '#submit' => ['morphsearch_edit_vocabulary_save'],
    '#attributes' => [
      'id' => ['button-voc-save'],
    ],
  );

  $form['actions']['add_dimension'] = array(
    '#type' => 'submit',
    '#value' => t('Add dimension'),
    '#submit' => ['morphsearch_edit_vocabulary_add'],
    '#attributes' => [
      'id' => ['button-voc-add'],
    ],
  );

  // show delete vocabulary button only while editing an existing vocabulary
  if ($voc !== FALSE) {
    $form['actions']['delete'] = array(
      '#type'       => 'submit',
      '#value'      => t('Delete dimension'),
      '#submit'     => ['morphsearch_edit_vocabulary_delete'],
      '#attributes' => [
        'id' => ['button-voc-delete'],
      ],
    );
  }

  return $form;
}

/**
 * Returns form to create a new vocabulary and the morphological box table.
 */
function morphsearch_get_morphbox($form, &$form_state) {

  // add morphological box edit form
  $form = morphsearch_get_vocabulary_edit($form,$form_state, 0);

  // add morphological box to from
  $form = morphsearch_morphbox($form);
  return $form;
}

/**
 * Returns form to edit an existing vocabulary and the morphological box table.
 */
function morphsearch_get_morphbox_vid($form, &$form_state, $vocabulary_id) {

  // add morphological box edit form
  $form = morphsearch_get_vocabulary_edit($form,$form_state, $vocabulary_id);

  // add morphological box to from
  $form = morphsearch_morphbox($form);
  return $form;
}

/**
 * Dynamically removes a form element and deletes the vocabulary term.
 *
 * @throws \Exception
 */
function morphsearch_ajax_edit_vocabulary_remove($form, &$form_state) {
  $delta_remove = $form_state['triggering_element']['#parents'][1];
  $term = $form_state['values']['terms_container'][$delta_remove];
  taxonomy_term_delete($term['tid']);

  $k = array_search($delta_remove, $form_state['no_terms']);
  unset($form_state['no_terms'][$k]);
  $form_state['no_terms'] = array_values($form_state['no_terms']);

  $form_state['rebuild'] = TRUE;

  #taxonomy_check_vocabulary_hierarchy($form['#vocabulary'], $form_state['values']);
  drupal_set_message(t('Deleted term %name.', array('%name' => $term['name'])));
  watchdog('taxonomy', 'Deleted term %name.', array('%name' => array('%name' => $term['name'])), WATCHDOG_NOTICE);
  cache_clear_all();

  drupal_get_messages();
}

/**
 * AJAX callback after form element has been removed.
 */
function morphsearch_ajax_edit_vocabulary_remove_callback($form, &$form_state) {
  return $form['terms_container'];
}

/**
 * Dynamically adds a form element for a new empty term.
 */
function morphsearch_ajax_edit_vocabulary_add_term($form, &$form_state) {
  $form_state['no_terms'][] = count($form_state['no_terms']) > 0 ? max($form_state['no_terms']) + 1 : 0;

  $form_state['rebuild'] = TRUE;
  drupal_get_messages();
}

/**
 * AJAX callback after form element has been added.
 */
function morphsearch_ajax_edit_vocabulary_add_term_callback($form, $form_state) {
  return $form['terms_container'];

}

/**
 * Creates a new vocabulary or updates an existing one including all terms.
 */
function morphsearch_edit_vocabulary_save($form, &$form_state) {

  if ($form_state['submit_handlers'][0] === 'morphsearch_edit_vocabulary_save') {
    $name = $form_state['values']['vocabulary_name'];
    $vid = $form_state['values']['vocabulary_vid'];


    // update existing vocabulary
    $vocabulary = new stdClass();
    if ($vid !== -1) {
      $vocabulary = taxonomy_vocabulary_load($vid);
      $vocabulary->name = $name;
    } else {
      // add new vocabulary
      $vocabulary->name = $name;
      $machineName = preg_replace('/[^a-z0-9]+/', '', strtolower($name));
      $vocabulary->machine_name = 'tax_morph_m_' . substr($machineName,0,28);
    }

    $status = taxonomy_vocabulary_save($vocabulary);
    switch ($status) {
      case SAVED_NEW:
        drupal_set_message(t('Created new vocabulary %name %vid.', array('%name' => $vocabulary->name, '%vid' => $vocabulary->vid)));
        watchdog('taxonomy', 'Created new vocabulary %name.', array('%name' => $vocabulary->name), WATCHDOG_NOTICE, l(t('edit'), 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/edit'));
        break;

      case SAVED_UPDATED:
        drupal_set_message(t('Updated vocabulary %name %vid.', array('%name' => $vocabulary->name, '%vid' => $vocabulary->vid)));
        watchdog('taxonomy', 'Updated vocabulary %name.', array('%name' => $vocabulary->name), WATCHDOG_NOTICE, l(t('edit'), 'admin/structure/taxonomy/' . $vocabulary->machine_name . '/edit'));
        break;
    }



    foreach ($form_state['values']['terms_container'] as $term) {
      $morphTerm = new stdClass();

      if (isset($term['tid'])) {
        // update term
        if ($term['tid'] !== -1) {
          $morphTerm = taxonomy_term_load($term['tid']);
          $morphTerm->name = $term['name'];
          taxonomy_term_save($morphTerm);
        } else {
          // add new term, ignore empty name fields
          if (!empty($term['name'])) {
            $morphTerm->vid = $vocabulary->vid;
            $morphTerm->name = $term['name'];
            $morphTerm->weight = $term['weight'];
            taxonomy_term_save($morphTerm);
          }
        }
      }
    }

    $form_state['redirect'] = 'config/morphbox/' . $vocabulary->vid;
  }
}


/**
 * Deletes a vocabulary.
 */
function morphsearch_edit_vocabulary_delete($form, &$form_state) {
  $vid = $form_state['values']['vocabulary_vid'];

  if ($vid > 0) {
    taxonomy_vocabulary_delete($vid);
    $form_state['redirect'] = 'config/morphbox/';
  }
}

/**
 * Redirects to the new vocabulary form.
 */
function morphsearch_edit_vocabulary_add($form, &$form_state) {

  if ($form_state['submit_handlers'][0] === 'morphsearch_edit_vocabulary_add') {
    $form_state['redirect'] = 'config/morphbox';
  }

}

/**
 * Form element validate handler. Validates that form field is not empty.
 */
function morphsearch_morphbox_validate_not_empty($element, &$form_state) {
  if (empty($element['#value'])) {
    form_error($element, t('This field is required.'));
  }
}