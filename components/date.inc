<?php
/** 
 * function webform_edit_date
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @returns An array of form items to be displayed on the edit component page
 **/
function webform_edit_date ($currfield) {
  $editFields = array();
  $editFields['value'] = array (
    '#type' => 'textfield',
    '#title' => t("Default value"),
    '#default_value' => $currfield['default'],
    '#description' => t('The default value of the field.').'<br />'.t('Accepts any date in any <a href="http://www.gnu.org/software/tar/manual/html_node/tar_109.html">GNU Date Input Format</a>. Strings such as today, +2 months, and Dec 9 2004 are all valid.'),
    '#size' => 60,
    '#maxlength' => 127,
    '#weight' => 0,
  );
  $editFields['extra']['timezone'] = array (
    '#type' => 'radios',
    '#title' => t("Timezone"),
    '#default_value' => empty($currfield['extra']['timezone']) ? "site" : $currfield['extra']['timezone'],
    '#description' => t('Adjust the date according to a specific timezone. Website timezone is defined in the <a href="%settings">Site Settings</a> and is the default.',array('%settings' => url('admin/settings'))),
    '#options' => array('site' => 'Website Timezone', 'user' => 'User Timezone', 'gmt' => 'GMT'),
    '#weight' => 0,
  );

  return $editFields;
}

/** 
 * function webform_render_date
 * Build a form item array containing all the properties of this component
 * @param $component An array of information describing the component, directly correlating to the webform_component database schema
 * @returns An array of a form item to be displayed on the client-side webform
 **/
function webform_render_date ($component) {
	
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

  $months = array("" => t("month"), 1 => t('January'), t('February'), t('March'), t('April'), t('May'), t('June'), t('July'), t('August'), t('September'), t('October'), t('November'), t('December'));
  $days = array("" => t("day"));
  for ($i = 1; $i <= 31; $i++) $days[$i] = $i;

  // Use format_date(), it handles user timezone and locale.
  if ($component['value'] > 0) {
    $year = format_date($timestamp, 'custom', 'Y');
    $month = format_date($timestamp, 'custom', 'm');
    $day = format_date($timestamp, 'custom', 'd');
  }
  
  $formItem['#type'] = 'fieldset';
  $formItem['#title'] = $component['name'];
  $formItem['#weight'] = $component['weight'];
  $formItem['#description'] = $component['extra']['description'];
  $formItem['#prefix'] = '<div class="container-inline">';
  $formItem['#suffix'] = '</div>';
  if ($component['mandatory']) {
    $formItem['#title'] .= '<span class="form-required" title="'.t("This field is required.").'">*</span>';
  }
  
  $formItem['month'] = array(
    '#type' => 'select',
    '#default_value' => $month,
    '#options' => $months,
    '#validate' => array ('webform_validate_date' => array ('month',$component['name'],$component['mandatory']) ),
    );
  $formItem['day'] = array(
    '#type' => 'select',
    '#default_value' => $day,
    '#options' => $days,
    '#validate' => array ('webform_validate_date' => array ('day',$component['name'],$component['mandatory']) ),
    );
  $formItem['year'] = array(
    '#type' => 'textfield',
    '#default_value' => $year,
    '#maxlength' => 4,
    '#size' => 4,
    '#validate' => array ('webform_validate_date' => array ('year',$component['name'],$component['mandatory']) ),
    );
    
	return $formItem;
}

function webform_validate_date ($field,$fieldName,$componentName,$mandatory) {
  if (!is_numeric($field['#value']) && $mandatory) {
    form_set_error ($fieldName,$componentName." ".$fieldName." ".t(" field required"));
    return false;
  }
}

?>
