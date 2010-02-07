<?php
// $Id$

/**
 * @file
 * Webform module date component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_date() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'email' => 1,
    'extra' => array(
      'check_daylight_savings' => 0,
      'year_start' => '1900',
      'year_end' => '2050',
      'year_textfield' => 0,
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_date() {
  return array(
    'webform_date' => array(
      'arguments' => array('element' => NULL),
    ),
    'webform_display_date' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_date($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.') .'<br />'. t('Accepts any date in any <a href="http://www.gnu.org/software/tar/manual/html_chapter/Date-input-formats.html">GNU Date Input Format</a>. Strings such as today, +2 months, and Dec 9 2004 are all valid.'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
  );
  $form['extra']['timezone'] = array(
    '#type' => 'radios',
    '#title' => t('Timezone'),
    '#default_value' => empty($component['extra']['timezone']) ? 'site' : $component['extra']['timezone'],
    '#description' => t('Adjust the date according to a specific timezone. Website timezone is defined in the <a href="!settings">Site Settings</a> and is the default.', array('!settings' => url('admin/settings/date-time'))),
    '#options' => array('site' => t('Website timezone'), 'user' => t('User timezone'), 'gmt' => t('GMT')),
    '#weight' => 1,
  );
  $form['extra']['check_daylight_savings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Observe Daylight Savings'),
    '#default_value' => $component['extra']['check_daylight_savings'],
    '#checked_value' => 1,
    '#description' => t('Automatically adjust the time during daylight savings.'),
    '#weight' => 2,
  );

  $form['extra']['year_start'] = array(
    '#type' => 'textfield',
    '#title' => t('Start year'),
    '#default_value' => empty($component['extra']['year_start']) ? '1900' : $component['extra']['year_start'],
    '#description' => t('The first year that is allowed to be entered.'),
    '#size' => 10,
    '#maxlength' => 4,
    '#weight' => 3,
  );
  $form['extra']['year_end'] = array(
    '#type' => 'textfield',
    '#title' => t('End year'),
    '#default_value' => empty($component['extra']['year_end']) ? '2050' : $component['extra']['year_end'],
    '#description' => t('The last year that is allowed to be entered.'),
    '#size' => 10,
    '#maxlength' => 4,
    '#weight' => 4,
  );
  $form['extra']['year_textfield'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use a textfield for year'),
    '#default_value' => $component['extra']['year_textfield'],
    '#description' => t('If checked, the generated date field will use a textfield for the year. Otherwise it will use a select list.'),
    '#weight' => 5,
  );

  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_date($component, $value = NULL) {
  $element = array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#type' => 'date',
    '#description' => _webform_filter_descriptions($component['extra']['description']),
    '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix' => '</div>',
    '#required' => $component['mandatory'],
    '#webform_component' => $component,
    '#process' => array('webform_expand_date'),
    '#element_validate' => array('webform_validate_date'),
  );

  if (isset($value)) {
    $element['#default_value'] = array(
      'month' => $value[0],
      'day' => $value[1],
      'year' => $value[2],
    );
  }

  return $element;
}

/**
 * Form API #process function for Webform date fields.
 */
function webform_expand_date($element) {
  $component = $element['#webform_component'];

  // Set defaults according to existing #default_value (set by Form API)
  if (isset($element['#default_value']['month']) || isset($element['#default_value']['day']) || isset($element['#default_value']['year'])) {
    $default_values = array(
      'month' => $element['#default_value']['month'],
      'day' => $element['#default_value']['day'],
      'year' => $element['#default_value']['year'],
    );
  }
  // Or, if none, use set the defaults of the component.
  elseif (drupal_strlen($component['value']) > 0) {
    // Calculate the timestamp in GMT.
    $timestamp = webform_strtotime(_webform_filter_values($component['value']));

    if ($component['extra']['timezone'] == 'user') {
      // Use the users timezone.
      global $user;
      $timestamp += (int)$user->timezone;
    }
    elseif ($component['extra']['timezone'] == 'gmt') {
      // Use GMT.
      $timestamp += 0;
    }
    else {
      // Use the Drupal site time.
      $timestamp += variable_get('date_default_timezone', 0);
    }

    // Check for daylight savings time.
    if ($component['extra']['check_daylight_savings'] && date('I')) {
      $timestamp += 3600;
    }

    $default_values = array(
      'day' => gmdate('j', $timestamp),
      'month' => gmdate('n', $timestamp),
      'year' => gmdate('Y', $timestamp),
    );
  }
  else {
    $default_values = array(
      'day' => NULL,
      'month' => NULL,
      'year' => NULL,
    );
  }

  // Let Drupal do it's normal expansion.
  $element = expand_date($element);

  // Don't use Drupal's validation though.
  $element['#element_validate'] = array('webform_validate_date');

  // Set default values.
  foreach ($default_values as $type => $value) {
    switch ($type) {
      case 'month':
        $none = t('Month');
        break;
      case 'day':
        $none = t('Day');
        break;
      case 'year':
        $none = t('Year');
        break;
    }
    unset($element[$type]['#value']);
    $element[$type]['#default_value'] = isset($default_values[$type]) ? $default_values[$type] : NULL;
    $element[$type]['#options'] = array('' => $none) + $element[$type]['#options'];
  }

  // Tweak the year field.
  if ($component['extra']['year_textfield']) {
    $element['year']['#type'] = 'textfield';
    $element['year']['#size'] = 5;
    $element['year']['#maxlength'] = 4;
    unset($element['year']['#options']);
  }
  elseif (is_numeric($component['extra']['year_start']) && is_numeric($component['extra']['year_end'])) {
    $element['year']['#options'] = array('' => t('Year')) + drupal_map_assoc(range($component['extra']['year_start'], $component['extra']['year_end']));
  }

  return $element;
}

