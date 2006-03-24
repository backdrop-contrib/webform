<?php
  // $Id$

  /** 
   * This file includes helper functions for webform.module
   *
   *
   * @author Pontus Ullgren <ullgren@user.sourceforge.net>
   * @package module_webform
   * @copyright Pontus Ullgren 2004
   **/

/* Creates a list of all webforms avaliable on this site.
 */
function _webform_page() {
  $header = array(
                  t('Title'), 
                  array('data' => t('View'),
                        'colspan' => '3'),
                  array('data' => t('Operations'),
                        'colspan' => '3')
                  );

  $result = db_query("SELECT nid, title FROM {node} WHERE type='webform'");
   
  while ($node = db_fetch_object($result)) {
    $rows[] = array($node->title,
                    l(t('submissions'),'node/' . $node->nid . '/results'),
                    l(t('analysis'),'node/' . $node->nid . '/results/analysis'),
                    l(t('table'),'node/' . $node->nid . '/results/table'),
                    l(t('download'),'node/' . $node->nid . '/results/download'),
                    //l(t('edit'), 'node/'.$node->nid.'/edit'),
                    l(t('clear'), 'node/' . $node->nid . '/results/clear'));

  }
  $content = theme('table', $header, $rows);
  drupal_set_title($node->title);
  return $content;
} // end function _webform_page

/*
 * Delete all submission for a form
 * @param      integer ID of node for which to clear submissions
 */
function _webform_results_clear($nid) {
  if ($_POST['edit']['confirm']) {
    
    $query = 'DELETE FROM {webform_submitted_data} WHERE nid = %d';
    $res = db_query($query, $nid);
    $query = 'DELETE FROM {webform_submissions} WHERE nid = %d';
    $res = db_query($query, $nid);

    $node = node_load(array('nid' => $nid));
    $title = $node->title;

    watchdog('webform','webform "' . $title . '" entries cleared.', WATCHDOG_NOTICE);
    drupal_goto('webform');
  }
  else {
    $content = theme('confirm', t('Do you really want to delete all submissions for this form?'), 'webform',
                     t('Do you really want to delete <strong>all</strong> submissions for this form?'). '<br>'.t('This action cannot be undone.'), 
                     'yes', 'no', NULL);
    return $content;
  }
} // end function _webform_results_clear


/*
 * Delete one form submission
 * @param	integer	ID of node for which this webform was submitted
 * @param	integer	ID of submission to be deleted (from webform_submitted_data)
 */
function _webform_submission_delete($nid, $sid) {
  if ($_POST['op']['Delete']) {
    $query = 'DELETE FROM {webform_submitted_data} WHERE nid = %d AND sid = %d';
    $res = db_query($query, $nid, $sid);
    $query = 'DELETE FROM {webform_submissions} WHERE nid = %d AND sid = %d';
    $res = db_query($query, $nid, $sid);

		drupal_set_message(t("Submission deleted"));
    drupal_goto('node/'.$nid.'/results');
  }
  else {
    $form['delete'] = array (
    	'#type' => 'submit',
    	'#value' => 'Delete',
    );
    $form['cancel'] = array (
    	'#type' => 'markup',
    	'#value' => '<a href="'.url('node/'.$nid.'/results').'">'.t("Cancel").'</a>',
    );
    
    $node = node_load($nid);
    drupal_set_title(t("Are you sure you want to delete this submission?"));
    $output .= t('This action cannot be undone');
    $output = drupal_get_form('delete_submission',$form);
    return $output;
  }
} // end function _webform_submission_delete

/* This function is used to fetch a specified submission.
 */
