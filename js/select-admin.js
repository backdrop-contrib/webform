
/**
 * @file
 * Enhancements for select list configuration options.
 */

(function ($) {

Backdrop.behaviors.webformSelectLoadOptions = {};
Backdrop.behaviors.webformSelectLoadOptions.attach = function(context) {
  settings = Backdrop.settings;

  $('#edit-extra-options-source', context).change(function() {
    var url = settings.webform.selectOptionsUrl + '/' + this.value;
    $.ajax({
      url: url,
      success: Backdrop.webform.selectOptionsLoad,
      dataType: 'json'
    });
  });
}

Backdrop.webform = Backdrop.webform || {};

Backdrop.webform.selectOptionsOriginal = false;
Backdrop.webform.selectOptionsLoad = function(result) {
  if (Backdrop.optionsElement) {
    if (result.options) {
      // Save the current select options the first time a new list is chosen.
      if (Backdrop.webform.selectOptionsOriginal === false) {
        Backdrop.webform.selectOptionsOriginal = $(Backdrop.optionElements[result.elementId].manualOptionsElement).val();
      }
      $(Backdrop.optionElements[result.elementId].manualOptionsElement).val(result.options);
      Backdrop.optionElements[result.elementId].disable();
      Backdrop.optionElements[result.elementId].updateWidgetElements();
    }
    else {
      Backdrop.optionElements[result.elementId].enable();
      if (Backdrop.webform.selectOptionsOriginal) {
        $(Backdrop.optionElements[result.elementId].manualOptionsElement).val(Backdrop.webform.selectOptionsOriginal);
        Backdrop.optionElements[result.elementId].updateWidgetElements();
        Backdrop.webform.selectOptionsOriginal = false;
      }
    }
  }
  else {
    var $element = $('#' + result.elementId);
    if (result.options) {
      $element.val(result.options);
      $.fn.prop ? $element.prop('readonly', true) : $element.attr('readonly', 'readonly');
    }
    else {
      $.fn.prop ? $element.prop('readonly', false) : $element.removeAttr('readonly');
    }
  }
}

})(jQuery);
