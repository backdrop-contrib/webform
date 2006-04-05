<?php
/** 
 * function webform_edit_textfield
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_textfield ($currfield) {
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
    $editFields['extra']['width'] = array (
      '#type' => 'textfield',
      '#title' => t("Width"),
      '#default_value' => $currfield['extra']['width'],
      '#description' => t('Width of the textfield.'),
      '#size' => 5,
      '#maxlength' => 10,
      '#weight' => 0,
    );
    $editFields['extra']['attributes']['disabled'] = array (
      '#type' => 'checkbox',
      '#title' => t("Disabled"),
      '#return_value' => 'disabled',
      '#description' => t('Make this field non-editable. Useful for setting an unchangeable default value.'),
      '#weight' => 3,
      '#default_value' => $currfield['extra']['attributes']['disabled'],
    );
    return $editFields;
}

/** 
 * function webform_render_textfield
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_textfield ($component) {
	$formItem = array (
    '#type'          => $component['type'],
    '#title'         => $component['name'],
    '#default_value' => _webform_filtervalues($component['value']),
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#attributes'    => $component['extra']['attributes'],
	);
	// Change the 'width' option to the correct 'size' option
	if ($component['extra']['width'] > 0) {
		$formItem['#size'] = $component['extra']['width'];
	}
	
	return $formItem;
}
?>
