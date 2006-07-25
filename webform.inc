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
  if ($_POST['op'] == t('Clear')) {
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
    $form['delete'] = array (
      '#type' => 'submit',
      '#value' => t('Clear'),
    );
    $form['cancel'] = array (
      '#type' => 'markup',
      '#value' => '<a href="'.url('node/'.$nid.'/results').'">'.t("Cancel").'</a>',
    );

    drupal_set_title(t("Clear Form Submissions"));
    $output = t("Are you sure you want to delete all submissions for this form?")." ".t('This action cannot be undone.');
    $output .= drupal_get_form('clear_results',$form);
    return $output;
  }
} // end function _webform_results_clear


/*
 * Delete one form submission
 * @param  integer  ID of node for which this webform was submitted
 * @param  integer  ID of submission to be deleted (from webform_submitted_data)
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
    $output .= drupal_get_form('delete_submission',$form);
    return $output;
  }
} // end function _webform_submission_delete

/* This function is used to fetch a specified submission.
 */
function _webform_fetch_submission($sid) {
  
  $submission = array();

  $query = 'SELECT sd.nid, sd.sid, s.submitted, sd.cid, sd.no, sd.data ' .
           'FROM {webform_submitted_data} as sd ' .
           'LEFT JOIN {webform_submissions} as s on (sd.sid = s.sid) '.
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

function theme_webform_create_mailmessage($form_values, $node) {
  global $user;
  $message .=  t('Submitted on').' '.format_date(time(), 'small')."\n";
  $ip_address = $_SERVER['REMOTE_ADDR'];
  
  if($user->uid) {
    $message .= t('Submitted by user').": $user->name [$ip_address]\n";
  }
  else {
    $message .= t('Submitted by anonymous user').": [$ip_address]\n";
  }

  $message .= "\n";
  function _webform_submission_format ($key, $value, $indent = "   ") {
    if (is_array($value)) {
      $message .=  $key.":\n";
      foreach ($value as $k => $v) {
        $message .= $indent;
        $message .= _webform_submission_format($k,$v, $indent."   ");
      }
    } else {
      // Note that newlines cannot be preceeded by spaces to display properly in some clients
      $message .=  "$key:".(empty($value) ? "\n" : " $value\n");
    }
    return ($message);
  }
  $message .= _webform_submission_format(t('Submitted values are'),$form_values['submitted_tree']);
  
  if (variable_get('webform_debug', 0) == 2) {
    $message .= "\n";
    $message .= "DEBUG INFO\n";
    $message .= "==========\n";
    $message .= "\$_SERVER is\n";
    $message .= print_r($_SERVER, true);
    $message .= "\n";
    $message .= "\$_POST is\n";
    $message .= print_r($_POST, true);
  }

  return $message;
}


function _webform_submission_spam_check($to, $headers, $subject, $message) {
  // check if they are spamming using a bcc hack
  if (preg_match('/b*cc\s*:/i', $to)
      || preg_match('/b*cc\s*:/i', $subject)
      || preg_match('/b*cc\s*:/i', $message)
      || preg_match('/b*cc\s*:/i', $headers)
    ) {
    return true; // Possible spam attempt
  }

  // check if they are spamming using a bcc hack
  if (preg_match('/content\-type/i', $to)
      || preg_match('/content\-type/i', $subject)
      || preg_match('/content\-type/i', $message)
      || preg_match('/content\-type/i', $headers)
    ) {
    return true; // Possible spam attempt
  }
  
  return false; // Not spam
}


function _webform_submission_limit_check($node, $form_values) {
  global $user;
  
  // check if submission limiting is enabled
  if ($node->submit_limit == '-1') {
    return false; // No check enabled
  }
  
  // Retrieve submission data for this IP address or username from the database
  $query = "SELECT submitted, user, remote_addr " .
           "FROM {webform_submissions} " .
           "WHERE (user = '%s' OR remote_addr = '%s') AND submitted > %d AND nid = %d";
  
  // Fetch all the entries from the database within the submit interval with this username and IP
  $result = db_query($query, $user->name, $_SERVER['REMOTE_ADDR'],time() - $node->submit_interval, $node->nid);
  $numSubmissionsDatabase = db_num_rows($result);
  
  // Double check the submission history from the users machine using cookies
  if (variable_get("webform_use_cookies",0)) {
    $cookieName = 'webform-'.$node->nid;
    
    if (isset($_COOKIE[$cookieName]) && is_array($_COOKIE[$cookieName])) {
      foreach ($_COOKIE[$cookieName] as $key => $timestamp) {
        if ($timestamp <= time() - $node->submit_interval) {
          // Remove the cookie if past the required time interval
          setcookie($cookieName."[".$key."]", "", 0);
        }
      }
      // Count the number of submissions recorded in cookies
      $numSubmissionsCookie = count($_COOKIE[$cookieName]);
    }
    else {
      $numSubmissionsCookie = 0;
    }
  }
  
  if ($numSubmissionsDatabase >= $node->submit_limit || $numSubmissionsCookie >= $node->submit_limit) {
    // Limit exceeded
    return $numSubmissionsDatabase; 
  }
  else {
    // Increment a cookie for triple recording of the submission
    if (variable_get("webform_use_cookies",0)) {
      $attemptedKey = 0;
      if ($numSubmissionsCookie > 0) {
        while (array_key_exists($attemptedKey,$_COOKIE[$cookieName])) {
          $attemptedKey++;
        }
      }
      // Set a cookie including the server's submission time
      // The cookie expires in the length of the interval plus a day to compensate for different timezones
      setcookie($cookieName."[".$attemptedKey."]", time(), time()+$node->submit_interval+86400);
    }
    
    // Limit not exceeded
    return false;
  }
}
