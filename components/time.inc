<?php
/** 
 * function webform_edit_time
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function _webform_edit_time ($currfield) {
    $editFields = array();
    $editFields['value'] = array (
      '#type' => 'textfield',
      '#title' => t("Default value"),
      '#default_value' => $currfield['default'],
      '#description' => t('The default value of the field.').'<br />'.t('Accepts a time in any <a href="http://www.gnu.org/software/tar/manual/html_node/tar_109.html">GNU Date Input Format</a>. Strings such as now, +2 hours, and 10:30pm are all valid.'),
      '#size' => 60,
      '#maxlength' => 127,
      '#weight' => 0,
      '#validate' => array ('webform_validate_time_string' => array ()),
    );
    $editFields['extra']['timezone'] = array (
      '#type' => 'radios',
      '#title' => t("Timezone"),
      '#default_value' => empty($currfield['extra']['timezone']) ? "site" : $currfield['extra']['timezone'],
      '#description' => t('Adjust the time according to a specific timezone. Website timezone is defined in the <a href="%settings">Site Settings</a> and is the default.',array('%settings' => url('admin/settings'))),
      '#options' => array('site' => 'Website Timezone', 'user' => 'User Timezone', 'gmt' => 'GMT'),
      '#weight' => 0,
    );
  $editFields['extra']['check_daylight_savings'] = array (
    '#type' => 'checkbox',
    '#title' => t("Observe Daylight Savings"),
    '#default_value' => $currfield['extra']['check_daylight_savings'],
    '#checked_value' => 1,
    '#description' => t('Automatically adjust the time during daylight savings.'),
    '#weight' => 1,
  );
    $editFields['extra']['hourformat'] = array (
      '#type' => 'radios',
      '#title' => t("Time Format"),
      '#default_value' => isset($currfield['extra']['hourformat']) ? $currfield['extra']['hourformat'] : '12-hour',
      '#description' => t('Format the display of the time in 12 or 24 hours.'),
      '#options' => array('12-hour' => '12-hour (am/pm)', '24-hour' => '24-hour'),
      '#weight' => 2,
    );
    return $editFields;
}

/** 
 * function webform_render_time
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function _webform_render_time ($component) {
	
	// Calculate the timestamp in GMT
	$timestamp = strtotime($component['value']);
	if ($component['extra']['timezone'] == "user") {
	  // Use the users timezone
    global $user;
    $timestamp += (int)$user->timezone;
  } elseif ($component['extra']['timezone'] == "gmt") {
    // Use GMT
    $timestamp += 0;
  } else {
    // Use the Drupal site time
    $timestamp += (int)variable_get('date_default_timezone',0);
  }

  // Check for daylight savings time
  if ($component['extra']['check_daylight_savings'] && date("I")) {
    $timestamp += 3600;
  }
  
  if ($component['extra']['hourformat'] == '12-hour') {
    $first_hour = 1;
    $last_hour = 12;
    $hour_format = 'g';
  } else {
    $first_hour = 0;
    $last_hour = 23;
    $hour_format = 'G';
  }

  // Generate the choices for drop-down selects
  $hours[""] = t("hour");
  $minutes[""] = t("minute");
  for ($i = $first_hour; $i <= $last_hour; $i++) $hours[$i] = $i;
  for ($i = 0; $i <= 59; $i++) $minutes[$i < 10 ? "0$i" : $i] = $i < 10 ? "0$i" : $i;
  $am_pms = array('am' => t('am'), 'pm' => t('pm'));
  
  if (strlen($component['value']) > 0) {
    $hour = gmdate($hour_format, $timestamp);
    $minute = gmdate('i', $timestamp);
    $am_pm = gmdate('a', $timestamp);
  }

  $formItem['#type'] = 'fieldset';
  $formItem['#title'] = $component['name'];
  if ($component['mandatory']) {
    $formItem['#title'] .= '<span class="form-required" title="'.t("This field is required.").'">*</span>';
  }
  $formItem['#weight'] = $component['weight'];
  $formItem['#description'] = $component['extra']['description'];
  $formItem['#prefix'] = '<div class="container-inline">';
  $formItem['#suffix'] = '</div>';
  $formItem['hour'] = array(
    '#prefix' => '',
    '#type' => 'select',
    '#default_value' => $hour,
    '#options' => $hours,
    '#validate' => array ('webform_validate_time' => array ('hour',$component['name'],$component['mandatory']) ),
    );
  $formItem['minute'] = array(
    '#prefix' => ':',
    '#type' => 'select',
    '#default_value' => $minute,
    '#options' => $minutes,
    '#validate' => array ('webform_validate_time' => array ('minute',$component['name'],$component['mandatory']) ),
    );
  if ($component['extra']['hourformat'] == '12-hour') {
    $formItem['ampm'] = array(
      '#type' => 'radios',
      '#default_value' => $am_pm,
      '#options' => $am_pms,
      );
  }
  
	return $formItem;
}

function webform_validate_time ($field,$fieldName,$componentName,$mandatory) {
  if (!is_numeric($field['#value']) && $mandatory) {
    form_set_error ($fieldName,$componentName." ".$fieldName." ".t(" field is required"));
    return false;
  }
}

/** 
 * function _webform_submission_data_time
 * Display the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_submission_display_time ($data,$component) {
  $formItem = _webform_render_time($component);
  $formItem['minute']['#default_value']   = $data['value']['1'];
  $formItem['minute']['#attributes']      = array("disabled" => "disabled");
  $formItem['hour']['#attributes']        = array("disabled" => "disabled");
  $formItem['ampm']['#attributes']        = array("disabled" => "disabled");
  
  // Match the hourly data to the hour format
  $timestamp = strtotime($data['value']['0'].":".$data['value']['1'].$data['value']['2']);
  if ($component['extra']['hourformat'] == "24-hour") {
    $formItem['hour']['#default_value']     = date("H",$timestamp);
  } else {
    $formItem['hour']['#default_value']     = date("g",$timestamp);
    $formItem['ampm']['#default_value']     = $data['value']['2'];
  }
  return $formItem;
}

/** 
 * function _webform_help_time
 * Module specific instance of hook_help
 **/
