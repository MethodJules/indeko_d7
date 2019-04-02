********************************************************************
                     D R U P A L    M O D U L E
********************************************************************
Name: Taxonomy Pivot
Author: Tobias Heide <tobias.heide at ercis.uni-muenster dot de>
Drupal: 7
********************************************************************

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Features
 * Upcoming features
 * Installation

INTRODUCTION
------------

The Taxnomy Pivot module generates a pivot-style Table with vocabularies
as dimensions on the axis. As values in the cells of the table it shows
the number of nodes attached to the terms of the vocabulary.

Example:
The nodes in the system are classified by the vocabularies Voc1
(with Terms a,b,c) and Voc2 (with Terms x,y,z). If Voc1 is chosen for the
x-Axis and Voc2 is chosen for the y-Axis the resulting table will look like
this:
      a     b     c
x    1     5    13
y    0     4      3
z    4     5     6

In this example there is 1 node classified with a & y and there are 6 nodes
with c & z.

Another more demonstrative example of a car resale site:
Each node is a car and the dimension describe different properties of the cars.
Dim1: Transmission (Vocabulary): Automatic,  Manual (Terms)
Dim2: Color: Red, Blue, Yellow
Dim3: Model: Ford, VW, BWM, Mercedes

In the pivot table now filters and the values for axis can be defined.
For the filter the "model" (or multiple dimensions) could be set and the terms
VW and BMW are selected. On the x-axis is the color and on the y-axis the type
of transmission. Each axis can have 0, 1 or multiple taxonomies assigned.
The described setup results in the following pivot table:

Model: VW, BMW
           Red Blue Yellow
Automatic   5   7     3
Manual      2   3     1

Which means there are 5 cars of red color with an automatic transmission of the
models VW and BMW.
Further options are the presentation of this table as a chart, the export
to Excel or displaying the nodes "behind" the values. A click on the "5" would
load a list of the 5 node titles and link to the contents.

In contrast to a view not only the output as a pivot table differs, but also
the configuration of filters. Not only the filter values (terms) can be
selected by the user, but also the configuration of the filters (which
vocabularies are used) is made available to the user.

Project page: https://drupal.org/sandbox/majestixx/1705412

FEATURES
--------
 * Multiple vocabularies on x- & y-axis: Each term is combined with each other
 * Applying multiple filters: If multiple vocabularies and multiple terms are
   selected, terms from same vocabulary are connected by "OR", while different
   vocabularies are connected by "AND". Example: Filter for a & b from Voc1 and
   x from Voc2. The node needs to have either a or b and needs x to be
   displayed.
 * Support for hierarchical vocabularies: From each vocabulary just the upper
   most level is shown and all data from childterms is aggregated.
   (See drill-down in upcoming features)
 * Excel Export: The generated table can be exported to excel by downloading
   an xls-File.
 * Display nodes: For each cells the nodes 'behind' the figure can be
   displayed and are listed below the table
 * Generation of diagrams: Generation of bar or pie charts based on the data
   in the table

UPCOMING FEATURES
-----------------
 * Drill down for hierarchical vocabularies
 * Support of relations between contenttypes: Tables can have vocabularies
   connected to different contenttypes on the axis. The node count is then
   calculated by using the relationship of nodes
 * Preconfiguration of pivot tables: Own content type for the Taxonomy Pivot
   table. The content type has settings like x-axis, y-axis and filters which
   can be preconfigured and exposed to the used (like it is currently the case)
   or fixed to display always the same data.

PREREQUISITES
-------------
 * Vocabularies with terms must exist
   (@see http://example.com/admin/structure/taxonomy)
 * Vocabulary has to be referenced to contenttype by "term reference"
   (@see e.g. http://example.com/admin/structure/types/manage/page/fields)
 * Terms have to be assigned to nodes

RECOMMENDED ADDITIONAL MODULES
-------------------------------
 * PHPExcel - For exporting tables to Excel
   (@see https://drupal.org/project/phpexcel)
 * Chart - For drawing tables as charts
   (@see https://drupal.org/project/chart)

INSTALLATION
------------
 * Install module
 * Call /taxonomy-pivot on your Drupal site
