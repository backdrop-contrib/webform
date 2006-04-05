<?php
/** 
 * function webform_edit_email
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_email ($currfield) {
    $editFields = array();
    $editFields['value'] = array (
      '#type' => 'markup',
      '#value' => '<label for="edit-default"><b>'.t("Default value").':</b></label>'.
                  '<div id="edit-default">'.
                  t('The default value of a email field is always the users email address if he/she is logged in.').
                  '</div>',
    );
    $editFields['extra']['width'] = array (
      '#type' => 'textfield',
      '#title' => t("Width"),
      '#default_value' => $currfield['extra']['width'],
      '#description' => t('Width of the textfield.'),
      '#size' => 5,
      '#maxlength' => 10,
    );
    return $editFields;
}

/** 
 * function webform_render_email
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_email ($component) {
	global $user;
	$formItem = array (
    '#type'          => 'textfield',
    '#title'         => $component['name'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
	);
	
    // Fill in the user's email address if available
    if($user->uid && $user->mail && empty($value)) {
      $formItem['#default_value'] = $user->mail;
    }
    
    // Change the 'width' option to the correct 'size' option
    if ($component['extra']['width'] > 0) {
      $formItem['#size'] = $component['extra']['width'];
    }
    // Add the email filter validation function
    $formItem['#validate'] = array('webform_validate_email' => array());
    
	return $formItem;
}

/** 
 * function webform_validate_email
 * A Drupal Forms API Validation function. Validates the entered values from email components on the client-side form.
 * @param $formelement An array of information describing the component, directly correlating to the webform_component database schema
 * @returns True if successful, calls a form_set_error on the field if the email is not valid.
 **/
function webform_validate_email ($formelement) {
  if (!valid_email_address($formelement['#value'])) {
    $elementName = substr($formelement['#name'],(strrpos($formelement['#name'],'[')+1),-1);
    form_set_error($formelement['#name'],t("Please enter a valid email address"));
  } 
  return true;
}
?>
