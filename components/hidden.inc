<?php
// $Id$
/** 
 * function webform_edit_hidden
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_hidden ($currfield) {
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
	$editFields['mandatory'] = array(
		'#type' => 'hidden',
		'#value' => 1,
	);
	$editFields['extra']['description'] = array(); // Hide the description box
  return $editFields;
}

/** 
 * function webform_render_hidden
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_hidden($component) {
	$formItem = array (
    '#type'          => 'hidden',
    '#value' 				 => _webform_filtervalues($component['value']),
    '#weight'        => $component['weight'],
	);
	
	return $formItem;
}

/** 
 * function _webform_submission_data_hidden
 * Display the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_hidden ($data,$component) {
  $formItem = _webform_render_hidden($component);
  $formItem['#value']         = $data['value']['0'];
  $formItem['#type']          = 'textfield';
  $formItem['#title']         = $component['name']." (hidden)";
  $formItem['#attributes']    = array("disabled" => "disabled");
  return $formItem;
}

/** 
 * function _webform_help_hidden
 * Module specific instance of hook_help
 **/
function _webform_help_hidden($section) {
  switch ($section) {
    case 'admin/settings/webform#hidden_description':
      $output = t("Create a field which is not visible to the user, but is recorded with the submission.");
      break;
  }
  return $output;
}

/** 
 * function _webform_analysis_view_hidden
 * Calculate and returns statistics about results for this component from all submission to this webform. The output of this function will be displayed under the "results" tab then "analysis"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data rows, each containing a statistic for this component's submissions.
 **/
function _webform_analysis_rows_hidden ($component) {  
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
  
  $rows[0] = array( t('Empty'), ($submissions - $nonblanks));
  $rows[1] = array( t('Non-empty'), $nonblanks);
  $rows[2] = array( t('Average submission length in words (ex blanks)'), 
                    ($nonblanks!=0?($wordcount/$nonblanks):'0'));
  return $rows;
}

/** 
 * function _webform_table_data_hidden
 * Return the result of this component's submission for display in a table. The output of this function will be displayed under the "results" tab then "table"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_table_data_hidden ($data) {
  return empty($data['value']['0']) ? "" : $data['value']['0'];
}


/** 
 * function _webform_csv_headers_hidden
 * Return the header information for this component to be displayed in a comma seperated value file. The output of this function will be displayed under the "results" tab then "download"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data to be displayed in the first three rows of a CSV file, not including either prefixed or trailing commas
 **/
function _webform_csv_headers_hidden ($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/** 
 * function _webform_csv_data_hidden
 * Return the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for CSV, not including either prefixed or trailing commas
 **/
function _webform_csv_data_hidden ($data) {
  return empty($data['value']['0']) ? "" : $data['value']['0'];
}
