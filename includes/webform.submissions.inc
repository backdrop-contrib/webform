<?php
// $Id$

/**
 * @file
 * This file is loaded when handling submissions, either submitting new,
 * editing, or viewing. It also contains all CRUD functions for submissions.
 *
 * @author Nathan Haug <nate@lullabot.com>
 */

function webform_submission_update($node, $sid, $submitted, $is_draft = 0) {
  // If is draft, only delete data for components submitted, to
  // preserve any data from form pages not visited in this submission.
  if ($is_draft) {
    $submitted_cids = array_keys($submitted);
    db_query("DELETE FROM {webform_submitted_data} WHERE sid = %d AND cid IN (" . implode(', ', $submitted_cids) . ")", $sid);
  }
  else {
    db_query("UPDATE {webform_submissions} SET is_draft = 0");
    db_query("DELETE FROM {webform_submitted_data} WHERE sid = %d", $sid);
  }

  // Then re-add it to the database.
  foreach ($submitted as $cid => $value) {
    // Don't save pagebreaks as submitted data.
    if ($node->webform['components'][$cid]['type'] == 'pagebreak') {
      continue;
    }

    if (is_array($value)) {
      $delta = 0;
      foreach ($value as $k => $v) {
        db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, $delta, $v);
        $delta++;
      }
    }
    else {
      db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, 0, $value);
    }
  }

  return $sid;
}

