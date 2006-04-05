<?php
/** 
 * function webform_edit_hidden
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_hidden ($currfield) {
    $editFields = array();
    $editFields['value'] = array (
      '#type' => 'textfield',
      '#title' => t("Default value"),
      '#default_value' => $currfield['default'],
      '#description' => t('The default value of the field.').'<br />'.webform_help('webform/helptext#variables'),
      '#size' => 60,
      '#maxlength' => 127,
      '#weight' => 0,
    );
    return $editFields;
}

/** 
 * function webform_render_hidden
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_hidden($component) {
	$formItem = array (
    '#type'          => 'hidden',
    '#value' 				 => _webform_filtervalues($component['value']),
    '#weight'        => $component['weight'],
	);
	
	return $formItem;
}
?>
