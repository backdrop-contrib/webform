<?php
// $Id$

/**
 * @file
 * This file is loaded when handling submissions, either submitting new,
 * editing, or viewing. It also contains all CRUD functions for submissions.
 *
 * @author Nathan Haug <nate@lullabot.com>
 */

function webform_submission_update($node, $sid, $submitted) {
  global $user;
  // Update submission by first deleting and then inserting it to the database.
  db_query("DELETE FROM {webform_submissions} WHERE sid = %d", $sid);
  db_query("INSERT INTO {webform_submissions} (nid, sid, uid, submitted, remote_addr) "." VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $user->uid, time(), ip_address());
  // Update the submission data by first removing all this submissions data.
  db_query("DELETE FROM {webform_submitted_data} WHERE sid = %d", $sid);
  // Then re-ad it to the database.
  foreach ($submitted as $cid => $value) {
    if (is_array($value)) {
      $delta = 0;
      foreach ($value as $k => $v) {
        db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) "."VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, $delta, $v);
        $delta++;
      }
    }
    else {
      db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) "."VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, 0, $value);
    }
  }

  return $sid;
}

function webform_submission_insert($node, $submitted) {
  global $user;

  $result = db_query("INSERT INTO {webform_submissions} (nid, uid, submitted, remote_addr) "." VALUES (%d, %d, %d, '%s')", $node->nid, $user->uid, time(), ip_address());

  $sid = db_last_insert_id('webform_submissions', 'sid');

  foreach ($submitted as $cid => $value) {
    if (is_array($value)) {
      $delta = 0;
      foreach ($value as $k => $v) {
        db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) "."VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, $delta, $v);
        $delta++;
      }
    }
    else {
      db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) "."VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, 0, $value);
    }
  }

  return $sid;
}

/**
 * Delete a single submission.
 * 
 * @param $nid
 *   ID of node for which this webform was submitted.
 * @param $sid
 *   ID of submission to be deleted (from webform_submitted_data).
 */
function webform_submission_delete($nid, $sid) {
  db_query('DELETE FROM {webform_submitted_data} WHERE nid = %d AND sid = %d', $nid, $sid);
  db_query('DELETE FROM {webform_submissions} WHERE nid = %d AND sid = %d', $nid, $sid);
}

/**
 * Delete all submissions for a node.
 * 
 * @param $nid
 *   The node id whose submissions will be deleted.
 */
function webform_results_clear($nid) {
  db_query('DELETE FROM {webform_submitted_data} WHERE nid = %d', $nid);
  db_query('DELETE FROM {webform_submissions} WHERE nid = %d', $nid);
}

/**
 * Confirmation form to delete all submissions for a node.
 * 
 * @param $nid
 *   ID of node for which to clear submissions.
 */
function webform_results_clear_form($node) {
  drupal_set_title(t("Clear Form Submissions"));

  $form = array();
  $form['nid'] = array('#type' => 'value', '#value' => $node->nid);
  $question = t("Are you sure you want to delete all submissions for this form?");

  return confirm_form($form, $question, 'node/'. $node->nid .'/webform-results', NULL, t('Clear'), t('Cancel'));
}

function webform_results_clear_form_submit($form, $form_state) {
  webform_results_clear($form_state['values']['nid']);
  $node = node_load(array('nid' => $form_state['values']['nid']));
  $title = $node->title;

  $message = t('Webform %title entries cleared.', array('%title' => $title));
  drupal_set_message($message);
  watchdog('webform', $message, WATCHDOG_NOTICE);
  drupal_goto('admin/content/webform');
}

/**
 * Confirm form to delete a single form submission.
 * 
 * @param $node
 *   The node for which this webform was submitted.
 * @param $submission
 *   The submission to be deleted (from webform_submitted_data).
 */
function webform_submission_delete_form($node, $submission) {
  $nid = $node->nid;
  $sid = $submission['sid'];

  drupal_set_title(t("Delete Form Submission"));

  $form = array();
  $form['nid'] = array('#type' => 'value', '#value' => $nid);
  $form['sid'] = array('#type' => 'value', '#value' => $sid);
  $question = t("Are you sure you want to delete this submission?");

  return confirm_form($form, $question, 'node/'. $nid .'/webform-results', NULL, t('Delete'), t('Cancel'));
}

function webform_submission_delete_form_submit($form, $form_state) {
  webform_submission_delete($form_state['values']['nid'], $form_state['values']['sid']);
  drupal_set_message(t("Submission deleted"));
  drupal_goto('node/'. $form_state['values']['nid'] .'/webform-results');
}

/**
 * Return all the submissions for a particular node.
 * 
 * @param $nid
 *   The node ID for which submissions are being fetched.
 * @param $header
 *   If the results of this fetch will be used in a sortable table, pass the
 *   array header of the table.
 */
