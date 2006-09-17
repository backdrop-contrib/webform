<?php
// $Id$
/** 
 * function webform_edit_email
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_email ($currfield) {
  $editFields = array();
  $editFields['value'] = array (
    '#type' => 'checkbox',
    '#title' => t("User email as default"),
    '#default_value' => $currfield['default'] ? $currfield['default'] : 'user email',
    '#return_value' => 'user email',
    '#description' => t('Set the default value of this field to the user email, if he/she is logged in.'),
    '#weight' => 0,
  );
  $editFields['extra']['carboncopy'] = array (
    '#type' => 'checkbox',
    '#title' => t("CC submission to this email"),
    '#return_value' => 'Y',
    '#default_value' => ($currfield['extra']['carboncopy']=='Y'?TRUE:FALSE),
    '#description' => t('Check this option if the email specified in this component should get a CC submission.'). ' '.
                      t('Note that this opens the risk that the form can be used to send emails to any address and might be missused as a spam gateway.'),
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
function _webform_render_email ($component) {
  global $user;
  $formItem = array (
    '#type'          => 'textfield',
    '#title'         => htmlspecialchars($component['name'], ENT_QUOTES),
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#prefix'        => '<div class="webform-component-'.$component['type'].'" id="webform-component-'._webform_safe_name($component['name']).'">',
    '#suffix'        => '</div>',
    '#validate'      => array('_webform_validate_email' => array($component['cid'])),
  );
  
  // Fill in the user's email address if available
  if($user->uid && $user->mail && $component['value'] == 'user email') {
    $formItem['#default_value'] = $user->mail;
  }
  
  // Change the 'width' option to the correct 'size' option
  if ($component['extra']['width'] > 0) {
    $formItem['#size'] = $component['extra']['width'];
  }
    
  return $formItem;
}

/** 
 * function webform_validate_email
 * A Drupal Forms API Validation function. Validates the entered values from email components on the client-side form.
 * @param $formelement An array of information describing the component, directly correlating to the webform_component database schema
 * @returns True if successful, calls a form_set_error on the field if the email is not valid.
 **/
function _webform_validate_email ($formelement, $name) {
  if (!empty($formelement['#value']) && !valid_email_address($formelement['#value'])) {
    form_set_error($name,t("'%value' is not a valid email address",array('%value' => $formelement['#value'])));
  }
}

/** 
 * function _webform_submission_data_email
 * Display the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_email ($data, $component) {
  $formItem = _webform_render_email($component);
  $formItem['#default_value'] = $data['value']['0'];
  $formItem['#attributes']    = array("disabled" => "disabled");
  return $formItem;
}

/** 
 * function _webform_help_email
 * Module specific instance of hook_help
 **/
function _webform_help_email($section) {
  switch ($section) {
    case 'admin/settings/webform#email_description':
      $output = t("A textfield that automatically fills in logged-in users e-mail.");
      break;
  }
  return $output;
}

/** 
 * function _webform_analysis_view_email
 * Calculate and returns statistics about results for this component from all submission to this webform. The output of this function will be displayed under the "results" tab then "analysis"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data rows, each containing a statistic for this component's submissions.
 **/
function _webform_analysis_rows_email ($component) {  
  $query = 'SELECT data '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d';
  $nonblanks = 0;
  $submissions = 0;
  $wordcount = 0;

  $result = db_query($query, $component['nid'], $component['cid']);
  while ($data = db_fetch_array($result)) {
    if ( strlen(trim($data['data'])) > 0 ) {
      $nonblanks++;
      $wordcount += str_word_count(trim($data['data']));
    }
    $submissions++;
  }
  
  $rows[0] = array( t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array( t('User entered value'), $nonblanks);
  $rows[2] = array( t('Average submission length in words (ex blanks)'), 
                    ($nonblanks !=0 ? number_format($wordcount/$nonblanks,2) : '0'));
  return $rows;
}

/** 
 * function _webform_table_data_email
 * Return the result of this component's submission for display in a table. The output of this function will be displayed under the "results" tab then "table"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_table_data_email ($data) {
  return check_plain(empty($data['value']['0']) ? "" : $data['value']['0']);
}


/** 
 * function _webform_csv_headers_email
 * Return the header information for this component to be displayed in a comma seperated value file. The output of this function will be displayed under the "results" tab then "download"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data to be displayed in the first three rows of a CSV file, not including either prefixed or trailing commas
 **/
function _webform_csv_headers_email ($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/** 
 * function _webform_csv_data_email
 * Return the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for CSV, not including either prefixed or trailing commas
 **/
function _webform_csv_data_email ($data) {
  return empty($data['value']['0']) ? "" : $data['value']['0'];
}
