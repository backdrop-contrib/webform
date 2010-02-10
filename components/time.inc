<?php
// $Id$

/**
 * @file
 * Webform module time component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_time() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'email' => 1,
    'extra' => array(
      'timezone' => 'site',
      'check_daylight_savings' => 0,
      'hourformat' => '12-hour',
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_time() {
  return array(
    'webform_time' => array(
      'arguments' => array('element' => NULL),
    ),
    'webform_display_time' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_time($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.') .'<br />'. t('Accepts a time in any <a href="http://www.gnu.org/software/tar/manual/html_chapter/Date-input-formats.html">GNU Date Input Format</a>. Strings such as now, +2 hours, and 10:30pm are all valid.'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
    '#element_validate' => array('webform_validate_time_string'),
  );
  $form['extra']['timezone'] = array(
    '#type' => 'radios',
    '#title' => t('Timezone'),
    '#default_value' => empty($component['extra']['timezone']) ? 'site' : $component['extra']['timezone'],
    '#description' => t('Adjust the time according to a specific timezone. Website timezone is defined in the <a href="!settings">Site Settings</a> and is the default.', array('!settings' => url('admin/settings/date-time'))),
    '#options' => array('site' => 'Website Timezone', 'user' => 'User Timezone', 'gmt' => 'GMT'),
    '#weight' => 0,
  );
  $form['extra']['check_daylight_savings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Observe Daylight Savings'),
    '#default_value' => $component['extra']['check_daylight_savings'],
    '#checked_value' => 1,
    '#description' => t('Automatically adjust the time during daylight savings.'),
    '#weight' => 1,
  );
  $form['display']['hourformat'] = array(
    '#type' => 'radios',
    '#title' => t('Time Format'),
    '#default_value' => isset($component['extra']['hourformat']) ? $component['extra']['hourformat'] : '12-hour',
    '#description' => t('Format the display of the time in 12 or 24 hours.'),
    '#options' => array('12-hour' => '12-hour (am/pm)', '24-hour' => '24-hour'),
    '#weight' => 2,
    '#parents' => array('extra', 'hourformat'),
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_time($component, $value = NULL) {
  if (drupal_strlen($component['value']) > 0) {
    // Calculate the timestamp in GMT.
    $timestamp = strtotime($component['value']);
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
      $timestamp += (int)variable_get('date_default_timezone', 0);
    }

    // Check for daylight savings time.
    if ($component['extra']['check_daylight_savings'] && date('I')) {
      $timestamp += 3600;
    }
  }

  if ($component['extra']['hourformat'] == '12-hour') {
    $first_hour = 1;
    $last_hour = 12;
    $hour_format = 'g';
  }
  else {
    $first_hour = 0;
    $last_hour = 23;
    $hour_format = 'G';
  }

  if (drupal_strlen($component['value']) > 0) {
    $hour = gmdate($hour_format, $timestamp);
    $minute = gmdate('i', $timestamp);
    $am_pm = gmdate('a', $timestamp);
  }
  else {
    $hour = '';
    $minute = '';
    $am_pm = 'am';
  }

  // Generate the choices for drop-down selects.
  $hours[''] = t('hour');
  $minutes[''] = t('minute');
  for ($i = $first_hour; $i <= $last_hour; $i++) $hours[$i] = $i;
  for ($i = 0; $i <= 59; $i++) $minutes[$i < 10 ? "0$i" : $i] = $i < 10 ? "0$i" : $i;
  $am_pms = array('am' => t('am'), 'pm' => t('pm'));

  $element = array(
    '#title' => $component['name'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#description' => _webform_filter_descriptions($component['extra']['description']),
    '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix' => '</div>',
    '#theme' => 'webform_time',
    '#element_validate' => array('webform_validate_time'),
    '#webform_component' => $component,
  );

  $element['hour'] = array(
    '#prefix' => '',
    '#type' => 'select',
    '#default_value' => $hour,
    '#options' => $hours,
  );
  $element['minute'] = array(
    '#prefix' => ':',
    '#type' => 'select',
    '#default_value' => $minute,
    '#options' => $minutes,
  );
  if ($component['extra']['hourformat'] == '12-hour') {
    $element['ampm'] = array(
      '#type' => 'radios',
      '#default_value' => $am_pm,
      '#options' => $am_pms,
    );
  }

  if (isset($value)) {
    $element['minute']['#default_value'] = $value[1];
  
    // Match the hourly data to the hour format.
    if ($value[1]) {
      $timestamp = strtotime($value[0] .':'. $value[1] . (isset($value[2]) ? $value[2] : ''));
      if ($component['extra']['hourformat'] == '24-hour') {
        $element['hour']['#default_value'] = date('H', $timestamp);
      }
      else {
        $element['hour']['#default_value'] = date('g', $timestamp);
        $element['ampm']['#default_value'] = $value[2];
      }
    }
  }

  return $element;
}

function webform_validate_time($element, $form_state) {
  $form_key = $element['#webform_component']['form_key'];
  $name = $element['#webform_component']['name'];

  // Check if the user filled the required fields.
  foreach ($element['#webform_component'] == '12-hour' ? array('hour', 'minute', 'ampm') : array('hour', 'minute') as $field_type) {
    if (!is_numeric($element[$field_type]['#value']) && $element['#required']) {
      form_set_error($form_key, t('%field field is required.', array('%field' => $name)));
      return;
    }
  }

  // Check for a valid time.
  if ($element['hour']['#value'] !== '' || $element['minute']['#value'] !== '') {
    if (!is_numeric($element['hour']['#value']) || !is_numeric($element['minute']['#value']) || (isset($element['ampm']) && $element['ampm']['#value'] === '')) {
      form_error($form_key, t('Entered %name is not a valid time.', array('%name' => $name)));
      return;
    }
  }
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_time($component, $value, $format = 'html') {
  $value = array(
    'hour' => isset($value[0]) ? $value[0] : NULL,
    'minute' => isset($value[1]) ? $value[1] : NULL,
    'ampm' => isset($value[2]) ? $value[2] : NULL,
  );

  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_time',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#component' => $component,
    '#format' => $format,
    '#hourformat' => $component['extra']['hourformat'],
    '#value' => $value,
  );
}

/**
 * Format the output of data for this component.
 */
