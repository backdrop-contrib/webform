<?php
// $Id$

/**
 * @file
 *   Webform module markup component.
 */

/**
 * Create a default markup component.
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
 * Create a set of form items to be displayed on the form for editing this
 * component. Use care naming the form items, as this correlates directly to the
 * database schema. The component "Name" and "Description" fields are added to
 * every component type and are not necessary to specify here (although they may
 * be overridden if desired).
 * @return
 *   An array of form items to be displayed on the edit component page.
 */
function _webform_edit_markup($currfield) {
  $edit_fields = array();
  $edit_fields['advanced']['mandatory'] = array(); // Do not render the mandatory checkbox.
  $edit_fields['value'] = array(
    '#type' => 'textarea',
    '#title' => t('Value'),
    '#default_value' => $currfield['value'],
    '#description' => t('Markup allows you to enter custom HTML or PHP logic into your form.') . theme('webform_token_help'),
    '#weight' => -1,
  );
  // Add the filter form.
  $edit_fields['extra']['format'] = filter_form($currfield['extra']['format'], 0, array('extra', 'format'));

  // No description for markup.
  $edit_fields['extra']['description'] = array();

  return $edit_fields;
}

/**
 * Build a form item array containing all the properties of this component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of a form item to be displayed on the client-side webform.
 */
function _webform_render_markup($component, $filter) {
  $form_item = array(
    '#type'   => 'markup',
    '#title'  => $component['name'],
    '#weight' => $component['weight'],
    '#markup' => $filter ? _webform_filter_values(check_markup($component['value'], $component['extra']['format'], FALSE), NULL, NULL, FALSE) : $component['value'],
    '#input_format' => $component['extra']['format'],
    '#prefix' => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix' => '</div>',
  );

  // TODO: Remove when #markup becomes available in D7.
  $form_item['#value'] = $form_item['#markup'];

  return $form_item;
}

/**
 * Display the markup  in the results. The output of this function will be
 * displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_submission_display_markup($data, $component, $enabled = FALSE) {
  $form_item = _webform_render_markup($component);
  return $form_item;
}

/**
 * Convert a FAPI form element into settings savable in a component.
 */
function _webform_form_builder_save_markup(&$component, $form_element) {
  $component['extra']['format'] = $form_element['#input_format'];
  $component['value'] = $form_element['#markup'];
}

/**
 * Module specific instance of hook_form_builder_preview_alter().
 */
function _webform_form_builder_preview_alter_markup(&$form_element) {
  $form_element['#markup'] = isset($form_element['#markup']) ? $form_element['#markup'] : '';
  $form_element['#value'] = check_markup(_webform_filter_values($form_element['#markup'], NULL, NULL, FALSE), $form_element['#input_format'], FALSE);

  if (strlen(trim($form_element['#value'])) == 0) {
    $form_element['#value'] = t('This markup element currently does not contain any text.');
  }
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

/**
 * Module specific instance of hook_help().
 */
function _webform_help_markup($section) {
  switch ($section) {
    case 'admin/settings/webform#markup_description':
      return t('Displays text as HTML in the form; does not render a field.');
  }
}
