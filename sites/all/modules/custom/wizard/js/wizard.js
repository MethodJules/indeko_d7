(function ($, Drupal) {

  // make hypothesis fieldsets clickable and display results in side bar block
  var elemBlockSearchresults = $('#block-views-searchresults-block');
  $('fieldset.hypothesis').click(function(){

    // display results in the AJAX block view besides the hypothesis
    if (elemBlockSearchresults.length) {

      // get search parameters
      var searchString = $(this).attr('data-search');
      var nodeIds = searchString.match(/(\d+)/g);

      // if there is only one linked node, follow it immediately
      if (nodeIds.length === 1) {
        window.location.href = Drupal.settings.basePath + 'node/' + nodeIds[0]
      } else {
        // execute the AJAX call to display results in sidebar
        $('#edit-keyword').val($(this).attr('data-search'));
        $('#edit-submit-searchresults').click();
      }
    }
  });

  // load qtip2 tooltips for hypothesis fields
  $('.hypothesis').each(function() {
    $(this).qtip({
      content: {
        text: $(this).find('.tooltip'),
        show: {delay: 1}
      },

      position: {
        my: 'bottom left',
        at: 'bottom left',
        target: 'mouse',
        adjust: {
          x: 0, y: -10
        }
      },

      style: {
        tip: false,
        classes: 'field-content'
      }
    });
  });

  // load qtip2 tooltips for references (retrieved via AJAX)
  $('.referencelink').each(function() {
    $(this).qtip({
      content: {
        text: 'Loading preview...',
        ajax: {
          url: Drupal.settings.basePath + 'get/publication/ajax/' + $(this).attr('id'),
          dataType: 'json',
          success: function(data, status) {
            this.set('content.text', data);
          }
        },
        show: {delay: 1}
      },

      position: {
        my: 'bottom left',
        at: 'bottom left',
        target: 'mouse',
        adjust: {
          x: 0, y: -10
        },
        viewport: $(window)
      },

      style: {
        tip: false,
        classes: 'field-content'
      }
    });
  });

  Drupal.behaviors.wizard = {
    attach: function (context, settings) {

      // display hypothesis results and adjust sidebar block size after AJAX update
      if (!elemBlockSearchresults.find('.view-empty').length) {
        elemBlockSearchresults.find('.block-content').css('height','100%');
        elemBlockSearchresults.show();
      }
    }
  };

})(jQuery, Drupal);
