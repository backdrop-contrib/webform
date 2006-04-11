<?php
/** 
 * function webform_edit_label
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_label ($currfield) {
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
function webform_render_label ($component) {
	$formItem = array (
    '#type'          => 'markup',
    '#value'				 => _webform_filtervalues($component['value']),
    '#weight'        => $component['weight'],
	);
	return $formItem;
}
?>
