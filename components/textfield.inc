<?php
// $Id$

/**
 * @file
 * Webform module textfield component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_textfield() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'email' => 1,
    'extra' => array(
      'width' => '',
      'maxlength' => '',
      'field_prefix' => '',
      'field_suffix' => '',
      'disabled' => 0,
      'description' => '',
      'attributes' => array(),
    ),
  );
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_textfield() {
  return array(
    'webform_display_textfield' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_textfield($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'textfield',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.') . theme('webform_token_help'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
  );
  $form['extra']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#default_value' => $component['extra']['width'],
    '#description' => t('Width of the textfield.') .' '. t('Leaving blank will use the default size.'),
    '#size' => 5,
    '#maxlength' => 10,
    '#weight' => 0,
  );
  $form['extra']['maxlength'] = array(
    '#type' => 'textfield',
    '#title' => t('Maxlength'),
    '#default_value' => $component['extra']['maxlength'],
    '#description' => t('Maxlength of the textfield.'),
    '#size' => 5,
    '#maxlength' => 10,
    '#weight' => 0,
  );
  $form['extra']['field_prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Label placed to the left of the textfield'),
    '#default_value' => $component['extra']['field_prefix'],
    '#description' => t('Examples: $, #, -.'),
    '#size' => 20,
    '#maxlength' => 127,
    '#weight' => 1.1,
  );
  $form['extra']['field_suffix'] = array(
    '#type' => 'textfield',
    '#title' => t('Label placed to the right of the textfield'),
    '#default_value' => $component['extra']['field_suffix'],
    '#description' => t('Examples: lb, kg, %.'),
    '#size' => 20,
    '#maxlength' => 127,
    '#weight' => 1.2,
  );
  $form['extra']['disabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disabled'),
    '#return_value' => 1,
    '#description' => t('Make this field non-editable. Useful for setting an unchangeable default value.'),
    '#weight' => 3,
    '#default_value' => $component['extra']['disabled'],
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_textfield($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#type'          => $component['type'],
    '#title'         => $component['name'],
    '#default_value' => $filter ? _webform_filter_values($component['value']) : $component['value'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#field_prefix'  => empty($component['extra']['field_prefix']) ? NULL : check_plain($component['extra']['field_prefix']),
    '#field_suffix'  => empty($component['extra']['field_suffix']) ? NULL : check_plain($component['extra']['field_suffix']),
    '#description'   => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#attributes'    => $component['extra']['attributes'],
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  if ($component['extra']['disabled']) {
    $element['#attributes']['readonly'] = 'readonly';
  }

  // Change the 'width' option to the correct 'size' option.
  if ($component['extra']['width'] > 0) {
    $element['#size'] = $component['extra']['width'];
  }
  if ($component['extra']['maxlength'] > 0) {
    $element['#maxlength'] = $component['extra']['maxlength'];
  }

  if (isset($value)) {
    $element['#default_value'] = $value[0];
  }

  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_textfield($component, $value, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_textfield',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#field_prefix' => $component['extra']['field_prefix'],
    '#field_suffix' => $component['extra']['field_suffix'],
    '#component' => $component,
    '#format' => $format,
    '#value' => isset($value[0]) ? $value[0] : '',
  );
}

/**
 * Implementation of _webform_form_builder_save_component().
 */
function _webform_form_builder_save_textfield($component, $form_element) {
  $component['extra']['width'] = isset($form_element['#size']) ? $form_element['#size'] : NULL;
  $component['extra']['maxlength'] = isset($form_element['#maxlength']) ? $form_element['#maxlength'] : NULL;
  $component['extra']['field_prefix'] = isset($form_element['#field_prefix']) ? $form_element['#field_prefix'] : NULL;
  $component['extra']['field_suffix'] = isset($form_element['#field_suffix']) ? $form_element['#field_suffix'] : NULL;
  $component['extra']['description'] = isset($form_element['#description']) ? $form_element['#description'] : NULL;
  $component['extra']['disabled'] = isset($form_element['#disabled']) ? $form_element['#disabled'] : FALSE;
  return $component;
}

/**
 * Implementation of _webform_form_builder_types_component().
 */
function _webform_form_builder_types_textfield() {
  $fields = array();

  $fields['textfield'] = array(
    'title' => t('Textfield'),
    'properties' => array(
      'title',
      'description',
      'field_prefix',
      'field_suffix',
      'default_value',
      'required',
      'size',
      'key',
    ),
    'default' => array(
      '#title' => t('New textfield'),
      '#type' => 'textfield',
    ),
  );

  return $fields;
}

/**
 * Format the output of data for this component.
 */
function theme_webform_display_textfield($element) {
  $prefix = $element['#format'] == 'html' ? filter_xss($element['#field_prefix']) : $element['#field_prefix'];
  $suffix = $element['#format'] == 'html' ? filter_xss($element['#field_suffix']) : $element['#field_suffix'];
  $value = $element['#format'] == 'html' ? check_plain($element['#value']) : $element['#value'];
  return $value ? ($prefix . $value . $suffix) : ' ';
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_textfield($component, $sids = array()) {
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (".implode(",", $placeholders).")" : "";
  $query = 'SELECT data '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '. $sidfilter;
  $nonblanks = 0;
  $submissions = 0;
  $wordcount = 0;

  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));
  while ($data = db_fetch_array($result)) {
    if (drupal_strlen(trim($data['data'])) > 0) {
      $nonblanks++;
      $wordcount += str_word_count(trim($data['data']));
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User entered value'), $nonblanks);
  $rows[2] = array(t('Average submission length in words (ex blanks)'), ($nonblanks != 0 ? number_format($wordcount/$nonblanks, 2) : '0'));
  return $rows;
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_textfield($component, $value) {
  return check_plain(empty($value[0]) ? '' : $value[0]);
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_textfield($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_textfield($component, $value) {
  return !isset($value[0]) ? '' : $value[0];
}
