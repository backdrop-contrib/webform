<?php
// $Id$

/**
 * @file
 * Webform module page break component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_pagebreak() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'extra' => array(),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_pagebreak($component) {
  drupal_set_message(t('Use caution when combining the pagebreak and file components. File components will not be submitted unless they are after the last pagebreak in the form.'), 'warning');

  $form = array();
  // No description.
  $form['extra']['description'] = array(
      '#type' => 'hidden',
      '#value' => '',
  );
  // Force the parent to always be root.
  $form['advanced']['pid'] = array(
      '#type' => 'hidden',
      '#value' => '0',
  );
  // Never mandatory.
  $form['advanced']['mandatory'] = array(
      '#type' => 'hidden',
      '#value' => '',
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_pagebreak($component, $value = NULL, $filter = TRUE) {
  $form_item = array();
  // Render page breaks as hidden elements so that they can be displayed in
  // emails as separators.
  $form_item = array(
    '#type' => 'hidden',
    '#value' => $component['name'],
    '#weight' => $component['weight'],
  );
  return $form_item;
}

/**
 * Implementation of _webform_theme_component().
 */
function _webform_theme_pagebreak() {
  return array(
    'webform_mail_pagebreak' => array(
      'arguments' => array('component' => NULL, 'value' => NULL),
    ),
  );
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
function theme_webform_mail_pagebreak($component, $value) {
  return "\n-- ". $component['name'] ." --\n";
}