function webform_get_submissions($nid, $header = NULL) {
  $query = 'SELECT s.*, sd.cid, sd.no, sd.data, u.name, u.mail, u.status '.
           'FROM {webform_submissions} s '.
           'LEFT JOIN {webform_submitted_data} sd ON sd.sid = s.sid '.
           'LEFT JOIN {users} u ON u.uid = s.uid '.
           'WHERE sd.nid = %d';

  if (is_array($header)) {
    $query .= tablesort_sql($header);
  }

  $res = db_query($query, $nid);
  $submissions = array();
  $previous = array();

  // Outer loop: iterate for each submission.
  while ($row = db_fetch_object($res)) {
    if ($row->sid != $previous) {
      $submissions[$row->sid]->sid = $row->sid;
      $submissions[$row->sid]->submitted = $row->submitted;
      $submissions[$row->sid]->remote_addr = $row->remote_addr;
      $submissions[$row->sid]->uid = $row->uid;
      $submissions[$row->sid]->name = $row->name;
      $submissions[$row->sid]->status = $row->status;
    }
    $submissions[$row->sid]->data[$row->cid]['value'][$row->no] = $row->data;
    $previous = $row->sid;
  }

  return $submissions;
}

/**
 * Fetch a specified submission for a webform node.
 */
function webform_get_submission($nid, $sid, $reset = FALSE) {
  static $submissions = array();

  if (!isset($submissions[$nid][$sid]) || $reset) {
    $submission = array();
    $query = 'SELECT s.*, sd.cid, sd.no, sd.data '.
             'FROM {webform_submitted_data} as sd '.
             'LEFT JOIN {webform_submissions} as s on (sd.sid = s.sid) '.
             'WHERE sd.sid = %d AND s.nid = %d';
  
    $result = db_query($query, $sid, $nid);
    if ($row = db_fetch_array($result)) {
      $submission['nid'] = $row['nid'];
      $submission['sid'] = $row['sid'];
      $submission['uid'] = $row['uid'];
      $submission['remote_addr'] = $row['remote_addr'];
      $submission['submitted'] = $row['submitted'];
  
      while ($row) {
        $submission['data'][$row['cid']]['value'][$row['no']] = $row['data'];
        $row = db_fetch_array($result);
      }
    }
    $submissions[$nid][$sid] = $submission;
  }

  return $submissions[$nid][$sid];
}


function _webform_submission_spam_check($to, $subject, $from, $headers = array()) {
  $headers = implode('\n', (array)$headers);
  // Check if they are attempting to spam using a bcc or content type hack.
  if (preg_match('/(b?cc\s?:)|(content\-type:)/i', $to ."\n". $subject ."\n". $from ."\n". $headers)) {
    return true; // Possible spam attempt.
  }
  return false; // Not spam.
}

function _webform_submission_limit_check($node, $form_values) {
  global $user, $db_type;

  // Check if submission limiting is enabled.
  if ($node->webform['submit_limit'] == '-1') {
    return false; // No check enabled.
  }

  // Retrieve submission data for this IP address or username from the database.
  $query = "SELECT count(*) ".
           "FROM {webform_submissions} ".
           "WHERE (( 0 = %d AND remote_addr = '%s') OR uid = %d )".
           "AND submitted > %d AND nid = %d";

  // Fetch all the entries from the database within the submit interval with this username and IP.
  $num_submissions_database = db_result(db_query($query, $user->uid, ip_address(), $user->uid, ($node->webform['submit_interval'] != -1) ? (time() - $node->webform['submit_interval']) : $node->webform['submit_interval'], $node->nid));

  // Double check the submission history from the users machine using cookies.
  if (variable_get("webform_use_cookies", 0)) {
    $cookie_name = 'webform-'. $node->nid;

    if (isset($_COOKIE[$cookie_name]) && is_array($_COOKIE[$cookie_name])) {
      foreach ($_COOKIE[$cookie_name] as $key => $timestamp) {
        if ($timestamp <= time() - $node->webform['submit_interval']) {
          // Remove the cookie if past the required time interval.
          setcookie($cookie_name ."[". $key ."]", "", 0);
        }
      }
      // Count the number of submissions recorded in cookies.
      $num_submissions_cookie = count($_COOKIE[$cookie_name]);
    }
    else {
      $num_submissions_cookie = 0;
    }
  }

  if ($num_submissions_database >= $node->webform['submit_limit'] || $num_submissions_cookie >= $node->webform['submit_limit']) {
    // Limit exceeded.
    return $num_submissions_database;
  }
  else {
    // Increment a cookie for triple recording of the submission.
    if (variable_get("webform_use_cookies", 0)) {
      $attempted_key = 0;
      if ($num_submissions_cookie > 0) {
        while (array_key_exists($attempted_key, $_COOKIE[$cookie_name])) {
          $attempted_key++;
        }
      }
      // Set a cookie including the server's submission time.
      // The cookie expires in the length of the interval plus a day to compensate for different timezones.
      setcookie($cookie_name ."[". $attempted_key ."]", time(), time() + $node->webform['submit_interval'] + 86400);
    }

    // Limit not exceeded.
    return false;
  }
}