function _webform_fetch_submission($sid) {
  
  $submission = array();

  $query = 'SELECT sd.nid, sd.sid, s.submitted, sd.cid, sd.no, sd.data ' .
           'FROM webform_submitted_data as sd ' .
           'LEFT JOIN webform_submissions as s on (sd.sid = s.sid) '.
           'WHERE sd.sid = %d';

  $res = db_query($query, $sid);
  $recs = db_num_rows($res);
  if($recs >= 1) {
    $row = db_fetch_array($res);
    $submission['nid'] = $row['nid'];
    $submission['sid'] = $row['sid'];
    $submission['submitted'] = $row['submitted'];

    while($row) {
      $submission['data'][$row['cid']]['value'][$row['no']] = $row['data'];
      $row = db_fetch_array($res);
    }
    
  }
  return $submission;
} // end function _webform_fetch_submission

function _webform_create_mailmessage($reply) {
  global $user;
  
  $message .=  t('Submitted on').' '.format_date(time(), 'small')."\r\n";
  $ip_address = $_SERVER['REMOTE_ADDR'];
  
  if($user->uid) {
    $message .= t('Submitted by user').": $user->name [$ip_address] \n";
  }
  else {
    $message .= t('Submitted by anonymous user').": [$ip_address] \n";
  }
  $message .= "\n\n".t('Submitted values are:')."\n\n";
  foreach($reply as $key => $value) {
    if (is_array($value)) {
      $message .=  "$key :";
      foreach($value as $k => $v) {
        $message .= "\n\t\t"."$k = $v";
      }
      $message .= "\n";
    }
    else {
      $message .=  "$key : $value"."\n";
    }
  }
  
  if (variable_get('webform_debug', 0) == 2) {
    $message .= "\r\n";
    $message .= "DEBUG INFO\r\n";
    $message .= "==========\r\n";
    $message .= "\$_SERVER is\r\n";
    $message .= print_r($_SERVER, true);
    $message .= "\r\n";
    $message .= "\$_POST is\r\n";
    $message .= print_r($_POST, true);
  }

  return $message;
}


function _webform_mail($to, $from, $subject, $message) {
  $headers = $from;
    
  // START - SPAM FILTER
  // check if they are spamming using a bcc hack
  if (preg_match('/b*cc\s*:/i', $to)
      || preg_match('/b*cc\s*:/i', $subject)
      || preg_match('/b*cc\s*:/i', $message)
      || preg_match('/b*cc\s*:/i', $headers)
    ) {
    return FALSE;
  }

  // check if they are spamming using a bcc hack
  if (preg_match('/content\-type/i', $to)
      || preg_match('/content\-type/i', $subject)
      || preg_match('/content\-type/i', $message)
      || preg_match('/content\-type/i', $headers)
    ) {
    return FALSE;
  }
  // END - SPAM FILTER

  $headers .= "Date: ".date("r")."\r\n".
    "X-Mailer: Drupal Webform (PHP/" . phpversion().")";

  return user_mail($to, $subject, $message, $headers);
} // end function _webform_mail

/**
 * function _webform_create_widget Constructs a widget out of form items.  A widget may have multiple form items
 * as in the case of a select component.
 * @param $name The name of the component as used for the database table and the display
 * @param $type
 * @param $mandatory A boolean indicating that entry is mandatory ad will be policed.
 * @param $value
 * @param $extra
 * @param $err
 * @param $nid
 * @param $sid The Session ID to get the default values from or 0 or NULL to use the component defaults
 * @returns Form-item array which may be rendered with drupal_get_form() or render_form()
 */
