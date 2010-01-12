<?php
// $Id$

/**
 * @file
 * Webform module multiple select component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_select() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'email' => 1,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
      'items' => '',
      'multiple' => NULL,
      'aslist' => NULL,
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_select($component) {
  $form = array();
  $form['extra']['items'] = array(
    '#type' => 'textarea',
    '#title' => t('Options'),
    '#default_value' => $component['extra']['items'],
    '#description' => t('A list of selectable options. One option per line. Key-value pairs may be entered seperated by pipes, such as "safe_key|Some readable option". Option groups for lists and menus may be specified with &lt;Group Name&gt;. &lt;&gt; can be used to insert items at the root of the menu after specifying a group.') . theme('webform_token_help'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -2,
    '#required' => TRUE,
    '#element_validate' => array('_webform_edit_validate_select'),
  );
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field. For multiple selects use commas to separate multiple defaults.') . theme('webform_token_help'),
    '#size' => 60,
    '#maxlength' => 256,
    '#weight' => 0,
  );
  $form['extra']['multiple'] = array(
    '#type' => 'checkbox',
    '#title' => t('Multiple'),
    '#return_value' => 'Y',
    '#default_value' => $component['extra']['multiple'],
    '#description' => t('Check this option if the user should be allowed to choose multiple values.'),
  );
  $form['extra']['aslist'] = array(
    '#type' => 'checkbox',
    '#title' => t('Listbox'),
    '#return_value' => 'Y',
    '#default_value' => $component['extra']['aslist'],
    '#description' => t('Check this option if you want the select component to be of listbox type instead of radiobuttons or checkboxes.'),
  );

  return $form;
}

/**
 * Element validation callback. Ensure keys are not duplicated.
 */