function webform_submission_insert($node, $submitted, $is_draft = 0) {
  global $user;

  $result = db_query("INSERT INTO {webform_submissions} (nid, uid, submitted, remote_addr, is_draft) VALUES (%d, %d, %d, '%s', %d)", $node->nid, $user->uid, time(), ip_address(), $is_draft);
  $sid = db_last_insert_id('webform_submissions', 'sid');

  foreach ($submitted as $cid => $value) {
    // Don't save pagebreaks as submitted data.
    if ($node->webform['components'][$cid]['type'] == 'pagebreak') {
      continue;
    }

    if (is_array($value)) {
      $delta = 0;
      foreach ($value as $k => $v) {
        db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, $delta, $v);
        $delta++;
      }
    }
    else {
      db_query("INSERT INTO {webform_submitted_data} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')", $node->nid, $sid, $cid, 0, $value);
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
function webform_submission_delete($node, $submission) {
  // Iterate through all components and let each do cleanup if necessary.
  foreach ($node->webform['components'] as $cid => $component) {
    if (isset($submission->data[$cid])) {
      webform_component_invoke($component['type'], 'delete', $component, $submission->data[$cid]);
    }
  }

  // Delete any anonymous session information.
  if (isset($_SESSION['webform_submission'][$submission->sid])) {
    unset($_SESSION['webform_submission'][$submission->sid]);
  }

  db_query('DELETE FROM {webform_submitted_data} WHERE nid = %d AND sid = %d', $node->nid, $submission->sid);
  db_query('DELETE FROM {webform_submissions} WHERE nid = %d AND sid = %d', $node->nid, $submission->sid);
}

/**
 * Confirm form to delete a single form submission.
 *
 * @param $form_state
 *   The current form state.
 * @param $node
 *   The node for which this webform was submitted.
 * @param $submission
 *   The submission to be deleted (from webform_submitted_data).
 */
function webform_submission_delete_form($form_state, $node, $submission) {
  webform_set_breadcrumb($node, $submission);

  $form = array();
  $form['node'] = array('#type' => 'value', '#value' => $node);
  $form['submission'] = array('#type' => 'value', '#value' => $submission);
  $question = t('Are you sure you want to delete this submission?');

  return confirm_form($form, NULL, isset($_GET['destination']) ? $_GET['destination'] : 'node/'. $node->nid .'/webform-results', $question, t('Delete'), t('Cancel'));
}

function webform_submission_delete_form_submit($form, &$form_state) {
  webform_submission_delete($form_state['values']['node'], $form_state['values']['submission']);
  drupal_set_message(t('Submission deleted.'));

  $form_state['redirect'] = 'node/'. $form_state['values']['node']->nid .'/webform-results';
}

/**
 * Menu title callback; Return the submission number as a title.
 */
function webform_submission_title($node, $submission) {
  return t('Submission #@sid', array('@sid' => $submission->sid));
}

/**
 * Menu callback; Present a Webform submission page for display or editing.
 */
function webform_submission_page($node, $submission, $format) {
  global $user;
  webform_set_breadcrumb($node, $submission);

  if ($format == 'form') {
    $output = drupal_get_form('webform_client_form_'. $node->nid, $node, $submission);
  }
  else {
    $output = webform_submission_render($node, $submission, $format);
  }

  // Determine the mode in which we're displaying this submission.
  $mode = ($format != 'form') ? 'display' : 'form';
  if (preg_match("!^q=print/!", $_SERVER['QUERY_STRING'])) {
    $mode = 'print';
  }
  if (preg_match("!^q=printpdf/!", $_SERVER['QUERY_STRING'])) {
    $mode = 'pdf';
  }

  // Add navigation for administrators.
  if (user_access('access all webform results') || (user_access('access own webform results') && $user->uid == $node->uid)) {
    $navigation = ($mode != 'pdf') ? theme('webform_submission_navigation', $node, $submission, $mode) : NULL;
    $information = theme('webform_submission_information', $node, $submission);
  }
  else {
    $navigation = NULL;
    $information = NULL;
  }

  // Disable the page cache for anonymous users viewing or editing submissions.
  if (!$user->uid) {
    webform_disable_page_cache();
  }

  return theme('webform_submission_page', $output, $navigation, $information);
}

/**
 * Print a Webform submission for display on a page or in an e-mail.
 */
function webform_submission_render($node, $submission, $format) {
  $component_tree = array();
  $renderable = array();
  $page_count = 1;

  _webform_components_tree_build($node->webform['components'], $component_tree, 0, $page_count);

  // Recursively add components to the form.
  foreach ($component_tree['children'] as $cid => $component) {
    _webform_client_form_add_component($component, NULL, $renderable, $renderable, $submission, $format);
  }

  return drupal_render($renderable);
}

/**
 * Return all the submissions for a particular node.
 *
 * @param $nid
 *   The node ID for which submissions are being fetched.
 * @param $header
 *   If the results of this fetch will be used in a sortable
 *   table, pass the array header of the table.
 * @param $uid
 *   Optional; the user ID to filter the submissions by.
 * @return $submissions
 *   An array of submissions matching your filters.
 */
function webform_get_submissions($nid, $header = NULL, $uid = NULL, $pager_count = 0) {
  $sids = NULL;
  $arguments = array($nid);
  if ($pager_count || $uid === 0) {
    $pager_query = 'SELECT * FROM {webform_submissions} WHERE nid = %d';
    if ($uid !== NULL) {
      $pager_query .= ' AND uid = %d';
      $arguments[] = $uid;
    }
    if ($uid === 0) {
      $submissions = array_keys($_SESSION['webform_submission']);
      $placeholders = count($submissions) ? array_fill(0, count($submissions), "%d") : array();
      $pager_query .= count($submissions) ? ' AND sid IN (' . implode(',', $placeholders) . ')' : ' AND sid = 0';
      $arguments = array_merge($arguments, $submissions);
    }
    $result = pager_query($pager_query, $pager_count, 0, NULL, $arguments);
    $sids = array();
    while ($row = db_fetch_object($result)) {
      $sids[] = $row->sid;
    }
    $sids = implode($sids, ',');
  }

  $query = 'SELECT s.*, sd.cid, sd.no, sd.data, u.name, u.mail, u.status '.
           'FROM {webform_submissions} s '.
           'LEFT JOIN {webform_submitted_data} sd ON sd.sid = s.sid '.
           'LEFT JOIN {users} u ON u.uid = s.uid '.
           'WHERE sd.nid = %d';

  if ($pager_count && isset($sids)) {
    $query .= empty($sids) ? ' AND s.sid = 0' : ' AND s.sid IN (%s)';
  }

  if ($uid) {
    $query .= ' AND u.uid = %d';
  }

  if (is_array($header)) {
    $query .= tablesort_sql($header);
    if (!isset($header[0]['sort'])) {
      $query .= ', sid ASC';
    }
    $query .= ', cid ASC, no ASC';
  }
  else {
    $query .= ' ORDER BY sid ASC, cid ASC, no ASC';
  }

  $res = $pager_count ? db_query($query, $nid, $sids, $uid) : db_query($query, $nid, $uid);
  $submissions = array();
  $previous = array();

  // Outer loop: iterate for each submission.
  while ($row = db_fetch_object($res)) {
    if ($row->sid != $previous) {
      $submissions[$row->sid] = new stdClass();
      $submissions[$row->sid]->sid = $row->sid;
      $submissions[$row->sid]->submitted = $row->submitted;
      $submissions[$row->sid]->remote_addr = $row->remote_addr;
      $submissions[$row->sid]->uid = $row->uid;
      $submissions[$row->sid]->name = $row->name;
      $submissions[$row->sid]->status = $row->status;
      $submissions[$row->sid]->is_draft = $row->is_draft;
    }
    $submissions[$row->sid]->data[$row->cid]['value'][$row->no] = $row->data;
    $previous = $row->sid;
  }

  return $submissions;
}

/**
 * Return a count of the total number of submissions for a node.
 *
 * @param $nid
 *   The node ID for which submissions are being fetched.
 * @param $uid
 *   Optional; the user ID to filter the submissions by.
 * @return
 *   An integer value of the number of submissions.
 */
function webform_get_submission_count($nid, $uid = NULL, $reset = FALSE) {
  static $counts;

  if (!isset($counts[$nid][$uid]) || $reset) {
    $query = 'SELECT count(*) FROM {webform_submissions} WHERE nid = %d';
    $arguments = array($nid);
    if ($uid !== NULL) {
      $query .= ' AND uid = %d';
      $arguments[] = $uid;
    }
    if ($uid === 0) {
      $submissions = $_SESSION['webform_submission'];
      $placeholders = count($submissions) ? array_fill(0, count($submissions), "%d") : array();
      $query .= count($submissions) ? ' AND sid IN (' . implode(',', $placeholders) . ')' : ' AND sid = 0';
      $arguments = array_merge($arguments, array_keys($_SESSION['webform_submission']));
    }

    $counts[$nid][$uid] = db_result(db_query($query, $arguments));
  }
  return $counts[$nid][$uid];
}

/**
 * Fetch a specified submission for a webform node.
 */
function webform_get_submission($nid, $sid, $reset = FALSE) {
  static $submissions = array();

  if (!isset($submissions[$nid][$sid]) || $reset) {
    $query = 'SELECT s.*, sd.cid, sd.no, sd.data '.
             'FROM {webform_submitted_data} as sd '.
             'LEFT JOIN {webform_submissions} as s on (sd.sid = s.sid) '.
             'WHERE sd.sid = %d AND s.nid = %d';

    $result = db_query($query, $sid, $nid);
    if ($row = db_fetch_object($result)) {
      $submission = new stdClass;
      $submission->nid = $row->nid;
      $submission->sid = $row->sid;
      $submission->uid = $row->uid;
      $submission->remote_addr = $row->remote_addr;
      $submission->submitted = $row->submitted;
      $submission->is_draft = $row->is_draft;

      while ($row) {
        $submission->data[$row->cid]['value'][$row->no] = $row->data;
        $row = db_fetch_object($result);
      }
    }
    $submissions[$nid][$sid] = isset($submission) ? $submission : NULL;
  }

  return $submissions[$nid][$sid];
}

function _webform_submission_spam_check($to, $subject, $from, $headers = array()) {
  $headers = implode('\n', (array)$headers);
  // Check if they are attempting to spam using a bcc or content type hack.
  if (preg_match('/(b?cc\s?:)|(content\-type:)/i', $to ."\n". $subject ."\n". $from ."\n". $headers)) {
    return TRUE; // Possible spam attempt.
  }
  return FALSE; // Not spam.
}

/**
 * Check if the current user has exceeded the limit on this form.
 *
 * @param $node
 *   The webform node to be checked.
 * @return
 *   Boolean TRUE if the user has exceeded their limit. FALSE otherwise.
 */
function _webform_submission_limit_check($node) {
  global $user;

  // Check if submission limiting is enabled.
  if ($node->webform['submit_limit'] == '-1') {
    return FALSE; // No check enabled.
  }

  // Retrieve submission data for this IP address or username from the database.
  $query = 'SELECT count(*) '.
           'FROM {webform_submissions} '.
           "WHERE (( 0 = %d AND remote_addr = '%s') OR (uid > 0 AND uid = %d)) ".
           'AND submitted > %d AND nid = %d AND is_draft = 0';

  // Fetch all the entries from the database within the submit interval with this username and IP.
  $num_submissions_database = db_result(db_query($query, $user->uid, ip_address(), $user->uid, ($node->webform['submit_interval'] != -1) ? (time() - $node->webform['submit_interval']) : $node->webform['submit_interval'], $node->nid));

  // Double check the submission history from the users machine using cookies.
  $num_submissions_cookie = 0;
  if ($user->uid == 0 && variable_get('webform_use_cookies', 0)) {
    $cookie_name = 'webform-'. $node->nid;

    if (isset($_COOKIE[$cookie_name]) && is_array($_COOKIE[$cookie_name])) {
      foreach ($_COOKIE[$cookie_name] as $key => $timestamp) {
        if ($timestamp <= time() - $node->webform['submit_interval']) {
          // Remove the cookie if past the required time interval.
          setcookie($cookie_name .'['. $key .']', '', 0);
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
    return TRUE;
  }

  // Limit not exceeded.
  return FALSE;
}

/**
 * Preprocess function for webform-submission-navigation.tpl.php
 */
function template_preprocess_webform_submission_navigation(&$vars) {
  $start_path = ($vars['mode'] == 'print') ? 'print/' : 'node/';
  $vars['previous'] = db_result(db_query('SELECT MAX(sid) FROM {webform_submissions} WHERE nid = %d AND sid < %d', array($vars['node']->nid, $vars['submission']->sid)));
  $vars['next'] = db_result(db_query('SELECT MIN(sid) FROM {webform_submissions} WHERE nid = %d AND sid > %d', array($vars['node']->nid, $vars['submission']->sid)));
  $vars['previous_url'] = $start_path . $vars['node']->nid . '/submission/' . $vars['previous'] . ($vars['mode'] == 'form' ? '/edit' : '');
  $vars['next_url'] = $start_path . $vars['node']->nid . '/submission/' . $vars['next'] . ($vars['mode'] == 'form' ? '/edit' : '');
}

/**
 * Preprocess function for webform-submission-navigation.tpl.php
 */
function template_preprocess_webform_submission_information(&$vars) {
  $vars['account'] = user_load(array('uid' => $vars['submission']->uid));
}
