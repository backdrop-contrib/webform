<?php
// $Id$
/** 
 * function webform_edit_label
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_label ($currfield) {
  $editFields = array();
  $editFields['extra']['description'] = array(); // Do not render the description field
  $editFields['mandatory'] = array(); // Do not render the mandatory checkbox

  $editFields['value'] = array (
    '#type' => 'textfield',
    '#title' => t("Description"),
    '#default_value' => $currfield['default'],
    '#description' => t('A short description of the field used as help for the user when he/she uses the form.').'<br />'.webform_help('webform/helptext#variables'),
    '#size' => 60,
    '#maxlength' => 127,
  );
  return $editFields;
}

/** 
 * function webform_render_label
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_label ($component) {
  $formItem = array (
    '#type'          => 'markup',
    '#value'         => _webform_filtervalues($component['value']),
    '#weight'        => $component['weight'],
  );
  return $formItem;
}

/** 
 * function _webform_submission_data_label
 * Display the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_label ($data,$component) {
  $formItem = _webform_render_label($component);
  return $formItem;
}

/** 
 * function _webform_help_label
 * Module specific instance of hook_help
 **/
function _webform_help_label($section) {
  switch ($section) {
    case 'admin/settings/webform#label_description':
      $output = t("Presents a markup area of text. Does not render a field.");
      break;
  }
  return $output;
}
