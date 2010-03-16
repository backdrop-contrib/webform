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
      'optrand' => 0,
      'other_option' => NULL,
      'other_text' => t('Other...'),
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_select() {
  return array(
    'webform_display_select' => array(
      'render element' => 'element',
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
    '#description' => t('<strong>Key-value pairs MUST be specified as "safe_key|Some readable option"</strong>. Only alphanumeric characters and underscores are allowed as a key. One option per line. Option groups may be specified with &lt;Group Name&gt;. &lt;&gt; can be used to insert items at the root of the menu after specifying a group.') . theme('webform_token_help'),
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
    '#default_value' => $component['extra']['multiple'],
    '#description' => t('Check this option if the user should be allowed to choose multiple values.'),
  );
  $form['extra']['aslist'] = array(
    '#type' => 'checkbox',
    '#title' => t('Listbox'),
    '#default_value' => $component['extra']['aslist'],
    '#description' => t('Check this option if you want the select component to be of listbox type instead of radiobuttons or checkboxes.'),
  );
  $form['extra']['other_option'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow "Other..." option'),
    '#default_value' => $component['extra']['other_option'],
    '#description' => t('Check this option if you want to allow users to enter an option not on the list.'),
    '#access' => module_exists('select_or_other'),
  );
  $form['extra']['other_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Text for "Other..." option'),
    '#default_value' => $component['extra']['other_text'],
    '#description' => t('If allowing other options, enter text to be used for other-enabling option.'),
    '#access' => module_exists('select_or_other'),
  );
  $form['display']['optrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize options'),
    '#default_value' => $component['extra']['optrand'],
    '#description' => t('Randomizes the order of the options when they are displayed in the form.'),
    '#parents' => array('extra', 'optrand'),
  );

  return $form;
}

/**
 * Element validation callback. Ensure keys are not duplicated.
 */
