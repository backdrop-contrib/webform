<?php
// $Id$
/**
 * Create a set of form items to be displayed on the form for editing this
 * component. Use care naming the form items, as this correlates directly to the
 * database schema. The component "Name" and "Description" fields are added to
 * every component type and are not necessary to specify here (although they may
 * be overridden if desired).
 * @return
 *   An array of form items to be displayed on the edit component page.
 */
function _webform_edit_select($currfield) {
  $edit_fields = array();
  $edit_fields['extra']['items'] = array(
    '#type' => 'textarea',
    '#title' => t("Options"),
    '#default_value' => $currfield['extra']['items'],
    '#description' => t('A list of selectable options. One option per line. Key-value pairs may be entered seperated by pipes. i.e. safe_key|Some readable option') .'<br />'. webform_help('webform/helptext#variables'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -2,
    '#required' => TRUE,
  );
  $edit_fields['value'] = array(
    '#type' => 'textfield',
    '#title' => t("Default value"),
    '#default_value' => $currfield['value'],
    '#description' => t('The default value of the field. For multiple selects use commas to separate multiple defaults.') .'<br />'. webform_help('webform/helptext#variables'),
    '#size' => 60,
    '#maxlength' => 256,
    '#weight' => 0,
  );
  $edit_fields['extra']['multiple'] = array(
    '#type' => 'checkbox',
    '#title' => t("Multiple"),
    '#return_value' => 'Y',
    '#default_value' => ($currfield['extra']['multiple'] == 'Y' ? TRUE : FALSE),
    '#description' => t('Check this option if the user should be allowed to choose multiple values.'),
  );
  $edit_fields['extra']['aslist'] = array(
    '#type' => 'checkbox',
    '#title' => t("Listbox"),
    '#return_value' => 'Y',
    '#default_value' => ($currfield['extra']['aslist'] == 'Y' ? TRUE : FALSE),
    '#description' => t('Check this option if you want the select component to be of listbox type instead of radiobuttons or checkboxes.'),
  );
  $edit_fields['extra']['email'] = array(
    '#type' => 'checkbox',
    '#title' => t("E-mail a submission copy"),
    '#return_value' => 1,
    '#default_value' => $currfield['extra']['email'],
    '#description' => t('Check this option if this component contains an e-mail address that should get a copy of the submission. Emails are sent individually so other emails will not be shown to the recipient.'),
  );
  return $edit_fields;
}

function _webform_edit_validate_select($form_values) {
  $rows = explode("\n", _webform_filtervalues($form_values['extra']['items'], NULL, NULL, FALSE));
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^(.+)?\|(.*)$/', $row, $matches)) {
      if (preg_match('/ |"/', $matches[1])) {
        form_set_error('field][extra][items', t('The options for this select contain illegal characters (quotes or spaces). Specify your options as <em>safe_value_no_spaces</em>|<em>The Real Value</em>.'));
        return false;
      }
    }
  }
  return true;
}

/**
 * Build a form item array containing all the properties of this component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of a form item to be displayed on the client-side webform.
 */
function _webform_render_select($component) {
  $form_item = array(
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  // Convert the user-entered options list into an array.
  $default_value = _webform_filtervalues($component['value'], NULL, NULL, FALSE);
  $rows = explode("\n", _webform_filtervalues($component['extra']['items'], NULL, NULL, FALSE));

  if ($component['extra']['aslist'] == 'Y' && $component['extra']['multiple'] != 'Y') {
    $options = array('' => t('select...'));
  }
  else {
    $options = array();
  }

  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    }
  }

  // Set the component options.
  $form_item['#options'] = $options;

  // Set the default value.
  if ($default_value) {
    // Convert default value to a list if necessary.
    if ($component['extra']['multiple'] == 'Y') {
      if (strpos($default_value, ',')) {
        $varray = explode(',', $default_value);
        foreach ($varray as $key => $v) {
          if (array_key_exists(_webform_safe_name($v), $options)) {
            $form_item['#default_value'][] = _webform_safe_name($v);
          }
          else {
            $form_item['#default_value'][] = $v;
          }
        }
      }
      else {
        if (array_key_exists(_webform_safe_name($default_value), $options)) {
          $form_item['#default_value'] = array(_webform_safe_name($default_value));
        }
        else {
          $form_item['#default_value'] = array($default_value);
        }
      }
    }
    else {
      if (array_key_exists(_webform_safe_name($default_value), $options)) {
        $form_item['#default_value'] = _webform_safe_name($default_value);
      }
      else {
        $form_item['#default_value'] = $default_value;
      }
    }
  }

  if ($component['extra']['aslist'] == 'Y') {
    // Set display as a select list:
    $form_item['#type'] = 'select';
    if ($component['extra']['multiple'] == 'Y') {
      $form_item['#multiple'] = TRUE;
    }
  }
  else {
    if ($component['extra']['multiple'] == 'Y') {
      // Set display as a checkbox set.
      $form_item['#type'] = 'checkboxes';

    }
    else {
      // Set display as a radio set.
      $form_item['#type'] = 'radios';
    }
  }
  return $form_item;
}


/**
 * Display the result of a textfield submission. The output of this function
 * will be displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_submission_display_select($data, $component, $enabled = false) {
  $form_item = _webform_render_select($component);
  if ($component['extra']['multiple'] == 'Y') {
    // Set the value as an array.
    foreach ((array)$data['value'] as $key => $value) {
      if (array_key_exists(_webform_safe_name($value), $form_item['#options'])) {
        $form_item['#default_value'][] = _webform_safe_name($value);
      }
      else {
        $form_item['#default_value'][] = $value;
      }
    }
  }
  else {
    // Set the value as a single string.
    foreach ((array)$data['value'] as $value) {
      if ($value !== '0') {
        if (array_key_exists(_webform_safe_name($value), $form_item['#options'])) {
          $form_item['#default_value'] = _webform_safe_name($value);
        }
        else {
          $form_item['#default_value'] = $value;
        }
        break;
      }
    }
  }
  $form_item['#disabled'] = !$enabled;
  return $form_item;
}


/**
 * Translates the submitted 'safe' form values back into their un-edited original
 * form.
 * @param $data
 *   The POST data associated with the component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Nothing.
 */
