<?php
// $Id$

/**
 * @file
 * Webform module textarea component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_textarea() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'mandatory' => 0,
    'email' => 1,
    'extra' => array(
      'cols' => '',
      'rows' => '',
      'resizable' => 1,
      'disabled' => 0,
      'description' => '',
      'attributes' => array(),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_textarea($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'textarea',
    '#title' => t('Default value'),
    '#default_value' => $component['value'],
    '#description' => t('The default value of the field.') . theme('webform_token_help'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => 0,
  );
  $form['extra']['cols'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#default_value' => $component['extra']['cols'],
    '#description' => t('Width of the textfield.') .' '. t('Leaving blank will use the default size.'),
    '#size' => 5,
    '#maxlength' => 10,
  );
  $form['extra']['rows'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#default_value' => $component['extra']['rows'],
    '#description' => t('Height of the textfield.') .' '. t('Leaving blank will use the default size.'),
    '#size' => 5,
    '#maxlength' => 10,
  );
  $form['extra']['resizable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Resizable'),
    '#description' => t('Make this field resizable by the user.'),
    '#weight' => 2,
    '#default_value' => $component['extra']['resizable'],
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
function _webform_render_textarea($component, $value = NULL, $filter = TRUE) {
  $form_item = array(
    '#type'          => 'textarea',
    '#title'         => $component['name'],
    '#default_value' => $filter ? _webform_filter_values($component['value']) : $component['value'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#rows'          => !empty($component['extra']['rows']) ? $component['extra']['rows'] : 5,
    '#cols'          => !empty($component['extra']['cols']) ? $component['extra']['cols'] : 60,
    '#attributes'    => $component['extra']['attributes'],
    '#resizable'     => (bool) $component['extra']['resizable'], // MUST be FALSE to disable.
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  if ($component['extra']['disabled']) {
    $form_item['#attributes']['readonly'] = 'readonly';
  }

  if (isset($value)) {
    $form_item['#default_value'] = $value[0];
  }

  return $form_item;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_textarea($component, $value) {
  $form_item = _webform_render_textarea($component, $value);
  $form_item['#default_value'] = $value[0];
  $form_item['#attributes']['readonly'] = 'readonly';
  return $form_item;
}

/**
 * Implementation of _webform_form_builder_types_component().
 */
function _webform_form_builder_types_textarea() {
  $fields = array();

  $fields['textarea'] = array(
    'title' => t('Textarea'),
    'properties' => array(
      'title',
      'description',
      'default_value',
      'required',
      'rows',
      'cols',
      'input_format',
      'key',
    ),
    'default' => array(
      '#title' => t('New textarea'),
      '#type' => 'textarea',
    ),
  );

  return $fields;
}

/**
 * Implementation of _webform_form_builder_save_component().
 */
function _webform_form_builder_save_textarea($component, $form_element) {
  $component['extra']['description'] = $form_element['#description'];
  $component['extra']['cols'] = $form_element['#cols'];
  $component['extra']['rows'] = $form_element['#rows'];
  $component['extra']['disabled'] = empty($form_element['#disabled']) ? NULL : TRUE;
  $component['extra']['format'] = isset($form_element['#input_format']) ? $form_element['#input_format'] : NULL;
  return $component;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_textarea($component, $sids = array()) {
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
function _webform_table_textarea($component, $value) {
  return empty($value[0]) ? '' : check_plain($value[0]);
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_textarea($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_textarea($component, $value) {
  return empty($value[0]) ? '' : $value[0];
}