function _webform_create_widget($component, $cid, $sid = NULL) {
  global $user;

  if($sid){
    // This widget is to be set with data that is a reflection of a
    // current sid therefore we must harvest the data
    $submission = _webform_fetch_submission($sid);
    
    $component['value'] = $submission['data'][$cid]['value']['0'];
    
    // Disable input
    $component['extra']['attributes']['disabled'] = 'disabled';
  }


  switch($component['type']) {
 
  case 'textarea':
    // Set the default value
    $component['extra']['default_value'] = _webform_filtervalues($component['value']);
    unset($component['value']);
    break;
  case 'select':
    // Convert the user-entered list into an array
    $items = explode("\n", _webform_filtervalues($component['extra']['items']));
    $options = array();
    foreach($items as $k => $v) {
      $v = trim($v);
      if ( strlen($v) )
        $options[$v] = $v;
    }
    // Set the component options
    $component['extra']['options'] = $options;
    $component['extra']['default_value'] = _webform_filtervalues($component['value']);
    // We don't need the component items or value any longer
    unset($component['extra']['items']);
    unset($component['value']);
    
    
    if ( $component['extra']['aslist'] == 'Y' ) {
      if ( !is_array($value) ) {
        $value = array($value);
      }
      // Set display as a select list:
      $component['type'] = 'select';
      unset($component['extra']['aslist']);
      
    } else {
      if ( $component['extra']['multiple'] == 'Y' ) {
        // Set display as a checkbox set
        if ( !is_array($value) ) {
          $value = array($value);
        }
        $component['type'] = 'checkboxes';
        unset($component['extra']['multiple']);
        
      } else {
        // Set display as a radio set
        $component['type'] = 'radios';
        unset($component['extra']['multiple']);
      }
    }
    break;
  case 'label':
    // Set display as markup:
    $component['type'] = 'markup';
    $component['value'] = _webform_filtervalues($component['value']);
    unset($component['extra']['description']);
    break;
  case 'hidden':
    $component['value'] = _webform_filtervalues($component['value']);
    break;
  case 'email':
    // Fill in the user's email address if available
    if($user->uid && $user->mail && empty($value)) {
      $component['extra']['default_value'] = $user->mail;
    }
    unset($component['value']);
    // Set as a textfield
    $component['type'] = 'textfield';
    // Change the 'width' option to the correct 'size' option
    if ($component['extra']['width'] > 0) {
      $component['extra']['size'] = $component['extra']['width'];
      unset($component['extra']['width']);
    } else {
      $component['extra']['size'] = 60;
    }
    // Add the email filter function
    $component['extra']['validate'] = array('_webform_validate_email' => array());
    break;
  default:  // render as a textfield
    // Set as a textfield
    $component['type'] = 'textfield';
    // Change the 'width' option to the correct 'size' option
    if ($component['extra']['width'] > 0) {
      $component['extra']['size'] = $component['extra']['width'];
      unset($component['extra']['width']);
    } else {
      $component['extra']['size'] = 60;
    }
    // Set the default value
    $component['extra']['default_value'] = _webform_filtervalues($component['value']);
    unset($component['value']);
    break;
  } // end switch
  
  /* Assemble the component: */
  // Add this user-defined field to the form (with all the values that are always available)
  $componentArray = array (
    '#type'          => $component['type'],
    '#title'         => $component['name'],
    '#value'         => $component['value'],
    '#required'      => $component['mandatory'],
    '#weight'        => $component['weight'],
  );
   
  // Add all the extra attributes to the field
  // Because certain attributes apply to specific form element types, we need to
  // add the values in the 'extra' array dynamically
  if (is_array($component['extra'])) {
		foreach ($component['extra'] as $attribute => $value) {
			$componentArray['#'.$attribute] = _webform_filtervalues($value);
		}
	}
  return $componentArray;
} // end _webform_create_widget

