// $Id$

/**
 * Webform node form interface enhancments.
 */

Drupal.behaviors.webform = function(context) {
  // Apply special behaviors to fields with default values.
  Drupal.webform.defaultValues(context);
  // On click or change, make a parent radio button selected.
  Drupal.webform.setActive(context);
  // Update the template select list upon changing a template.
  Drupal.webform.updateTemplate(context);
}

Drupal.webform = new Object();

Drupal.webform.defaultValues = function(context) {
  var $fields = $('.webform-default-value', context);
  var $forms = $('.webform-default-value', context).parents('form:first');
  $fields.each(function() {
    this.defaultValue = this.value;
    $(this).focus(function() {
      if (this.value == this.defaultValue) {
        this.value = '';
        $(this).removeClass('webform-default-value');
      }
    });
    $(this).blur(function() {
      if (this.value == '') {
        $(this).addClass('webform-default-value');
        this.value = this.defaultValue;
      }
    });
  });

  // Clear all the form elements before submission.
  $forms.submit(function() {
    $fields.focus();
  });
};

Drupal.webform.setActive = function(context) {
  var setActive = function() {
    $('.form-radio', $(this).parent().parent()).attr('checked', true);
  };
  $('.webform-set-active', context).click(setActive).change(setActive);
};

Drupal.webform.updateTemplate = function(context) {
  var defaultTemplate = $('#edit-templates-default').val();
  var $templateSelect = $('#webform-template-fieldset select', context);
  var $templateTextarea = $('#webform-template-fieldset textarea', context);

  var updateTemplateSelect = function() {
    if ($(this).val() == defaultTemplate) {
      $templateSelect.val('default');
    }
    else {
      $templateSelect.val('custom');
    }
  }

  var updateTemplateText = function() {
    if ($(this).val() == 'default') {
      $templateTextarea.val(defaultTemplate);
    }
  }

  $templateTextarea.keyup(updateTemplateSelect);
  $templateSelect.change(updateTemplateText);
}
