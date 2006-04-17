<?php
/** 
 * function webform_edit_time
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_time ($currfield) {
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
    $editFields['extra']['hourformat'] = array (
      '#type' => 'radios',
      '#title' => t("Time Format"),
      '#default_value' => $currfield['extra']['hourformat'],
      '#description' => t('Format the display of the time in 12 or 24 hours.'),
      '#options' => array('12-hour' => '12-hour (am/pm)', '24-hour' => '24-hour'),
      '#weight' => 0,
    );
    return $editFields;
}

/** 
 * function webform_render_time
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_time ($component) {
	
	// Calculate the timestamp in GMT
	$timestamp = strtotime($component['value']." GMT");
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

  if ($component['extra']['hourformat'] == '12-hour') {
    $first_hour = 1;
    $last_hour = 12;
    $hour_format = 'g';
  } else {
    $first_hour = 0;
    $last_hour = 23;
    $hour_format = 'G';
  }

  $hours[""] = t("hour");
  $minutes[""] = t("minute");
  for ($i = $first_hour; $i <= $last_hour; $i++) $hours[$i] = $i;
  for ($i = 0; $i <= 59; $i++) $minutes[$i < 10 ? "0$i" : $i] = $i < 10 ? "0$i" : $i;
  $am_pms = array('am' => t('am'), 'pm' => t('pm'));
  
  // Use format_date(), it handles user timezone and locale.
  if ($component['value'] > 0) {
    $hour = format_date($timestamp, 'custom', $hour_format);
    $minute = format_date($timestamp, 'custom', 'i');
    $am_pm = format_date($timestamp, 'custom', 'a');;
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


?>
