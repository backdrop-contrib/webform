
/**
 * JavaScript behaviors for the front-end display of webforms.
 */

(function ($) {

Backdrop.behaviors.webform = Backdrop.behaviors.webform || {};

Backdrop.behaviors.webform.attach = function(context) {
  // Calendar datepicker behavior.
  Backdrop.webform.datepicker(context);

  // Conditional logic.
  if (Backdrop.settings.webform && Backdrop.settings.webform.conditionals) {
    Backdrop.webform.conditional(context);
  }
};

Backdrop.webform = Backdrop.webform || {};

Backdrop.webform.datepicker = function(context) {
  $('div.webform-datepicker').each(function() {
    var $webformDatepicker = $(this);
    var $calendar = $webformDatepicker.find('input.webform-calendar');

    // Ensure the page we're on actually contains a datepicker.
    if ($calendar.length == 0) {
      return;
    }

    var startDate = $calendar[0].className.replace(/.*webform-calendar-start-(\d{4}-\d{2}-\d{2}).*/, '$1').split('-');
    var endDate = $calendar[0].className.replace(/.*webform-calendar-end-(\d{4}-\d{2}-\d{2}).*/, '$1').split('-');
    var firstDay = $calendar[0].className.replace(/.*webform-calendar-day-(\d).*/, '$1');
    // Convert date strings into actual Date objects.
    startDate = new Date(startDate[0], startDate[1] - 1, startDate[2]);
    endDate = new Date(endDate[0], endDate[1] - 1, endDate[2]);

    // Ensure that start comes before end for datepicker.
    if (startDate > endDate) {
      var laterDate = startDate;
      startDate = endDate;
      endDate = laterDate;
    }

    var startYear = startDate.getFullYear();
    var endYear = endDate.getFullYear();

    // Set up the jQuery datepicker element.
    $calendar.datepicker({
      dateFormat: 'yy-mm-dd',
      yearRange: startYear + ':' + endYear,
      firstDay: parseInt(firstDay),
      minDate: startDate,
      maxDate: endDate,
      onSelect: function(dateText, inst) {
        var date = dateText.split('-');
        $webformDatepicker.find('select.year, input.year').val(+date[0]).trigger('change');
        $webformDatepicker.find('select.month').val(+date[1]).trigger('change');
        $webformDatepicker.find('select.day').val(+date[2]).trigger('change');
      },
      beforeShow: function(input, inst) {
        // Get the select list values.
        var year = $webformDatepicker.find('select.year, input.year').val();
        var month = $webformDatepicker.find('select.month').val();
        var day = $webformDatepicker.find('select.day').val();

        // If empty, default to the current year/month/day in the popup.
        var today = new Date();
        year = year ? year : today.getFullYear();
        month = month ? month : today.getMonth() + 1;
        day = day ? day : today.getDate();

        // Make sure that the default year fits in the available options.
        year = (year < startYear || year > endYear) ? startYear : year;

        // jQuery UI Datepicker will read the input field and base its date off
        // of that, even though in our case the input field is a button.
        $(input).val(year + '-' + month + '-' + day);
      }
    });

    // Prevent the calendar button from submitting the form.
    $calendar.click(function(event) {
      $(this).focus();
      event.preventDefault();
    });
  });
};

Backdrop.webform.conditional = function(context) {
  // Add the bindings to each webform on the page.
  $.each(Backdrop.settings.webform.conditionals, function(formKey, settings) {
    var $form = $('.' + formKey + ':not(.webform-conditional-processed)');
    $form.each(function(index, currentForm) {
      var $currentForm = $(currentForm);
      $currentForm.addClass('webform-conditional-processed');
      $currentForm.bind('change', { 'settings': settings }, Backdrop.webform.conditionalCheck);

      // Trigger all the elements that cause conditionals on this form.
      $.each(Backdrop.settings.webform.conditionals[formKey]['ruleGroups'], function(rgid_key, rule_group) {
        Backdrop.webform.doCondition($form, settings, rgid_key);
      });
    })
  });
};

/**
 * Event handler to respond to field changes in a form.
 *
 * This event is bound to the entire form, not individual fields.
 */
Backdrop.webform.conditionalCheck = function(e) {
  var $triggerElement = $(e.target).closest('.webform-component');
  var $form = $triggerElement.closest('form');
  var triggerElementKey = $triggerElement.attr('class').match(/webform-component--[^ ]+/)[0];
  var settings = e.data.settings;
  if (settings.sourceMap[triggerElementKey]) {
    $.each(settings.ruleGroups, function(rgid_key, rule_group) {
      Backdrop.webform.doCondition($form, settings, rgid_key);
    });
  }
};

/**
 * Processes one condition.
 */
Backdrop.webform.doCondition = function($form, settings, rgid_key) {
  var ruleGroup = settings.ruleGroups[rgid_key];

  // Perform the comparison callback and build the results for this group.
  var conditionalResult = true;
  var conditionalResults = [];
  $.each(ruleGroup['rules'], function(m, rule) {
    var elementKey = rule['source'];
    var element = $form.find('.' + elementKey)[0];
    var existingValue = settings.values[elementKey] ? settings.values[elementKey] : null;
    conditionalResults.push(window['Backdrop']['webform'][rule.callback](element, existingValue, rule['value'] ));
  });

  // Filter out false values.
  var filteredResults = [];
  for (var i = 0; i < conditionalResults.length; i++) {
    if (conditionalResults[i]) {
      filteredResults.push(conditionalResults[i]);
    }
  }

  // Calculate the and/or result.
  if (ruleGroup['andor'] === 'or') {
    conditionalResult = filteredResults.length > 0;
  }
  else {
    conditionalResult = filteredResults.length === conditionalResults.length;
  }

  // Flip the result of the action is to hide.
  var showComponent;
  if (ruleGroup['action'] == 'hide') {
    showComponent = !conditionalResult;
  }
  else {
    showComponent = conditionalResult;
  }

  var $target = $form.find('.' + ruleGroup['target']);
  var $targetElements;
  if (showComponent != Backdrop.webform.isVisible($target)) {
    if (showComponent) {
      $targetElements = $target.find('.webform-conditional-disabled').removeClass('webform-conditional-disabled');
      $.fn.prop ? $targetElements.prop('disabled', false) : $targetElements.removeAttr('disabled');
      $target.show();
    }
    else {
      $targetElements = $target.find(':input').addClass('webform-conditional-disabled');
      $.fn.prop ? $targetElements.prop('disabled', true) : $targetElements.attr('disabled', true);
      $target.hide();
    }
  }
}

Backdrop.webform.conditionalOperatorStringEqual = function(element, existingValue, ruleValue) {
  var returnValue = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase() === ruleValue.toLowerCase()) {
      returnValue = true;
      return false; // break.
    }
  });
  return returnValue;
};

