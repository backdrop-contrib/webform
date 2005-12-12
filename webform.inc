<?php
  // $Id$

  /** 
   * This file includes general helper functions for webform.module
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
  print theme('page', $content);
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
  if ($_POST['edit']['confirm']) {
    $query = 'DELETE FROM {webform_submitted_data} WHERE nid = %d AND sid = %d';
    $res = db_query($query, $nid, $sid);
    $query = 'DELETE FROM {webform_submissions} WHERE nid = %d AND sid = %d';
    $res = db_query($query, $nid, $sid);

    drupal_goto('node/'.$nid.'/results');
  }
  else {
    $content = theme('confirm', t('Do you really want to delete this form submission?'), 'node/'.$nid.'/results', 
                     t('Do you really want to delete this form submission?').'<br>'.t('This action cannot be undone.'), 
                     'yes', 'no', NULL);
    return $content;
  }
} // end function _webform_submission_delete

/* This function is used to fetch a specified submission.
 */
function _webform_fetch_submission($sid) {
  $submission = array();

  $query = 'SELECT sd.nid, sd.sid, s.submitted, c.cid, c.name, sd.no, sd.data, c.extra, c.type '.
    ' FROM {webform_submitted_data} sd, {webform_component} c, {webform_submissions} s '.
    ' WHERE sd.sid = s.sid AND sd.cid = c.cid AND sd.sid = %d '.
    ' ORDER BY c.weight, c.name, c.cid ';

  $res = db_query($query, $sid);
  $recs = db_num_rows($res);
  if($recs >= 1) {
    $row = db_fetch_array($res);
    $submission['nid'] = $row['nid'];
    $submission['sid'] = $row['sid'];
    $submission['submitted'] = $row['submitted'];

    while($row) {
      $submission['data'][$row['cid']]['name'] = $row['name'];
      $submission['data'][$row['cid']]['type'] = $row['type'];
      $submission['data'][$row['cid']]['extra'] = unserialize($row['extra']);
      $submission['data'][$row['cid']]['value'][$row['no']] = $row['data'];

      $row = db_fetch_array($res);
    }
  }
  return $submission;
} // end function _webform_fetch_submission

function _webform_mail($to, $from, $subject, $message) {
  $headers = $from;
  $headers .= "Date: ".date("r")."\r\n".
    "X-Mailer: Drupal Webform (PHP/" . phpversion().")";
    
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

  return user_mail($to, $subject, $message, $headers);
} // end function _webform_mail

?>
