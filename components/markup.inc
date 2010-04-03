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
      'format' => filter_default_format(),
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_markup($component) {
  $form = array();
  $form['value'] = array(
    '#type' => 'text_format',
    '#title' => t('Value'),
    '#default_value' => $component['value'],
    '#description' => t('Markup allows you to enter custom HTML or PHP logic into your form.') . theme('webform_token_help'),
    '#weight' => -1,
    '#format' => $component['extra']['format'],
    '#element_validate' => array('_webform_edit_markup_validate'),
  );

  // No description for markup.
  $form['extra']['description'] = array();

  return $form;
}

/**
 * Element validate handler; Set the text format value.
 */
function _webform_edit_markup_validate($form, &$form_state) {
  $form_state['values']['extra']['format'] = $form_state['values']['format'];
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
    '#prefix' => '<div class="webform-component webform-component-' . $component['type'] . '" id="webform-component-' . $component['form_key'] . '">',
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
