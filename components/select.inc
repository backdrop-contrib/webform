<?php
/** 
 * function webform_edit_select
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_select ($currfield) {
    $editFields = array();
    $editFields['extra']['items'] = array (
      '#type' => 'textarea',
      '#title' => t("Options"),
      '#default_value' => $currfield['extra']['items'],
      '#description' => t('A list of selectable options. One option per line.').'<br />'.webform_help('webform/helptext#variables'),
      '#cols' => 60,
      '#rows' => 5,
      '#weight' => -2,
      '#required' => TRUE,
    );
    $editFields['value'] = array (
      '#type' => 'textfield',
      '#title' => t("Default value"),
      '#default_value' => $currfield['default'],
      '#description' => t('The default value of the field.').'<br />'.webform_help('webform/helptext#variables'),
      '#size' => 60,
      '#maxlength' => 127,
      '#weight' => 0,
    );
    $editFields['extra']['multiple'] = array (
      '#type' => 'checkbox',
      '#title' => t("Multiple"),
      '#return_value' => 'Y',
      '#default_value' => ($currfield['extra']['multiple']=='Y'?TRUE:FALSE),
      '#description' => t('Check this option if the user should be allowed to choose multiple values.'),
    );
    $editFields['extra']['aslist'] = array (
      '#type' => 'checkbox',
      '#title' => t("Listbox"),
      '#return_value' => 'Y',
      '#default_value' => ($currfield['extra']['aslist']=='Y'?TRUE:FALSE),
      '#description' => t('Check this option if you want the select component to be of listbox type instead of radiobuttons or checkboxes.'),
    );
    return $editFields;
}

/** 
 * function webform_render_select
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_select ($component) {
	$formItem = array (
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
	);
	
	// Convert the user-entered list into an array
	$value = _webform_filtervalues($component['value']);
	$items = explode("\n", _webform_filtervalues($component['extra']['items']));
	$options = array();
	foreach($items as $k => $v) {
		$v = trim($v);
		if ( strlen($v) )
			$options[$v] = $v;
	}
	// Set the component options
	$formItem['#options'] = $options;
	
	if ( $component['extra']['aslist'] == 'Y' ) {
		// Set display as a select list:
		$formItem['#default_value'] = array($value);
		$formItem['#type'] = 'select';
		if ( $component['extra']['multiple'] == 'Y' ) {
			$formItem['#multiple'] = TRUE;
		}
	} else {
		if ( $component['extra']['multiple'] == 'Y' ) {
			// Set display as a checkbox set
			$formItem['#default_value'] = array($value);
			$formItem['#type'] = 'checkboxes';
			
		} else {
			// Set display as a radio set
			$formItem['#default_value'] = $value;
			$formItem['#type'] = 'radios';
		}
	}
	return $formItem;
}
?>