function _webform_help_time($section) {
  switch ($section) {
    case 'admin/settings/webform#time_description':
      $output = t("Presents the user with hour and minute fields. Optional am/pm fields.");
      break;
  }
  return $output;
}

/** 
 * function _webform_analysis_view_time
 * Calculate and returns statistics about results for this component from all submission to this webform. The output of this function will be displayed under the "results" tab then "analysis"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data rows, each containing a statistic for this component's submissions.
 **/
function _webform_analysis_rows_time ($component) {  
  $query = 'SELECT no,data '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND  cid = %d '.
    ' ORDER BY sid,no ASC ';

  $result = db_query($query, $component['nid'], $component['cid']);
  // build an array of timestamps from entered values
  $timestamps = array();
  $submissions++;
  while ($row = db_fetch_array($result)) {
    if ($row['no'] == '0') {
      $submissions++;
      $hour = $row['data'];
      if ($row = db_fetch_array($result)) {
        if ($row['no'] == '1') {
          $minute = $row['data'];
          if ($row = db_fetch_array($result)) {
            if ($row['no'] == '2') {
              $ampm = $row['data'];
              // Build the full timestamp
              if (strlen($hour) > 0 && strlen($day) > 0 ) {
                $timestamps[] = strtotime($hour.":".$minute.$ampm);
              }
            } else {
              // Build military time
              $timestamps[] = strtotime($hour.":".$minute);
            }
          }
        }
      }
    }    
  }
  
  // Display stats
  // TODO: display date statistics in javascript tabs
  $nonblanks = count($timestamps);
  $rows[0] = array( t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array( t('User entered value'), $nonblanks);
  return $rows;
}

/** 
 * function _webform_table_data_time
 * Return the result of this component's submission for display in a table. The output of this function will be displayed under the "results" tab then "table"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_table_data_time ($data,$component) {
  if (strlen($data['value']['0']) > 0 && strlen($data['value']['1']) > 0) {
    $timestamp = strtotime($data['value']['0'].":".$data['value']['1'].$data['value']['2']);
    if ($component['extra']['hourformat'] == '24-hour') {
      return date("H:i",$timestamp);
    } else {
      return date("g:i a",$timestamp);
    }
  } else {
    return "";
  }
}


/** 
 * function _webform_csv_headers_time
 * Return the header information for this component to be displayed in a comma seperated value file. The output of this function will be displayed under the "results" tab then "download"
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of data to be displayed in the first three rows of a CSV file, not including either prefixed or trailing commas
 **/
function _webform_csv_headers_time ($component) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $component['name'];
  return $header;
}

/** 
 * function _webform_csv_data_time
 * Return the result of a textfield submission. The output of this function will be displayed under the "results" tab then "submissions"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for CSV, not including either prefixed or trailing commas
 **/
function _webform_csv_data_time ($data) {
  if (strlen($data['value']['0']) > 0 && strlen($data['value']['1']) > 0) {
    $timestamp = strtotime($data['value']['0'].":".$data['value']['1'].$data['value']['2']);
    if ($component['extra']['hourformat'] == '24-hour') {
      return date("H:i",$timestamp);
    } else {
      return date("g:i a",$timestamp);
    }
  } else {
    return "";
  }
}



?>