function _webform_edit_validate_select($element, &$form_state) {
  // Check for duplicate key values to prevent unexpected data loss. Require
  // all options to include a safe_key.
  if (!empty($element['#value'])) {
    $lines = explode("\n", trim($element['#value']));
    $existing_keys = array();
    $duplicate_keys = array();
    $missing_keys = array();
    $long_keys = array();
    $group = '';
    foreach ($lines as $line) {
      $matches = array();
      $line = trim($line);
      if (preg_match('/^\<([^>]*)\>$/', $line, $matches)) {
        $group = $matches[1];
        $key = NULL; // No need to store group names.
      }
      elseif (preg_match('/^([^|]*)\|(.*)$/', $line, $matches)) {
        $key = $matches[1];
        if (strlen($key) > 128) {
          $long_keys[] = $key;
        }
      }
      else {
        $missing_keys[] = $line;
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

    if (!empty($missing_keys)) {
      form_error($element, t('Every option must have a key specified. Specify each option as "safe_key|Some readable option".'));
    }

    if (!empty($long_keys)) {
      form_error($element, t('Option keys must be less than 128 characters. The following keys exceed this limit:') . theme('item_list', $long_keys));
    }

    if (!empty($duplicate_keys)) {
      form_error($element, t('Options within the select list must be unique. The following keys have been used multiple times:') . theme('item_list', array('items' => $duplicate_keys)));
    }

  }

  return TRUE;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_select($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
  );

  // Convert the user-entered options list into an array.
  $default_value = $filter ? _webform_filter_values($component['value'], NULL, NULL, NULL, FALSE) : $component['value'];
  $options = _webform_select_options($component['extra']['items'], !$component['extra']['aslist'], $filter);

  if ($component['extra']['optrand']) {
    _webform_shuffle_options($options);
  }

  if ($component['extra']['aslist'] && !$component['extra']['multiple'] && !($component['mandatory'] && !empty($component['value']))) {
    $options = array('' => t('select...')) + $options;
  }

  // Set the component options.
  $element['#options'] = $options;

  // Set the default value.
  if (isset($value)) {
    if ($component['extra']['multiple']) {
      // Set the value as an array.
      $element['#default_value'] = array();
      foreach ((array) $value as $key => $option_value) {
        $element['#default_value'][] = $option_value;
      }
    }
    else {
      // Set the value as a single string.
      $element['#default_value'] = '';
      foreach ((array) $value as $option_value) {
        $element['#default_value'] = $option_value;
      }
    }
  }
  elseif ($default_value != '') {
    // Convert default value to a list if necessary.
    if ($component['extra']['multiple']) {
      $varray = array_filter(explode(',', $default_value));
      foreach ($varray as $key => $v) {
        $element['#default_value'][] = $v;
      }
    }
    else {
      $element['#default_value'] = $default_value;
    }
  }
  elseif ($component['extra']['multiple']) {
    $element['#default_value'] = array();
  }

  if ($component['extra']['other_option'] && module_exists('select_or_other')) {
    // Set display as a select list:
    $element['#type'] = 'select_or_other';
    $element['#other'] = !empty($component['extra']['other_text']) ? check_plain($component['extra']['other_text']) : t('Other...');
    $element['#other_unknown_defaults'] = 'other';
    $element['#other_delimiter'] = ', ';
    if ($component['extra']['multiple']) {
      $element['#multiple'] = TRUE;
      $element['#select_type'] = 'checkboxes';
    }
    else {
      $element['#multiple'] = FALSE;
      $element['#select_type'] = 'radios';
    }
    if ($component['extra']['aslist']) {
      $element['#select_type'] = 'select';
    }
  }
  elseif ($component['extra']['aslist']) {
    // Set display as a select list:
    $element['#type'] = 'select';
    if ($component['extra']['multiple']) {
      $element['#multiple'] = TRUE;
    }
  }
  else {
    if ($component['extra']['multiple']) {
      // Set display as a checkbox set.
      $element['#type'] = 'checkboxes';
      // Drupal 6 hack to properly render on multipage forms.
      $element['#process'] = array('webform_expand_checkboxes', 'webform_expand_select_ids');
    }
    else {
      // Set display as a radio set.
      $element['#type'] = 'radios';
      $element['#process'] = array('expand_radios', 'webform_expand_select_ids');
    }
  }

  $element['#prefix'] = '<div class="webform-component-' . $element['#type'] . '" id="webform-component-' . $component['form_key'] . '">';
  $element['#suffix'] = '</div>';

  return $element;
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

  // Escape the values of checkboxes.
  foreach (element_children($element) as $key) {
    $element[$key]['#return_value'] = check_plain($element[$key]['#return_value']);
    $element[$key]['#name'] = $element['#name'] . '[' . $element[$key]['#return_value'] . ']';
  }

  foreach ($default_value as $key => $val) {
    $element[$key]['#default_value'] = $val;
  }
  return $element;
}

/**
 * FAPI process function to rename IDs attached to checkboxes and radios.
 */
function webform_expand_select_ids($element) {
  $id = $element['#id'] = str_replace('_', '-', _webform_safe_name(strip_tags($element['#id'])));
  $delta = 0;
  foreach (element_children($element) as $key) {
    $delta++;
    // Convert the #id for each child to a safe name, regardless of key.
    $element[$key]['#id'] = $id . '-' . $delta;

    // Prevent scripts or CSS in the labels for each checkbox or radio.
    $element[$key]['#title'] = _webform_filter_xss($element[$key]['#title']);
  }
  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_select($component, $value, $format = 'html') {
  $value = (array) $value;
  ksort($value);
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_select',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#component' => $component,
    '#format' => $format,
    '#value' => $value,
  );
}

/**
 * Implementation of _webform_submit_component().
 *
 * Convert FAPI 0/1 values into something saveable.
 */
function _webform_submit_select($component, $value) {
  $options = drupal_map_assoc(array_flip(_webform_select_options($component['extra']['items'], TRUE)));

  $return = NULL;
  if (is_array($value)) {
    $return = array();
    foreach ($value as $key => $option_value) {
      // Handle options that are specified options.
      if ($option_value !== '' && isset($options[$key])) {
        // Checkboxes submit an *integer* value of 0 when not checked.
        if ($option_value === 0 && $options[$key] !== '0' && !$component['extra']['aslist'] && $component['extra']['multiple']) {
          unset($value[$key]);
        }
        else {
          $return[] = $key;
        }
      }
      // Handle options that are added through the "other" field.
      elseif ($component['extra']['other_option'] && module_exists('select_or_other')) {
        $return[] = $option_value;
      }
    }
  }
  elseif (is_string($value)) {
    $return = $value;
  }

  return $return;
}

/**
 * Format the text output for this component.
 */
function theme_webform_display_select($variables) {
  $element = $variables['element'];
  $component = $element['#component'];

  // Convert submitted 'safe' values to un-edited, original form.
  $options = _webform_select_options($component['extra']['items'], TRUE);

  $items = array();
  if ($component['extra']['multiple']) {
    foreach ((array) $element['#value'] as $option_value) {
      if ($option_value !== '') {
        // Administer provided values.
        if (isset($options[$option_value])) {
          $items[] = _webform_filter_xss($options[$option_value]);
        }
        // User-specified in the "other" field.
        else {
          $items[] = $option_value;
        }
      }
    }
  }
  else {
    if (isset($element['#value'][0]) && $element['#value'][0] !== '') {
      // Administer provided values.
      if (isset($options[$element['#value'][0]])) {
        $items[] = $element['#format'] == 'html' ? _webform_filter_xss($options[$element['#value'][0]]) : $options[$element['#value'][0]];
      }
      // User-specified in the "other" field.
      else {
        $items[] = $element['#format'] == 'html' ? check_plain($element['#value'][0]) : $element['#value'][0];
      }
    }
  }

  if ($element['#format'] == 'html') {
    $output = count($items) > 1 ? theme('item_list', array('items' => $items)) : (isset($items[0]) ? $items[0] : ' ');
  }
  else {
    if (count($items) > 1) {
      foreach ($items as $key => $item) {
        $items[$key] = ' - ' . $item;
      }
      $output = implode("\n", $items);
    }
    else {
      $output = isset($items[0]) ? $items[0] : ' ';
    }
  }

  return $output;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_select($component, $sids = array(), $single = FALSE) {
  $options = _webform_select_options($component['extra']['items'], TRUE);
  $show_other_results = $single;

  $sid_placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sid_filter = count($sids) ? " AND sid IN (" . implode(",", $sid_placeholders) . ")" : "";

  $option_operator = $show_other_results ? 'NOT IN' : 'IN';
  $query = db_select('webform_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('wsd', array('data'))
    ->condition('nid', $component['nid'])
    ->condition('cid', $component['cid'])
    ->condition('data', '', '<>')
    ->condition('data', $options, $option_operator)
    ->groupBy('data');
  $query->addExpression('COUNT(data)', 'datacount');

  if (count($sids)) {
    $query->condition('sid', $sids, 'IN');
  }

  $count_query = db_select('webform_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
    ->condition('nid', $component['nid'])
    ->condition('cid', $component['cid'])
    ->condition('data', '', '<>');
  $count_query->addExpression('COUNT(*)', 'datacount');
  if (count($sids)) {
    $count_query->condition('sid', $sids, 'IN');
  }

  $result = $query->execute();
  $rows = array();
  $normal_count = 0;
  foreach ($result as $data) {
    $display_option = $single ? $data['data'] : $options[$data['data']];
    $rows[$data['data']] = array(_webform_filter_xss($display_option), $data['datacount']);
    $normal_count += $data['datacount'];
  }

  if (!$show_other_results) {
    // Order the results according to the normal options array.
    $ordered_rows = array();
    foreach (array_intersect_key($options, $rows) as $key => $label) {
      $ordered_rows[] = $rows[$key];
    }

    // Add a row for any unknown or user-entered values.
    if ($component['extra']['other_option']) {
      $full_count = $count_query->execute()->fetchField();
      $other_count = $full_count - $normal_count;
      $display_option = !empty($component['extra']['other_text']) ? check_plain($component['extra']['other_text']) : t('Other...');
      $other_text = $other_count ? $other_count . ' (' . l(t('view'), 'node/' . $component['nid'] . '/webform-results/analysis/' . $component['cid']) . ')' : $other_count;
      $ordered_rows[] = array($display_option, $other_text);
    }

    $rows = $ordered_rows;
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
      if ($option_value !== '') {
        $output .= _webform_filter_xss($option_value) . '<br />';
      }
    }
  }
  else {
    $output .= empty($value[0]) ? '' : _webform_filter_xss($value[0]);
  }
  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_select($component, $export_options) {
  $headers = array(
    0 => array(),
    1 => array(),
    2 => array(),
  );

  if ($component['extra']['multiple'] && $export_options['select_format'] == 'separate') {
    $headers[0][] = '';
    $headers[1][] = $component['name'];
    $items = _webform_select_options($component['extra']['items'], TRUE);
    if ($component['extra']['other_option']) {
      $other_label = !empty($component['extra']['other_text']) ? check_plain($component['extra']['other_text']) : t('Other...');
      $items[$other_label] = $other_label;
    }
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
function _webform_csv_data_select($component, $export_options, $value) {
  $options = _webform_select_options($component['extra']['items'], TRUE);
  $return = array();

  if ($component['extra']['multiple']) {
    foreach ($options as $key => $item) {
      $index = array_search($key, (array) $value);
      if ($index !== FALSE) {
        $return[] = ($export_options['select_format'] == 'separate') ? 'X' : $key;
        unset($value[$index]);
      }
      elseif ($export_options['select_format'] == 'separate') {
        $return[] = '';
      }
    }

    // Any remaining items in the $value array will be user-added options.
    if ($component['extra']['other_option']) {
      $return[] = count($value) ? implode(',', $value) : '';
    }
  }
  else {
    $return = $value[0];
  }

  if ($component['extra']['multiple'] && $export_options['select_format'] == 'compact') {
    $return = implode(',', (array) $return);
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
        $group = $filter ? _webform_filter_values($matches[1], NULL, NULL, NULL, FALSE) : $matches[1];
      }
    }
    elseif (preg_match('/^([^|]+)\|(.*)$/', $option, $matches)) {
      $key = $filter ? _webform_filter_values($matches[1], NULL, NULL, NULL, FALSE) : $matches[1];
      $value = $filter ? _webform_filter_values($matches[2], NULL, NULL, NULL, FALSE) : $matches[2];
      isset($group) ? $options[$group][$key] = $value : $options[$key] = $value;
    }
    else {
      $filtered_option = $filter ? _webform_filter_values($option, NULL, NULL, NULL, FALSE) : $option;
      isset($group) ? $options[$group][$filtered_option] = $filtered_option : $options[$filtered_option] = $filtered_option;
    }
  }
  return $options;
}

/**
 * Utility function to shuffle an array while preserving key-value pairs.
 */
function _webform_shuffle_options(&$array) {
  // First shuffle the array keys, then use them as the basis for ordering
  // the options.
  $aux = array();
  $keys = array_keys($array);
  shuffle($keys);
  foreach ($keys as $key) {
    $aux[$key] = $array[$key];
  }
  $array = $aux;
}
