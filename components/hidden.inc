<?php
// $Id$

/**
 * @file
 * Webform module hidden component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_hidden() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
    ),
  );
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_hidden() {
  return array(
    'webform_display_hidden' => array(
      'arguments' => array('element' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_hidden($component) {
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
  $form['advanced']['mandatory'] = array(
    '#type' => 'hidden',
    '#value' => 1,
  );
  $form['extra']['description'] = array(); // Hide the description box.
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_hidden($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#type'          => 'hidden',
    '#title'         => $component['name'],
    '#default_value' => $filter ? _webform_filter_values($component['value']) : $component['value'],
    '#weight'        => $component['weight'],
  );

  if (isset($value[0])) {
    $element['#default_value'] = $value[0];
  }

  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_hidden($component, $value, $format = 'html') {
  $element = array(
    '#title' => t('!name (hidden)', array('!name' => $component['name'])),
    '#value' => isset($value[0]) ? $value[0] : NULL,
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_hidden',
    '#component' => $component,
    '#format' => $format,
    '#theme' => 'webform_display_hidden',
    '#theme_wrappers' => $format == 'text' ? array('webform_element_text') : array('webform_element'),
    '#post_render' => array('webform_element_wrapper'),
    '#access' => user_access('edit all webform submissions') || user_access('access all webform results'),
  );
  return $element;
}

function theme_webform_display_hidden($element) {
  return $element['#format'] == 'html' ? check_plain($element['#value']) : $element['#value'];
}

/**
 * Implementation of _webform_form_builder_preview_alter_component().
 *
 * Make hidden fields visible while editing.
 */
function _webform_form_builder_preview_alter_hidden($form_element) {
  $form_element['#field_suffix'] = '(' . t('hidden') . ')';
  $form_element['#autocomplete_path'] = NULL; // Needed to avoid notices.
  $form_element['#type'] = 'textfield';
  return $form_element;
}

/**
 * Implementation of _webform_form_builder_types_component().
 */
function _webform_form_builder_types_hidden() {
  $fields = array();

  $fields['hidden'] = array(
    'title' => t('Hidden'),
    'properties' => array(
      'title',
      'default_value',
      'key',
    ),
    'default' => array(
      '#title' => t('Hidden field'),
      '#type' => 'hidden',
    ),
  );

  return $fields;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_hidden($component, $sids = array()) {
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
    if ( strlen(trim($data['data'])) > 0 ) {
      $nonblanks++;
      $wordcount += str_word_count(trim($data['data']));
    }
    $submissions++;
  }

  $rows[0] = array( t('Empty'), ($submissions - $nonblanks));
  $rows[1] = array( t('Non-empty'), $nonblanks);
  $rows[2] = array( t('Average submission length in words (ex blanks)'),
                    ($nonblanks !=0 ? number_format($wordcount/$nonblanks, 2) : '0'));
  return $rows;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_table_hidden($component, $value) {
  return check_plain(empty($value[0]) ? '' : $value[0]);
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_headers_hidden($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_hidden($component, $value) {
  return empty($value[0]) ? '' : $value[0];
}