Backdrop.webform.conditionalOperatorStringNotEqual = function(element, existingValue, ruleValue) {
  var found = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase() === ruleValue.toLowerCase()) {
      found = true;
    }
  });
  return !found;
};

Backdrop.webform.conditionalOperatorStringContains = function(element, existingValue, ruleValue) {
  var returnValue = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase().indexOf(ruleValue.toLowerCase()) > -1) {
      returnValue = true;
      return false; // break.
    }
  });
  return returnValue;
};

Backdrop.webform.conditionalOperatorStringDoesNotContain = function(element, existingValue, ruleValue) {
  var found = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase().indexOf(ruleValue.toLowerCase()) > -1) {
      found = true;
    }
  });
  return !found;
};

Backdrop.webform.conditionalOperatorStringBeginsWith = function(element, existingValue, ruleValue) {
  var returnValue = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase().indexOf(ruleValue.toLowerCase()) === 0) {
      returnValue = true;
      return false; // break.
    }
  });
  return returnValue;
};

Backdrop.webform.conditionalOperatorStringEndsWith = function(element, existingValue, ruleValue) {
  var returnValue = false;
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  $.each(currentValue, function(n, value) {
    if (value.toLowerCase().lastIndexOf(ruleValue.toLowerCase()) === value.length - ruleValue.length) {
      returnValue = true;
      return false; // break.
    }
  });
  return returnValue;
};

Backdrop.webform.conditionalOperatorStringEmpty = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  var returnValue = true;
  $.each(currentValue, function(n, value) {
    if (value !== '') {
      returnValue = false;
      return false; // break.
    }
  });
  return returnValue;
};

Backdrop.webform.conditionalOperatorStringNotEmpty = function(element, existingValue, ruleValue) {
  return !Backdrop.webform.conditionalOperatorStringEmpty(element, existingValue, ruleValue);
};

Backdrop.webform.conditionalOperatorNumericEqual = function(element, existingValue, ruleValue) {
  // See float comparison: http://php.net/manual/en/language.types.float.php
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  var epsilon = 0.000001;
  // An empty string does not match any number.
  return currentValue[0] === '' ? false : (Math.abs(parseFloat(currentValue[0]) - parseFloat(ruleValue)) < epsilon);
};

Backdrop.webform.conditionalOperatorNumericNotEqual = function(element, existingValue, ruleValue) {
  // See float comparison: http://php.net/manual/en/language.types.float.php
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  var epsilon = 0.000001;
  // An empty string does not match any number.
  return currentValue[0] === '' ? true : (Math.abs(parseFloat(currentValue[0]) - parseFloat(ruleValue)) >= epsilon);
};

Backdrop.webform.conditionalOperatorNumericGreaterThan = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  return parseFloat(currentValue[0]) > parseFloat(ruleValue);
};

Backdrop.webform.conditionalOperatorNumericLessThan = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.stringValue(element, existingValue);
  return parseFloat(currentValue[0]) < parseFloat(ruleValue);
};

