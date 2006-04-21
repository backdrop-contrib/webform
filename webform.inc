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

    drupal_set_title(t("Are you sure you want to delete all submissions for this form?"));
    $output = t('This action cannot be undone.');
    $output .= drupal_get_form('clear_results',$form);
    return $output;
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
    $output .= drupal_get_form('delete_submission',$form);
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

function theme_webform_create_mailmessage($reply) {
  global $user;
  
  $message .=  t('Submitted on').' '.format_date(time(), 'small')."\n";
  $ip_address = $_SERVER['REMOTE_ADDR'];
  
  if($user->uid) {
    $message .= t('Submitted by user').": $user->name [$ip_address]\n";
  }
  else {
    $message .= t('Submitted by anonymous user').": [$ip_address]\n";
  }
  $message .= "\n\n".t('Submitted values are:')."\n\n";
  foreach($reply as $key => $value) {
    if (is_array($value)) {
      $message .=  "$key :";
      foreach($value as $k => $v) {
        if (!empty($v)) {
          $message .= "\n\t\t$k : $v";
        }
      }
      $message .= "\n";
    }
    else {
      $message .=  "$key : $value\n";
    }
  }
  
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

  $headers .= "Date: ".date("r")."\n".
    "X-Mailer: Drupal Webform (PHP/" . phpversion().")";

  return user_mail($to, $subject, $message, $headers);
} // end function _webform_mail

?>
