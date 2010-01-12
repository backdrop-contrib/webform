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
  $form['extra']['hourformat'] = array(
    '#type' => 'radios',
    '#title' => t('Time Format'),
    '#default_value' => isset($component['extra']['hourformat']) ? $component['extra']['hourformat'] : '12-hour',
    '#description' => t('Format the display of the time in 12 or 24 hours.'),
    '#options' => array('12-hour' => '12-hour (am/pm)', '24-hour' => '24-hour'),
    '#weight' => 2,
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_time($component) {
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

  $form_item = array(
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

  $form_item['hour'] = array(
    '#prefix' => '',
    '#type' => 'select',
    '#default_value' => $hour,
    '#options' => $hours,
  );
  $form_item['minute'] = array(
    '#prefix' => ':',
    '#type' => 'select',
    '#default_value' => $minute,
    '#options' => $minutes,
  );
  if ($component['extra']['hourformat'] == '12-hour') {
    $form_item['ampm'] = array(
      '#type' => 'radios',
      '#default_value' => $am_pm,
      '#options' => $am_pms,
    );
  }

  if (isset($value)) {
    $form_item['minute']['#default_value'] = $value[1];
  
    // Match the hourly data to the hour format.
    if ($value[1]) {
      $timestamp = strtotime($value[0] .':'. $value[1] . (isset($value[2]) ? $value[2] : ''));
      if ($component['extra']['hourformat'] == '24-hour') {
        $form_item['hour']['#default_value'] = date('H', $timestamp);
      }
      else {
        $form_item['hour']['#default_value'] = date('g', $timestamp);
        $form_item['ampm']['#default_value'] = $value[2];
      }
    }
  }

  return $form_item;
}

function webform_validate_time($form_item, $form_state) {
  $form_key = $form_item['#webform_component']['form_key'];
  $name = $form_item['#webform_component']['name'];

  // Check if the user filled the required fields.
  foreach ($form_item['#webform_component'] == '12-hour' ? array('hour', 'minute', 'ampm') : array('hour', 'minute') as $field_type) {
    if (!is_numeric($form_item[$field_type]['#value']) && $form_item['#required']) {
      form_set_error($form_key, t('%field field is required.', array('%field' => $name)));
      return;
    }
  }

  // Check for a valid time.
  if ($form_item['hour']['#value'] !== '' || $form_item['minute']['#value'] !== '') {
    if (!is_numeric($form_item['hour']['#value']) || !is_numeric($form_item['minute']['#value']) || (isset($form_item['ampm']) && $form_item['ampm']['#value'] === '')) {
      form_error($form_key, t('Entered %name is not a valid time.', array('%name' => $name)));
      return;
    }
  }
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_time($component, $value) {
  $form_item = _webform_render_time($component, $value);
  $form_item['minute']['#default_value'] = $value[1];
  $form_item['minute']['#disabled'] = TRUE;
  $form_item['hour']['#disabled'] = TRUE;
  $form_item['ampm']['#disabled'] = TRUE;

  // Match the hourly data to the hour format.
  if ($value[1]) {
    $timestamp = strtotime($value[0] .':'. $value[1] . (isset($value[2]) ? $value[2] : ''));
    if ($component['extra']['hourformat'] == '24-hour') {
      $form_item['hour']['#default_value'] = date('H', $timestamp);
    }
    else {
      $form_item['hour']['#default_value'] = date('g', $timestamp);
      $form_item['ampm']['#default_value'] = $value[2];
    }
  }
  return $form_item;
}

/**
 * Format the output of e-mailed data for this component
 *
 * @param $component
 *   A Webform component array.
 * @param $data
 *   A string or array of the submitted data
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_time($component, $value) {
  $output = $component['name'] .':';
  if ($value['hour'] && $value['minute']) {
    if ($component['extra']['hourformat'] == '24-hour') {
      $output .= ' '. $value['hour'] .':'. $value['minute'];
    }
    else {
      $output .= ' '. $value['hour'] .':'. $value['minute'] .' '. $value['ampm'];
    }
  }
  return $output;
}

/**
 * Implementation of _webform_help_component().
 */
function _webform_help_time($section) {
  switch ($section) {
    case 'admin/settings/webform#time_description':
      return t('Presents the user with hour and minute fields. Optional am/pm fields.');
  }
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_time() {
  return array(
    'webform_time' => array(
      'arguments' => array('element' => NULL),
    ),
    'webform_mail_time' => array(
      'arguments' => array('component' => NULL, 'value' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_analysis_rows_component().
 */
function _webform_analysis_rows_time($component, $sids = array()) {
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
 * Implementation of _webform_table_data_component().
 */
function _webform_table_data_time($component, $value) {
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
function _webform_csv_headers_time($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_time($component, $value) {
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