Backdrop.webform.conditionalOperatorDateEqual = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.dateValue(element, existingValue);
  return currentValue === ruleValue;
};

Backdrop.webform.conditionalOperatorDateBefore = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.dateValue(element, existingValue);
  return (currentValue !== false) && currentValue < ruleValue;
};

Backdrop.webform.conditionalOperatorDateAfter = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.dateValue(element, existingValue);
  return (currentValue !== false) && currentValue > ruleValue;
};

Backdrop.webform.conditionalOperatorTimeEqual = function(element, existingValue, ruleValue) {
  var currentValue = Backdrop.webform.timeValue(element, existingValue);
  return currentValue === ruleValue;
};

Backdrop.webform.conditionalOperatorTimeBefore = function(element, existingValue, ruleValue) {
  // Date and time operators intentionally exclusive for "before".
  var currentValue = Backdrop.webform.timeValue(element, existingValue);
  return (currentValue !== false) && (currentValue < ruleValue);
};

Backdrop.webform.conditionalOperatorTimeAfter = function(element, existingValue, ruleValue) {
  // Date and time operators intentionally inclusive for "after".
  var currentValue = Backdrop.webform.timeValue(element, existingValue);
  return (currentValue !== false) && (currentValue >= ruleValue);
};

/**
 * Utility to return current visibility. Uses actual visibility, except for
 * hidden components which use the applied disabled class.
 */
Backdrop.webform.isVisible = function($element) {
  return $element.hasClass('webform-component-hidden')
            ? !$element.find('input').first().hasClass('webform-conditional-disabled')
            : $element.is(':visible');
}

/**
 * Utility function to get a string value from a select/radios/text/etc. field.
 */
Backdrop.webform.stringValue = function(element, existingValue) {
  var value = [];
  if (element) {
    var $element = $(element);
    if (Backdrop.webform.isVisible($element)) {
      // Checkboxes and radios.
      $element.find('input[type=checkbox]:checked,input[type=radio]:checked').each(function() {
        value.push(this.value);
      });
      // Select lists.
      if (!value.length) {
        var selectValue = $element.find('select').val();
        if (selectValue) {
          value.push(selectValue);
        }
      }
      // Simple text fields. This check is done last so that the select list in
      // select-or-other fields comes before the "other" text field.
      if (!value.length) {
        $element.find('input:not([type=checkbox],[type=radio]),textarea').each(function() {
          value.push(this.value);
        });
      }
    }
  }
  else {
    switch ($.type(existingValue)) {
      case 'array':
        value = existingValue;
        break;
      case 'string':
        value.push(existingValue);
        break;
    }
  }
  return value;
};

/**
 * Utility function to calculate a millisecond timestamp from a time field.
 */
Backdrop.webform.dateValue = function(element, existingValue) {
  var value = false;
  if (element) {
    var $element = $(element);
    if ($element.is(':visible')) {
      var day = $element.find('[name*=day]').val();
      var month = $element.find('[name*=month]').val();
      var year = $element.find('[name*=year]').val();
      // Months are 0 indexed in JavaScript.
      if (month) {
        month--;
      }
      if (year !== '' && month !== '' && day !== '') {
        value = Date.UTC(year, month, day) / 1000;
      }
    }
  }
  else {
    if ($.type(existingValue) === 'array' && existingValue.length) {
      existingValue = existingValue[0];
    }
    if ($.type(existingValue) === 'string') {
      existingValue = existingValue.split('-');
    }
    if (existingValue.length === 3) {
      value = Date.UTC(existingValue[0], existingValue[1], existingValue[2]) / 1000;
    }
  }
  return value;
};

/**
 * Utility function to calculate a millisecond timestamp from a time field.
 */
Backdrop.webform.timeValue = function(element, existingValue) {
  var value = false;
  if (element) {
    var $element = $(element);
    if ($element.is(':visible')) {
      var hour = $element.find('[name*=hour]').val();
      var minute = $element.find('[name*=minute]').val();
      var ampm = $element.find('[name*=ampm]:checked').val();

      // Convert to integers if set.
      hour = (hour === '') ? hour : parseInt(hour);
      minute = (minute === '') ? minute : parseInt(minute);

      if (hour !== '') {
        hour = (hour < 12 && ampm == 'pm') ? hour + 12 : hour;
        hour = (hour === 12 && ampm == 'am') ? 0 : hour;
      }
      if (hour !== '' && minute !== '') {
        value = Date.UTC(1970, 0, 1, hour, minute) / 1000;
      }
    }
  }
  else {
    if ($.type(existingValue) === 'array' && existingValue.length) {
      existingValue = existingValue[0];
    }
    if ($.type(existingValue) === 'string') {
      existingValue = existingValue.split(':');
    }
    if (existingValue.length >= 2) {
      value = Date.UTC(1970, 0, 1, existingValue[0], existingValue[1]) / 1000;
    }
  }
  return value;
};

})(jQuery);