function _webform_submit_select(&$data, $component) {
  $value = _webform_filtervalues($component['value'], NULL, NULL, FALSE);
  $rows = explode("\n", _webform_filtervalues($component['extra']['items'], NULL, NULL, FALSE));

  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $safekey = $matches[1];
      $options[$safekey] = $safekey;
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    }
  }

  if (is_array($data)) {
    foreach ($data as $key => $value) {
      if ($value) {
        $data[$key] = $options[$key];
      }
      else {
      	$data[$key] = NULL;
      }
    }
  }
  elseif (!empty($data)) {
    $data = $options[$data];
  }
}

/**
 * Format the output of emailed data for this component.
 *
 * @param $data
 *   A string or array of the submitted data.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_select($data, $component) {
  // Convert submitted 'safe' values to un-edited, original form.
  $rows = explode("\n", _webform_filtervalues($component['extra']['items'], NULL, NULL, FALSE));
  $options = array();

  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    }
  }
  // Generate the output.
  if ($component['extra']['multiple']) {
    $output = $component['name'] .":\n";
    foreach ((array)$data as $value) {
      if ($value) {
        if ($options[$value]) {
          $output .= "    - ". $options[$value] ."\n";
        }
        elseif($options[_webform_safe_name($value)]) {
          $output .= "    - ". $options[_webform_safe_name($value)] ."\n";
        }
      }
    }
  }
  else {
    if ($options[$data]) {
      $output = $component['name'] .": ". $options[$data] ."\n";
    }
    elseif($options[_webform_safe_name($data)]) {
      $output = $component['name'] .": ". $options[_webform_safe_name($data)] ."\n";
    }
  }
  return $output;
}

/**
 * Module specific instance of hook_help().
 */
function _webform_help_select($section) {
  switch ($section) {
    case 'admin/settings/webform#select_description':
      $output = t("Allows creation of checkboxes, radio buttons, or select menus.");
      break;
  }
  return $output;
}

/**
 * Calculate and returns statistics about results for this component from all
 * submission to this webform. The output of this function will be displayed
 * under the "results" tab then "analysis".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema
 * @return
 *   An array of data rows, each containing a statistic for this component's
 *   submissions.
 */
function _webform_analysis_rows_select($component) {
  $rows = explode("\n", _webform_filtervalues($component['extra']['items'], NULL, NULL, FALSE));
  $options = array();

  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    }
  }

  $query = 'SELECT data, count(data) as datacount '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '.
    " AND data != '0' AND data != '' ".
    ' GROUP BY data ';
  $result = db_query($query, $component['nid'], $component['cid']);
  $rows = array();
  while ($data = db_fetch_array($result)) {
    if ($options[$data['data']]) {
      $display_option = $options[$data['data']];
    }
    else {
      $display_option = $data['data'];
    }
    $rows[] = array($display_option, $data['datacount']);
  }
  return $rows;
}

/**
 * Return the result of this component's submission for display in a table. The
 * output of this function will be displayed under the "results" tab then "table".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_table_data_select($data) {
  // Set the value as a single string.
  if (is_array($data['value'])) {
    foreach ($data['value'] as $value) {
      if ($value !== '0') {
        $output .= check_plain($value) ."<br />";
      }
    }
  }
  else {
    $output = check_plain(empty($data['value']['0']) ? "" : $data['value']['0']);
  }
  return $output;
}


/**
 * Return the header information for this component to be displayed in a comma
 * seperated value file. The output of this function will be displayed under the
 * "results" tab then "download".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of data to be displayed in the first three rows of a CSV file, not
 *   including either prefixed or trailing commas.
 */
function _webform_csv_headers_select($component) {
  $header = array();
  $header[0] = '';

  if ($component['extra']['multiple']) {
    $header[1] = $component['name'];
    $items = split("\n", $component['extra']['items']);
    foreach ($items as $item) {

    $item = trim($item);
    if (preg_match('/^([^"|]+)\|(.*)$/', $item, $matches)) {
      $item = $matches[1];
    }

      $header[2] .= "\,". $item;
      // empty column per sub-field in main header.
      $header[1] .= "\,";
    }
    // Remove the preceding extra comma.
    $header[2] = substr($header[2], 2);
    // Remove the trailing column.
    $header[1] = substr($header[1], 0, -2);
  }
  else {
    $header[2] = $component['name'];
  }
  return $header;
}

/**
 * Return the result of a textfield submission. The output of this function will
 * be displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @return
 *   Textual output formatted for CSV, not including either prefixed or trailing
 *   commas.
 */
function _webform_csv_data_select($data, $component) {
  $value = _webform_filtervalues($component['value'], NULL, NULL, FALSE);
  $rows = explode("\n", _webform_filtervalues($component['extra']['items'], NULL, NULL, FALSE));
  $options = array();

  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    }
  }

  if ($component['extra']['multiple']) {
    foreach ($options as $key => $item) {
      if (in_array($item, (array)$data['value']) === true || in_array($key, (array)$data['value']) === true) {
        $output .= '\,Yes';
      }
      else {
        $output .= '\,No';
      }
    }
    // Remove the preceding extra comma.
    $output = substr($output, 2);
  }
  else {
    $output = $data['value'][0];
  }
  return $output;
}
