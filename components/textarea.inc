<?php
// $Id$
/** 
 * function webform_edit_textarea
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_textarea ($currfield) {
  $editFields = array();
  $editFields['value'] = array (
    '#type' => 'textarea',
    '#title' => t("Default value"),
    '#default_value' => $currfield['default'],
    '#description' => t('The default value of the field.').'<br />'.webform_help('webform/helptext#variables'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => 0,
  );
  $editFields['extra']['cols'] = array (
    '#type' => 'textfield',
    '#title' => t("Width"),
    '#default_value' => $currfield['extra']['cols'],
    '#description' => t('Width of the textfield.'),
    '#size' => 5,
    '#maxlength' => 10,
  );
  $editFields['extra']['rows'] = array (
    '#type' => 'textfield',
    '#title' => t("Height"),
    '#default_value' => $currfield['extra']['rows'],
    '#description' => t('Height of the textfield.'),
    '#size' => 5,
    '#maxlength' => 10,
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
 * function webform_render_textarea
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_textarea ($component) {
  $formItem = array (
    '#type'          => "textarea",
    '#title'         => htmlspecialchars($component['name'], ENT_QUOTES),
    '#default_value' => _webform_filtervalues($component['value']),
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#rows'          => $component['extra']['rows'],
    '#cols'          => $component['extra']['cols'],
    '#attributes'    => $component['extra']['attributes'],
	'#prefix'        => '<div class="webform-component-'.$component['type'].'" id="webform-component-'._webform_safe_name($component['name']).'">',
	'#suffix'        => '</div>',
  );
  if ($component['extra']['rows'] > 0) {
    $formItem['#rows'] = $component['extra']['rows'];
  }
  if ($component['extra']['cols'] > 0) {
    $formItem['#cols'] = $component['extra']['cols'];
  }

  return $formItem;
}

/** 
 * function _webform_submission_data_textarea
 * Display the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_textarea ($data,$component) {
  $formItem = _webform_render_textarea($component);
  $formItem['#default_value'] = $data['value']['0'];
  $formItem['#attributes']    = array("disabled" => "disabled");
  return $formItem;
}

/** 
 * function _webform_help_textarea
 * Module specific instance of hook_help
 **/
function _webform_help_textarea($section) {
  switch ($section) {
    case 'admin/settings/webform#textarea_description':
      $output = t("A large text area that allows for multiple lines of input.");
      break;
  }
  return $output;
}

/** 
 * function _webform_analysis_view_textarea
 * Calculate and returns statistics about results for this component from all submission to this webform. The output of this function will be displayed under the "results" tab then "analysis"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data rows, each containing a statistic for this component's submissions.
 **/
function _webform_analysis_rows_textarea ($component) {  
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
                    ($nonblanks!=0?($wordcount/$nonblanks):'0'));
  return $rows;
}

/** 
 * function _webform_table_data_textarea
 * Return the result of this component's submission for display in a table. The output of this function will be displayed under the "results" tab then "table"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_table_data_textarea ($data) {
  return empty($data['value']['0']) ? "" : check_markup($data['value']['0'], FILTER_HTML_STRIP, FALSE);
}


/** 
 * function _webform_csv_headers_textarea
 * Return the header information for this component to be displayed in a comma seperated value file. The output of this function will be displayed under the "results" tab then "download"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data to be displayed in the first three rows of a CSV file, not including either prefixed or trailing commas
 **/
function _webform_csv_headers_textarea ($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/** 
 * function _webform_csv_data_textarea
 * Return the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for CSV, not including either prefixed or trailing commas
 **/
function _webform_csv_data_textarea ($data) {
  return empty($data['value']['0']) ? "" : $data['value']['0'];
}