function _webform_edit_validate_select($element, $form_state) {
  // TODO: Validate e-mail addresses when used as keys?\

  // Check for duplicate key values to prevent unexpected data loss.
  if (!empty($element['#value'])) {
    $lines = explode("\n", $element['#value']);
    $existing_keys = array();
    $duplicate_keys = array();
    $group = '';
    foreach ($lines as $line) {
      $matches = array();
      $line = trim($line);
      if (preg_match('/^\<([^>]*)\>$/', $line, $matches)) {
        $group = $matches[1];
        $key = NULL; // No need to store group names.
      }
      elseif (preg_match('/^([^|]+)\|(.*)$/', $line, $matches)) {
        $key = $matches[1];
      }
      else {
        $key = $line;
      }

      if (isset($key)) {
        if (isset($existing_keys[$group][$key])) {
          $duplicate_keys[$key] = $key;
        }
        else {
          $existing_keys[$group][$key] = $key;
        }
      }
    }

    if (!empty($duplicate_keys)) {
      form_error($element, t('Options within the select list must be unique. The following keys have been used multiple times:') . theme('item_list', $duplicate_keys));
    }

  }

  return TRUE;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_select($component, $value = NULL, $filter = TRUE) {
  $form_item = array(
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  // Convert the user-entered options list into an array.
  $default_value = $filter ? _webform_filter_values($component['value'], NULL, NULL, FALSE) : $component['value'];
  $options = _webform_select_options($component['extra']['items'], $component['extra']['aslist'] !== 'Y', $filter);

  if ($component['extra']['aslist'] === 'Y' && $component['extra']['multiple'] !== 'Y') {
    $options = array('' => t('select...')) + $options;
  }

  // Set the component options.
  $form_item['#options'] = $options;

  // Set the default value.
  if (isset($value)) {
    if ($component['extra']['multiple'] === 'Y') {
      // Set the value as an array.
      $form_item['#default_value'] = array();
      foreach ((array) $value as $key => $option_value) {
        $form_item['#default_value'][] = $option_value;
      }
    }
    else {
      // Set the value as a single string.
      $form_item['#default_value'] = '';
      foreach ((array) $value as $option_value) {
        $form_item['#default_value'] = $option_value;
      }
    }
  }
  elseif ($default_value != '') {
    // Convert default value to a list if necessary.
    if ($component['extra']['multiple'] === 'Y') {
      $varray = array_filter(explode(',', $default_value));
      foreach ($varray as $key => $v) {
        $form_item['#default_value'][] = $v;
      }
    }
    else {
      $form_item['#default_value'] = $default_value;
    }
  }

  if ($component['extra']['aslist'] === 'Y') {
    // Set display as a select list:
    $form_item['#type'] = 'select';
    if ($component['extra']['multiple'] === 'Y') {
      $form_item['#multiple'] = TRUE;
    }
  }
  else {
    if ($component['extra']['multiple'] === 'Y') {
      // Set display as a checkbox set.
      $form_item['#type'] = 'checkboxes';
      // Drupal 6 hack to properly render on multipage forms.
      $form_item['#process'] = array('webform_expand_checkboxes');
    }
    else {
      // Set display as a radio set.
      $form_item['#type'] = 'radios';
    }
  }
  return $form_item;
}

/**
 * Drupal 6 hack that properly *renders* checkboxes in multistep forms. This is
 * different than the value hack needed in Drupal 5, which is no longer needed.
 */
function webform_expand_checkboxes($element) {
  // Elements that have a value set are already in the form structure cause
  // them not to be written when the expand_checkboxes function is called.
  $default_value = array();
  foreach (element_children($element) as $key) {
    if (isset($element[$key]['#default_value'])) {
      $default_value[$key] = $element[$key]['#default_value'];
      unset($element[$key]);
    }
  }

  $element = expand_checkboxes($element);
  foreach ($default_value as $key=>$val) {
    $element[$key]['#default_value'] = $val;
  }
  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_select($component, $value) {
  $form_item = _webform_render_select($component, $value);
  if ($component['extra']['multiple'] === 'Y') {
    // Set the value as an array.
    $form_item['#default_value'] = array();
    foreach ((array) $value as $key => $value) {
      $form_item['#default_value'][] = $value;
    }
  }
  else {
    // Set the value as a single string.
    $form_item['#default_value'] = '';
    foreach ((array) $value as $value) {
      $form_item['#default_value'] = $value;
    }
  }
  $form_item['#disabled'] = TRUE;
  return $form_item;
}

/**
 * Implementation of _webform_submit_component().
 *
 * Convert FAPI 0/1 values into something saveable.
 */
function _webform_submit_select($component, $value) {
  $options = drupal_map_assoc(array_flip(_webform_select_options($component['extra']['items'], TRUE)));

  if (is_array($value)) {
    foreach ($value as $key => $option_value) {
      if ($option_value != '') {
        $value[$key] = $options[$key];
      }
      // Checkboxes submit a value of 0 when not checked.
      elseif ($option_value == 0 && $component['extra']['aslist'] !== 'Y' && $component['extra']['multiple'] === 'Y') {
        unset($value[$key]);
      }
      else {
        unset($value[$key]);
      }
    }
  }

  return $value;
}

/**
 * Module specific instance of hook_form_builder_types().
 */
function _webform_form_builder_types_select() {
  $fields = array();

  $fields['select'] = array(
    'title' => t('Select list'),
    'properties' => array(
      'title',
      'description',
      'default_value',
      'required',
      'options',
      'multiple',
      'key',
    ),
    'default' => array(
      '#title' => t('New select list'),
      '#type' => 'select',
      '#options' => array('one' => 'one', 'two' => 'two', 'three' => 'three'),
      '#default_value' => 'one',
      '#webform' => array('type' => 'select'),
    ),
  );

  $fields['checkboxes'] = array(
    'title' => t('Checkboxes'),
    'properties' => array(
      'title',
      'description',
      'default_value',
      'required',
      'options',
      'multiple',
      'key',
    ),
    'default' => array(
      '#title' => t('New checkboxes'),
      '#type' => 'checkboxes',
      '#options' => array('one' => 'one', 'two' => 'two', 'three' => 'three'),
      '#default_value' => array('one', 'two'),
      '#multiple' => TRUE,
      '#webform' => array('type' => 'select'),
    ),
  );

  $fields['radios'] = array(
    'title' => t('Radios'),
    'properties' => array(
      'title',
      'description',
      'default_value',
      'required',
      'options',
      'key',
    ),
    'default' => array(
      '#title' => t('New radios'),
      '#type' => 'radios',
      '#options' => array('one' => 'one', 'two' => 'two', 'three' => 'three'),
      '#default_value' => 'one',
      '#webform' => array('type' => 'select'),
    ),
  );

  return $fields;
}

/**
 * Module specific instance of hook_form_builder_load().
 */
function _webform_form_builder_load_select($form_element) {
  if (in_array($form_element['#type'], array('radios', 'checkboxes', 'select'))) {
    $form_element['#webform']['type'] = 'select';
    if ($form_element['#type'] == 'checkboxes') {
      $form_element['#multiple'] = TRUE;
    }
    return $form_element;
  }
}

/**
 * Implementation of _webform_form_builder_save_component().
 */
function _webform_form_builder_save_select($component, $form_element) {
  $options = '';

  foreach ($form_element['#options'] as $key => $option_value) {
    $options .= $key .'|'. $option_value ."\n";
  }

  $component['extra']['items'] = $options;
  $component['extra']['multiple'] = empty($form_element['#multiple']) ? 'N' : 'Y';
  $component['extra']['aslist'] = $form_element['#type'] == 'select' ? 'Y' : 'N';
  $component['extra']['description'] = isset($form_element['#description']) ? $form_element['#description'] : '';
  return $component;
}

/**
 * Format the output of e-mailed data for this component.
 *
 * @param $component
 *   A Webform component array.
 * @param $value
 *   A string or array of the submitted data.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_select($component, $value) {
  // Convert submitted 'safe' values to un-edited, original form.
  $options = _webform_select_options($component['extra']['items'], TRUE);

  // Generate the output.
  $output = '';
  if ($component['extra']['multiple']) {
    $output .= $component['name'] .":\n";
    foreach ((array) $value as $option_value) {
      if ($option_value) {
        if ($options[$option_value]) {
          $output .= '    - '. $options[$option_value] ."\n";
        }
      }
    }
  }
  else {
    if ($value !== '' && $options[$value]) {
      $output .= $component['name'] .": ". $options[$value] ."\n";
    }
  }
  return $output;
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_select() {
  return array(
    'webform_mail_select' => array(
      'arguments' => array('component' => NULL, 'value' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_select($component, $sids = array()) {
  $options = _webform_select_options($component['extra']['items'], TRUE);
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (".implode(",", $placeholders).")" : "";

  $query = 'SELECT data, count(data) as datacount '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '.
    " AND data != '0' AND data != '' $sidfilter ".
    ' GROUP BY data ';
  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));
  $rows = array();
  while ($data = db_fetch_array($result)) {
    if (isset($options[$data['data']])) {
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
 * Implementation of _webform_table_component().
 */
function _webform_table_select($component, $value) {
  $output = '';
  // Set the value as a single string.
  if (is_array($value)) {
    foreach ($value as $option_value) {
      if ($option_value !== '0') {
        $output .= check_plain($option_value) .'<br />';
      }
    }
  }
  else {
    $output .= check_plain(empty($value[0]) ? '' : $value[0]);
  }
  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_select($component) {
  $headers = array(
    0 => array(),
    1 => array(),
    2 => array(),
  );

  if ($component['extra']['multiple']) {
    $headers[0][] = '';
    $headers[1][] = $component['name'];
    $items = _webform_select_options($component['extra']['items'], TRUE);
    $count = 0;
    foreach ($items as $key => $item) {
      // Empty column per sub-field in main header.
      if ($count != 0) {
        $headers[0][] = '';
        $headers[1][] = '';
      }
      $headers[2][] = $key;
      $count++;
    }
  }
  else {
    $headers[0][] = '';
    $headers[1][] = '';
    $headers[2][] = $component['name'];
  }
  return $headers;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_select($component, $value) {
  $component_values = _webform_filter_values($component['value'], NULL, NULL, FALSE);
  $options = _webform_select_options($component['extra']['items'], TRUE);
  $return = array();

  if ($component['extra']['multiple']) {
    foreach ($options as $key => $item) {
      if (in_array($key, (array) $component_values) === TRUE) {
        $return[] = 'X';
      }
      else {
        $return[] = '';
      }
    }
  }
  else {
    $return = $value[0];
  }
  return $return;
}

/**
 * Utility function to split user-entered values from new-line seperated
 * text into an array of options.
 *
 * @param $text
 *   Text to be converted into a select option array.
 * @param $flat
 *   Optional. If specified, return the option array and exclude any optgroups.
 * @param $filter
 *   Optional. Whether or not to filter returned values.
 */
function _webform_select_options($text, $flat = FALSE, $filter = TRUE) {
  $options = array();
  $rows = array_filter(explode("\n", trim($text)));
  $group = NULL;
  foreach ($rows as $option) {
    $option = trim($option);
    /**
     * If the Key of the option is within < >, treat as an optgroup
     *
     * <Group 1>
     *   creates an optgroup with the label "Group 1"
     *
     * <>
     *   Unsets the current group, allowing items to be inserted at the root element.
     */

    if (preg_match('/^\<([^>]*)\>$/', $option, $matches)) {
      if (empty($matches[1])) {
        unset($group);
      }
      elseif (!$flat) {
        $group = $filter ? _webform_filter_values($matches[1], NULL, NULL, FALSE) : $matches[1];
      }
    }
    elseif (preg_match('/^([^|]+)\|(.*)$/', $option, $matches)) {
      $key = $filter ? _webform_filter_values($matches[1], NULL, NULL, FALSE) : $matches[1];
      $value = $filter ? _webform_filter_values($matches[2], NULL, NULL, FALSE) : $matches[2];
      isset($group) ? $options[$group][$key] = $value : $options[$key] = $value;
    }
    else {
      $filtered_option = $filter ? _webform_filter_values($option, NULL, NULL, FALSE) : $option;
      isset($group) ? $options[$group][$filtered_option] = $filtered_option : $options[$filtered_option] = $filtered_option;
    }
  }
  return $options;
}