/**
 * Element validation for Webform date fields.
 */
function webform_validate_date($element, $form_state) {
  $component = $element['#webform_component'];
  $form_key = $component['form_key'];
  $name = $component['name'];

  // Check if the user filled the required fields.
  foreach (array('day', 'month', 'year') as $field_type) {
    if (!is_numeric($element[$field_type]['#value']) && $element['#required']) {
      form_set_error($form_key, t('!name field is required.', array('!name' => $name)));
      return;
    }
  }
  // Check for a valid date.
  if ($element['month']['#value'] !== '' || $element['day']['#value'] !== '' || $element['year']['#value'] !== '') {
    if (!checkdate((int)$element['month']['#value'], (int)$element['day']['#value'], (int)$element['year']['#value'])) {
      form_set_error($form_key, t('Entered !name is not a valid date.', array('!name' => $name)));
      return;
    }
  }
  // Check the date is between allowed years.
  if ($element['year']['#value'] !== '' && is_numeric($component['extra']['year_start']) && is_numeric($component['extra']['year_end'])) {
    if ($element['year']['#value'] < $component['extra']['year_start'] || $element['year']['#value'] > $component['extra']['year_end']) {
      form_set_error($form_key .'][year', t('The entered date needs to be between the years @start and @end.', array('@start' => $component['extra']['year_start'], '@end' => $component['extra']['year_end'])));
      return;
    }
  }
}

/**
 * Implementation of _webform_submit_component().
 */
function _webform_submit_date($component, $value) {
  // Webform stores dates in month/day/year rows.
  // Ensure consistency when using international date formats.
  return array($value['month'], $value['day'], $value['year']);
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_date($component, $value, $format = 'html') {
  $value = array(
    'month' => isset($value[0]) ? $value[0] : NULL,
    'day' => isset($value[1]) ? $value[1] : NULL,
    'year' => isset($value[2]) ? $value[2] : NULL,
  );

  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_date',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#component' => $component,
    '#format' => $format,
    '#value' => $value,
  );
}

/**
 * Format the text output for this component.
 */
function theme_webform_display_date($element) {
  $output = ' ';
  if ($element['#value']) {
    $timestamp = webform_strtotime($element['#value']['month'] .'/'. $element['#value']['day'] .'/'. $element['#value']['year']);
    $format = webform_date_format('medium');
    $output = format_date($timestamp, 'custom', $format, 0);
  }

  return $output;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_date($component, $sids = array()) {
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (".implode(",", $placeholders).")" : "";
  $query = 'SELECT no,data '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND  cid = %d '. $sidfilter.
    ' ORDER BY sid,no ASC ';

  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));
  // build an array of timestamps from entered values.
  $timestamps = array();
  $submissions = 1;
  while ($row = db_fetch_array($result)) {
    if ($row['no'] == '0') {
      $submissions++;
      $month = $row['data'];
      if ($row = db_fetch_array($result)) {
        if ($row['no'] == '1') {
          $day = $row['data'];
          if ($row = db_fetch_array($result)) {
            if ($row['no'] == '2') {
              $year = $row['data'];
              // Build the full timestamp.
              if (drupal_strlen($month) > 0  && drupal_strlen($day) > 0  && drupal_strlen($year) > 0) {
                $timestamp = webform_strtotime($month .'/'. $day .'/'. $year);
                // Add usefull information about this date into an array.
                $timestamps[$timestamp] = array(
                  date('l', $timestamp), // Day of the week (Monday, Tuesday, etc.).
                  date('F', $timestamp), // Full Month name (January, February, etc.).
                  $year, // Year.
                  $day,  // Day of the month (1,2,...,31).
                );
              }
            }
          }
        }
      }
    }
  }

  // Display stats.
  $nonblanks = count($timestamps);
  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  return $rows;
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_date($component, $value) {
  if (drupal_strlen($value[0]) > 0 && drupal_strlen($value[1]) > 0 && drupal_strlen($value[2]) > 0) {
    return check_plain($value[0] .'/'. $value[1] .'/'. $value[2]);
  }
  else {
    return '';
  }
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_date($component, $export_options) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_date($component, $export_options, $value) {
  if (drupal_strlen($value[0]) > 0 && drupal_strlen($value[1]) > 0 && drupal_strlen($value[2]) > 0) {
    $timestamp = webform_strtotime($value[0] .'/'. $value[1] .'/'. $value[2]);
    $format = webform_date_format('short');
    return format_date($timestamp, 'custom', $format, 0);
  }
  else {
    return '';
  }
}

/**
 * Get a timestamp in GMT time, ensuring timezone accuracy.
 */
function webform_strtotime($date) {
  $current_tz = date_default_timezone_get();
  date_default_timezone_set('UTC');
  $timestamp = strtotime($date);
  date_default_timezone_set($current_tz);
  return $timestamp;
}

/**
 * Get a date format according to the site settings.
 *
 * @param $size
 *   A choice of 'short', 'medium', or 'long' date formats.
 */
function webform_date_format($size = 'medium') {
    // Format date according to site's given format.
    $format = variable_get('date_format_'. $size, 'D, m/d/Y - H:i');
    $time = 'aABgGhHisueIOPTZ';
    $day_of_week = 'Dlw';
    $special = ',-: ';
    $date_format = trim($format, $time . $day_of_week . $special);

    // Ensure that a day, month, and year value are present. Use a default
    // format if all the values are not found.
    if (strpos($date_format, array('d', 'j')) === FALSE || strpos($date_format, array('F', 'm', 'M', 'n')) === FALSE || strpos($date_format, array('o', 'Y', 'y')) === FALSE) {
      $date_format = 'm/d/Y';
    }

    return $date_format;
}