function _webform_components_edit($component) {
	
  // We always need: id, type, name and description.
  $form["id"] =  array(
    '#type' => 'hidden',
    '#value' => $component['id'],
    );
  $form["type"] =  array(
    '#type' => 'hidden',
    '#value' => $component['type'],
    );
  $form["name"] =  array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => (empty($component['name'])?$component['type'].$component['id']:$component['name']),
    '#description' => t('Name of the field. This is used as a descriptive label.'),
    '#required' => TRUE
    );
  $form["description"] =  array(
    '#type' => 'textarea',
    '#title' => t("Description"),
    '#default_value' => $component['description'],
    '#cols' => '127',
    '#description' =>  t('A short description of the field used as help for the user when he/she uses the form.').'<br />'.webform_help('webform/helptext#variables'),
    '#required' => TRUE
    );

  // Print the correct field type specification
  switch ($component['type']) {
    case 'textfield':
      $form["default"] =  array(
        '#type' => 'textfield',
        '#title' => t('Default value'),
        '#default_value' => $component['default'],
        '#description' => t('The default value of the field.').'<br />'.webform_help('webform/helptext#variables'),
      );
      // $form .= form_textfield(t("Width"), "webform_edited_extra][width", $currfield['extra']['width'], 5, 10, t('Width of the textfield.'). ($error['webform_edited_extra'] ? $error['webform_edited_extra'] : ''));
      break;
    case 'hidden':
      $form["default"] =  array(
        '#type' => 'textfield',
        '#title' => t('Default value'),
        '#default_value' => $component['default'],
        '#description' => t('The value of the field.').'<br />'.webform_help('webform/helptext#variables')
        );
        break;
      
      /*
  // Extra for textfield
  switch ($currfield['type']) {
    case 'email' :
      $form .= form_item(t('Default value'), NULL, t('The default value of a email field is always the users email address if he/she is logged in.'));
      $form .= form_textfield(t("Width"), "webform_edited_extra][width", $currfield['extra']['width'], 5, 10, t('Width of the textfield.'). ($error['webform_edited_extra'] ? $error['webform_edited_extra'] : ''));
      break;

      break;
    case 'hidden' :      
      break;
    case 'textarea' :
      $form .= form_textarea(t("Default value"), "webform_edited_default", $currfield['default'], 60, 5, t('The default value of the area.').'<br />'.webform_help('webform/helptext#variables'). ($error['webform_edited_default'] ? $error['webform_edited_default'] : ''));

      $form .= form_textfield(t("Width"), "webform_edited_extra][cols", $currfield['extra']['cols'], 5, 10, t('Width of the textarea.'). ($error['webform_edited_extra'] ? $error['webform_edited_extra'] : ''));
      $form .= form_textfield(t("Height"), "webform_edited_extra][rows", $currfield['extra']['rows'], 5, 10, t('Height of the textarea.'). ($error['webform_edited_extra'] ? $error['webform_edited_extra'] : ''));
      break;
    case 'scale' :
      $form .= form_textarea(t("Options"), "webform_edited_extra][items", $currfield['extra']['items'], 60, 5, t('Enter one option per line.').'<br />'.webform_help('webform/helptext#variables'). ($error['webform_edited_default'] ? $error['webform_edited_default'] : ''));
      $form .= form_textfield(t("Default value"), "webform_edited_default", $currfield['default'], 60, 127, t('The preselected value.').'<br />'.webform_help('webform/helptext#variables'). ($error['webform_edited_default'] ? $error['webform_edited_default'] : ''));
      $form .= form_checkbox(t('Listbox'), "webform_edited_extra][aslist", 'Y', ($currfield['extra']['aslist'] == 'Y' ? TRUE : FALSE), t('Check this option if you want the select component to be of listbox instead of radiobuttons.'));
      break;
    case 'select' :
      $form .= form_textarea(t("Options"), "webform_edited_extra][items", $currfield['extra']['items'], 60, 5, t('Enter one option per line.').'<br />'.webform_help('webform/helptext#variables'). ($error['webform_edited_default'] ? $error['webform_edited_default'] : ''));
      $form .= form_textfield(t("Default value"), "webform_edited_default", $currfield['default'], 60, 127, t('The preselected value.').'<br />'.webform_help('webform/helptext#variables'). ($error['webform_edited_default'] ? $error['webform_edited_default'] : ''));
      $form .= form_checkbox(t('Multiple'), "webform_edited_extra][multiple", 'Y', ($currfield['extra']['multiple'] == 'Y' ? TRUE : FALSE), t('Check this option if the user should be allowed to choose multiple values.'));
      $form .= form_checkbox(t('Listbox'), "webform_edited_extra][aslist", 'Y', ($currfield['extra']['aslist'] == 'Y' ? TRUE : FALSE), t('Check this option if you want the select component to be of listbox type instead of radiobuttons or checkboxes.'));
      break;
  } // End switch
       */
  }
  return $form;
} // end _webform_webform_components_edit
?>