function theme_webform_display_time($element) {
  $output = ' ';
  if ($element['#value']['hour'] && $element['#value']['minute']) {
    if ($element['#hourformat'] == '24-hour') {
      $output = $element['#value']['hour'] .':'. $element['#value']['minute'];
    }
    else {
      $output = $element['#value']['hour'] .':'. $element['#value']['minute'] .' '. $element['#value']['ampm'];
    }
  }
  return $output;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_time($component, $sids = array()) {
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
      $hour = $row['data'];
      if ($row = db_fetch_array($result)) {
        if ($row['no'] == '1') {
          $minute = $row['data'];
          if ($row = db_fetch_array($result)) {
            if ($row['no'] == '2') {
              $ampm = $row['data'];
              // Build the full timestamp.
              if (drupal_strlen($hour) > 0 && drupal_strlen($minute) > 0 ) {
                $timestamps[] = strtotime($hour .':'. $minute .' '. $ampm);
              }
            }
            else {
              // Build military time.
              $timestamps[] = strtotime($hour .':'. $minute);
            }
          }
        }
      }
    }
  }

  // Display stats.
  // TODO: display date statistics in javascript tabs.
  $nonblanks = count($timestamps);
  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  return $rows;
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_time($component, $value) {
  if (drupal_strlen($value[0]) > 0 && drupal_strlen($value[1]) > 0) {
    $timestamp = strtotime($value[0] .':'. $value[1] . $value[2]);
    if ($component['extra']['hourformat'] == '24-hour') {
      return check_plain(date('H:i', $timestamp));
    }
    else {
      return check_plain(date('g:i a', $timestamp));
    }
  }
  else {
    return '';
  }
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_time($component, $export_options) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_time($component, $export_options, $value) {
  if (drupal_strlen($value[0]) > 0 && drupal_strlen($value[1]) > 0) {
    $timestamp = strtotime($value[0] .':'. $value[1] . $value[2]);
    if ($component['extra']['hourformat'] == '24-hour') {
      return date('H:i', $timestamp);
    }
    else {
      return date('g:i a', $timestamp);
    }
  }
  else {
    return '';
  }
}

/**
 * Theme a webform time element.
 */
function theme_webform_time($element) {
  $element['#type'] = 'element';
  $element['#children'] = '<div class="container-inline">'. drupal_render($element['hour']) . drupal_render($element['minute']) . drupal_render($element['ampm']) .'</div>';

  return theme('form_element', $element, $element['#children']);
}
