<?php
// $Id$
/** 
 * function webform_edit_fieldset
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_fieldset ($currfield) {
  $editFields = array();
  $editFields['extra']['collapsible'] = array (
    '#type' => 'checkbox',
    '#title' => t("Collapsible"),
    '#default_value' => $currfield['extra']['collapsible'],
    '#description' => t('If this fieldset is collapsible, the user may open or close the fieldset.'),
    '#weight' => 0,
  );
  $editFields['extra']['collapsed'] = array (
    '#type' => 'checkbox',
    '#title' => t("Collapsed by Default"),
    '#default_value' => $currfield['extra']['collapsed'],
    '#description' => t('Collapsible fieldsets are "open" by default. Select this option to default the fieldset to "closed."'),
    '#weight' => 3,
  );
  $editFields['mandatory'] = array();
  return $editFields;
}

/** 
 * function webform_render_fieldset
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_fieldset ($component) {
  $formItem = array (
    '#type'          => $component['type'],
    '#title'         => $component['name'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#collapsible'   => $component['extra']['collapsible'],
    '#collapsed'     => $component['extra']['collapsed'],
    '#attributes'    => array("class" => "webform-component-".$component['type'], "id" => "webform-component-".$component['cid']),
  );
  // Change the 'width' option to the correct 'size' option
  if ($component['extra']['width'] > 0) {
    $formItem['#size'] = $component['extra']['width'];
  }
  
  return $formItem;
}

/** 
 * function _webform_submission_display_fieldset
 * Display the result of a fieldset submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_fieldset ($data,$component) {
  $formItem = _webform_render_fieldset($component);
  return $formItem;
}

/** 
 * function _webform_help_fieldset
 * Module specific instance of hook_help
 **/
function _webform_help_fieldset($section) {
  switch ($section) {
    case 'admin/settings/webform#fieldset_description':
      $output = t("Fieldsets allow you to organize complex webforms into groups of fields.");
      break;
  }
  return $output;
}
