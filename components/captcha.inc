<?php
/** 
 * function webform_edit_captcha
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_captcha ($currfield) {
	if (!module_exist("captcha")) {
		drupal_set_message(t("Using captcha components in webform requires the <a href='http://drupal.org/project/captcha'>Captcha</a> and <a href='http://drupal.org/node/52349'>Textimage</a> modules."),"error");
	}
	
	$editFields = array();
	$editFields['value'] = array ();
	$editFields['mandatory'] = array (
		'#type' => 'hidden',
		'#value' => 1,
	);
	$editFields['mandatory-display'] = array (
		'#type' => 'checkbox',
		'#title' => t("Mandatory"),
		'#default_value' => TRUE,
		'#attributes' => array('disabled' => 'disabled'),
		'#description' => t('Captchas are always mandatory'),
		'#weight' => 2,
	);
	return $editFields;
}

/** 
 * function webform_render_captcha
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_captcha ($component) {
	if (module_exist("captcha")) {
		// Create a dummy form to pass to the captcha function
		$dummyForm = array();
		// Pass the form to the captcha rendering function
		$captcha_type = variable_get("captcha_type", NULL);
		if (module_hook($captcha_type, 'captchachallenge')) {
			call_user_func_array($captcha_type.'_captchachallenge', array(&$dummyForm, &$_SESSION['captcha']));   
		}
		// Set a custom validation function (defined below)
		$dummyForm['captcha_response']['#validate'] = array ("_webform_validate_captcha" => array());
		// Extract the created captcha into formItem to be returned to webform
		$formItem = $dummyForm['captcha_response'];
		
		// Override the default title
		$formItem['#title'] = _webform_filtervalues($component['name']);
		
		// Override the default captcha description if the user has defined something different
		if (strlen(trim($component['extra']['description'])) > 0) {
			$formItem['#description'] = _webform_filtervalues($component['extra']['description']);
		}
	} else {
		drupal_set_message(t("Unable to render captcha field because the captcha module is not available."),"error");
	}
	return $formItem;
}


function _webform_validate_captcha($captcha_response) {
  // This function copied directly from the captcha module.
  // The below line is commented out to require every submission to use a captcha,
  // no caching correct answers allowed, lest one real user enter the captcha
  // and then send a bot to churn out spam.

	//if ($_SESSION['captcha_correct']) return TRUE; // Potentially Dangerous
	if (is_array($captcha_response)) $captcha_response = $captcha_response['#value'];
	if (trim($captcha_response) == '') return FALSE;

	global $user;
	$captcha_type = variable_get("captcha_type", NULL);
	$trigger = NULL;

	if (module_hook($captcha_type, 'captchavalidate')) {
		call_user_func_array($captcha_type.'_captchavalidate', array(&$captcha_response, &$_SESSION['captcha_correct']));
	}
	
	return $_SESSION['captcha_correct'];
}

function _webform_help_captcha($section) {
  switch ($section) {
    case 'admin/settings/webform#captcha_description':
      $output = t("Require a question to prove the user is human. Requires <a href='http://drupal.org/project/captcha'>Captcha</a> module.");
      break;
  }
  return $output;
}

/** 
 * function _webform_submission_display_captcha
 * Display the result of a captcha submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_captcha ($data,$component) {
  $formItem = array();
  return $formItem;
}

?>
