<?php
// $Id$

/**
 * @file
 * Webform module markup component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_markup() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'extra' => array(
      'format' => FILTER_FORMAT_DEFAULT,
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_markup($component) {
  $form = array();
  $form['advanced']['mandatory'] = array(); // Do not render the mandatory checkbox.
  $form['value'] = array(
    '#type' => 'textarea',
    '#title' => t('Value'),
    '#default_value' => $component['value'],
    '#description' => t('Markup allows you to enter custom HTML or PHP logic into your form.') . theme('webform_token_help'),
    '#weight' => -1,
  );
  // Add the filter form.
  $form['extra']['format'] = filter_form($component['extra']['format'], 0, array('extra', 'format'));

  // No description for markup.
  $form['extra']['description'] = array();

  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_markup($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#type'   => 'markup',
    '#title'  => $component['name'],
    '#weight' => $component['weight'],
    '#markup' => $filter ? _webform_filter_values(check_markup($component['value'], $component['extra']['format'], FALSE), NULL, NULL, NULL, FALSE) : $component['value'],
    '#input_format' => $component['extra']['format'],
    '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix' => '</div>',
  );

  // TODO: Remove when #markup becomes available in D7.
  $element['#value'] = $element['#markup'];

  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_markup($component, $value, $format = 'html') {
  return array();
}

/**
 * Implementation of _webform_form_builder_save_component().
 */
function _webform_form_builder_save_markup($component, $form_element) {
  $component['extra']['format'] = $form_element['#input_format'];
  $component['value'] = $form_element['#markup'];
  return $component;
}

/**
 * Module specific instance of hook_form_builder_preview_alter().
 */
function _webform_form_builder_preview_alter_markup($form_element) {
  $form_element['#markup'] = isset($form_element['#markup']) ? $form_element['#markup'] : '';
  $form_element['#value'] = check_markup(_webform_filter_values($form_element['#markup'], NULL, NULL, NULL, FALSE), $form_element['#input_format'], FALSE);

  if (strlen(trim($form_element['#value'])) == 0) {
    $form_element['#value'] = t('This markup element currently does not contain any text.');
  }
  return $form_element;
}

/**
 * Module specific instance of hook_form_builder_types().
 */
function _webform_form_builder_types_markup() {
  $fields = array();

  $default = '';
  $default .= '<p><strong>' . t('New HTML Markup') . '</strong></p>' . "\n\n";
  $default .= '<p>' . t('Enter any text supported by available input formats.') . '</p>';

  $fields['markup'] = array(
    'title' => t('Markup'),
    'properties' => array(
      'title',
      'markup',
      'input_format',
      'key',
    ),
    'default' => array(
      '#title' => t('Markup'),
      '#type' => 'markup',
      '#markup' => $default,
      '#input_format' => FILTER_FORMAT_DEFAULT,
    ),
  );

  return $fields;
}
