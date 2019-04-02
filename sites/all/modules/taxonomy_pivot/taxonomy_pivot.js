(function ($) {
  Drupal.behaviors.initTaxonomyPivot = {
    attach: function (context, settings) {
      $('.pivotValueCellFull', context).once('taxonomyPivot').click(function () {
        var x_axis = $(this).children('.hidden_x_axis').val();
        var y_axis = $(this).children('.hidden_y_axis').val();
        var filters = $(this).children('.hidden_filters').val();

        //Load URL
        var url = Drupal.settings.basePath + 'taxonomy-pivot/load-nodes';

        //Progress Throbber
        var progress = $('<div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div>');
        $(this).append(progress);

        $.ajax({
          type: "POST",
          url: url,
          data: { x_axis: x_axis, y_axis: y_axis, filters: filters },
          success: function(html) {
            $('.ajax-progress-throbber').remove();
            $("#nodes_area").html(html);
          },
        });
      });
    }
  };
})(jQuery);
