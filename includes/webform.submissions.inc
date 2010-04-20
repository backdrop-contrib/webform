<?php
// $Id$

/**
 * @file
 * This file is loaded when handling submissions, either submitting new,
 * editing, or viewing. It also contains all CRUD functions for submissions.
 *
 * @author Nathan Haug <nate@lullabot.com>
 */

/**
 * Given an array of submitted values, flatten it into data for a submission.
 */
function webform_submission_data($node, $submitted) {
  $data = array();

  foreach ($submitted as $cid => $values) {
    // Don't save pagebreaks as submitted data.
    if ($node->webform['components'][$cid]['type'] == 'pagebreak') {
      continue;
    }

    if (is_array($values)) {
      $data[$cid]['value'] = $values;
    }
    else {
      $data[$cid]['value'][0] = $values;
    }
  }

  return $data;
}

function webform_submission_update($node, $submission) {
  // Allow other modules to modify the submission before saving.
  foreach (module_implements('webform_submission_presave') as $module) {
    $function = $module . '_webform_submission_presave';
    $function($node, $submission);
  }

  // Update the main submission info.
   db_update('webform_submissions')
     ->fields(array(
       'uid' => $submission->uid,
       'submitted' => $submission->submitted,
       'remote_addr' => $submission->remote_addr,
       'is_draft' => $submission->is_draft,
     ))
     ->condition('sid', $submission->sid)
     ->execute();

  // If is draft, only delete data for components submitted, to
  // preserve any data from form pages not visited in this submission.
  if ($submission->is_draft) {
    db_delete('webform_submitted_data')
      ->condition('sid', $submission->sid)
      ->condition('cid', array_keys($submission->data), 'IN')
      ->execute();
  }
  else {
    db_delete('webform_submitted_data')
      ->condition('sid', $submission->sid)
      ->execute();
  }

  // Then re-add submission data to the database.
  $submission->is_new = FALSE;
  webform_submission_insert($node, $submission);

  module_invoke_all('webform_submission_update', $node, $submission);

  return $submission->sid;
}

function webform_submission_insert($node, $submission) {
  // The submission ID may already be set if being called as an update.
  if (!isset($submission->sid) && (!isset($submission->is_new) || $submission->is_new == FALSE)) {
    // Allow other modules to modify the submission before saving.
    foreach (module_implements('webform_submission_presave') as $module) {
      $function = $module . '_webform_submission_presave';
      $function($node, $submission);
    }

    $submission->sid = db_insert('webform_submissions')
      ->fields(array(
        'nid' => $node->nid,
        'uid' => $submission->uid,
        'submitted' => $submission->submitted,
        'remote_addr' => $submission->remote_addr,
        'is_draft' => $submission->is_draft,
      ))
      ->execute();
    $is_new = TRUE;
  }

  foreach ($submission->data as $cid => $values) {
    foreach ($values['value'] as $delta => $value) {
      db_insert('webform_submitted_data')
        ->fields(array(
          'nid' => $node->nid,
          'sid' => $submission->sid,
          'cid' => $cid,
          'no' => $delta,
          'data' => $value
        ))
        ->execute();
    }
  }

  // Invoke the insert hook after saving all the data.
  if (isset($is_new)) {
    module_invoke_all('webform_submission_insert', $node, $submission);
  }

  return $submission->sid;
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
      webform_component_invoke($component['type'], 'delete', $component, $submission->data[$cid]['value']);
    }
  }

  // Delete any anonymous session information.
  if (isset($_SESSION['webform_submission'][$submission->sid])) {
    unset($_SESSION['webform_submission'][$submission->sid]);
  }

  db_delete('webform_submitted_data')
    ->condition('nid', $node->nid)
    ->condition('sid', $submission->sid)
    ->execute();
  db_delete('webform_submissions')
    ->condition('nid', $node->nid)
    ->condition('sid', $submission->sid)
    ->execute();

  module_invoke_all('webform_submission_delete', $node, $submission);
}

/**
 * Confirm form to delete a single form submission.
 *
 * @param $form
 *   The new form array.
 * @param $form_state
 *   The current form state.
 * @param $node
 *   The node for which this webform was submitted.
 * @param $submission
 *   The submission to be deleted (from webform_submitted_data).
 */
function webform_submission_delete_form($form, $form_state, $node, $submission) {
  webform_set_breadcrumb($node, $submission);

  // Keep the NID and SID in the same location as the webform_client_form().
  // This helps mollom identify the same fields when deleting a submission.
  $form['#tree'] = TRUE;
  $form['details']['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['details']['sid'] = array(
    '#type' => 'value',
    '#value' => $submission->sid,
  );

  $question = t('Are you sure you want to delete this submission?');

  if (isset($_GET['destination'])) {
    $destination = $_GET['destination'];
  }
  elseif (user_access('access all webform results') || ($node->uid == $user->uid && user_access('access own webform results'))) {
    $destination = 'node/' . $node->nid . '/webform-results';
  }
  else {
    $destination = 'node/' . $node->nid . '/submissions';
  }

  return confirm_form($form, NULL, $destination, $question, t('Delete'), t('Cancel'));
}

function webform_submission_delete_form_submit($form, &$form_state) {
  $node = node_load($form_state['values']['details']['nid']);
  $submission = webform_get_submission($form_state['values']['details']['nid'], $form_state['values']['details']['sid']);
  webform_submission_delete($node, $submission);
  drupal_set_message(t('Submission deleted.'));

  $form_state['redirect'] = 'node/' . $node->nid . '/webform-results';
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
    $output = drupal_get_form('webform_client_form_' . $node->nid, $node, $submission);
  }
  else {
    $output = webform_submission_render($node, $submission, NULL, $format);
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
    $navigation = ($mode != 'pdf') ? theme('webform_submission_navigation', array('node' => $node, 'submission' => $submission, 'mode' => $mode)) : NULL;
    $information = theme('webform_submission_information', array('node' => $node, 'submission' => $submission));
  }
  else {
    $navigation = NULL;
    $information = NULL;
  }

  // Disable the page cache for anonymous users viewing or editing submissions.
  if (!$user->uid) {
    webform_disable_page_cache();
  }

  return theme('webform_submission_page', array('node' => $node, 'submission' => $submission, 'submission_content' => $output, 'submission_navigation' => $navigation, 'submission_information' => $information));
}

/**
 * Print a Webform submission for display on a page or in an e-mail.
 */
function webform_submission_render($node, $submission, $email, $format) {
  $component_tree = array();
  $renderable = array();
  $page_count = 1;
  $excluded_components = isset($email) ? $email['excluded_components'] : array();

  _webform_components_tree_build($node->webform['components'], $component_tree, 0, $page_count);

  // Recursively add components to the form.
  foreach ($component_tree['children'] as $cid => $component) {
    if (!in_array($cid, $excluded_components) && _webform_client_form_rule_check($node, $component, $component['page_num'], NULL, $submission)) {
      _webform_client_form_add_component($component, NULL, $renderable, $renderable, $submission, $format);
    }
  }

  return drupal_render($renderable);
}

/**
 * Return all the submissions for a particular node.
 *
 * @param $filters
 *   An array of filters to apply to this query. Usually in the format
 *   array('nid' => $nid, 'uid' => $uid). A single integer may also be passed
 *   in, which will be equivalent to specifying a $nid filter.
 * @param $header
 *   If the results of this fetch will be used in a sortable
 *   table, pass the array header of the table.
 * @param $pager_count
 *   Optional. The number of submissions to include in the results.
 */
function webform_get_submissions($filters = array(), $header = NULL, $pager_count = 0) {
  $submissions = array();

  if (!is_array($filters)) {
    $filters = array('nid' => $filters);
  }

  // No need to find SIDs if it was given to us.
  if (isset($filters['sid'])) {
    $sids = array($filters['sid']);
  }
  // Build the list of SIDs that need to be retrieved.
  else {
    $pager_query = db_select('webform_submissions')->fields('webform_submissions', array('sid'));
    foreach ($filters as $column => $value) {
      $pager_query->condition($column, $value);
    }

    if (isset($filters['uid']) && $filters['uid'] === 0) {
      $anonymous_sids = array_keys($_SESSION['webform_submission']);
      if (count($anonymous_sids)) {
        $pager_query->condition('sid', $anonymous_sids, 'IN');
      }
    }

    if (is_array($header)) {
      $pager_query->extend('TableSort')->orderByHeader($header);
    }
    else {
      $pager_query->orderBy('sid', 'ASC');
    }

    if ($pager_count) {
      $pager_query->extend('PagerDefault')->limit($pager_count);
    }
    $result = $pager_query->execute();

    $sids = array();
    foreach ($result as $row) {
      $sids[] = $row->sid;
      $submissions[$row->sid] = FALSE;
    }
  }

  // If there are no submissions being retrieved, return an empty array.
  if (empty($sids)) {
    return $submissions;
  }

  // Query the required submission data.
  $query = db_select('webform_submissions', 's');
  $query->leftJoin('webform_submitted_data', 'sd', 'sd.sid = s.sid');
  $query->leftJoin('users', 'u', 'u.uid = s.uid');
  $query
    ->fields('s')
    ->fields('sd')
    ->fields('u', array('name'))
     ->condition('s.sid', $sids, 'IN')
    ->orderBy('sd.sid', 'ASC')
    ->orderBy('sd.cid', 'ASC')
    ->orderBy('sd.no', 'ASC');
  $result = $query->execute();

  // Convert the queried rows into submissions.
  $previous = 0;
  foreach ($result as $row) {
    if ($row->sid != $previous) {
      $submissions[$row->sid] = new stdClass();
      $submissions[$row->sid]->sid = $row->sid;
      $submissions[$row->sid]->nid = $row->nid;
      $submissions[$row->sid]->submitted = $row->submitted;
      $submissions[$row->sid]->remote_addr = $row->remote_addr;
      $submissions[$row->sid]->uid = $row->uid;
      $submissions[$row->sid]->name = $row->name;
      $submissions[$row->sid]->is_draft = $row->is_draft;
    }
    $submissions[$row->sid]->data[$row->cid]['value'][$row->no] = $row->data;
    $previous = $row->sid;
  }

  foreach (module_implements('webform_submission_load') as $module) {
    $function = $module . '_webform_submission_load';
    $function($submissions);
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
    $query = db_select('webform_submissions')
      ->countQuery()
      ->condition('nid', $nid);
    $arguments = array($nid);
    if ($uid !== NULL) {
      $query->condition('uid', $uid);
    }
    if ($uid === 0) {
      $submissions = isset($_SESSION['webform_submission']) ? $_SESSION['webform_submission'] : array();
      $query->condition('sid', $submissions, 'IN');
    }

    $counts[$nid][$uid] = $query->execute()->fetchField();
  }
  return $counts[$nid][$uid];
}

/**
 * Fetch a specified submission for a webform node.
 */
function webform_get_submission($nid, $sid, $reset = FALSE) {
  static $submissions = array();

  if ($reset) {
    $submissions = array();
    if (!isset($sid)) {
      return;
    }
  }

  // Load the submission if needed.
  if (!isset($submissions[$sid])) {
    $new_submissions = webform_get_submissions(array('sid' => $sid));
    $submissions[$sid] = $new_submissions[$sid];
  }

  // Ensure that the requested NID matches the submission NID.
  if ($submissions[$sid]->nid != $nid) {
    return FALSE;
  }

  return $submissions[$sid];
}

function _webform_submission_spam_check($to, $subject, $from, $headers = array()) {
  $headers = implode('\n', (array)$headers);
  // Check if they are attempting to spam using a bcc or content type hack.
  if (preg_match('/(b?cc\s?:)|(content\-type:)/i', $to . "\n" . $subject . "\n" . $from . "\n" . $headers)) {
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
  $query = db_select('webform_submissions')
    ->countQuery()
    ->condition('nid', $node->nid)
    ->condition('is_draft', 0);

  if ($node->webform['submit_interval'] != -1) {
    $query->condition('submitted', REQUEST_TIME - $node->webform['submit_interval'], '>');
  }

  if ($user->uid) {
    $query->condition('uid', $user->uid);
  }
  else {
    $query->condition('remote_addr', ip_address());
  }

  // Fetch all the entries from the database within the submit interval with this username and IP.
  $num_submissions_database = $query->execute()->fetchField();

  // Double check the submission history from the users machine using cookies.
  $num_submissions_cookie = 0;
  if ($user->uid == 0 && variable_get('webform_use_cookies', 0)) {
    $cookie_name = 'webform-' . $node->nid;

    if (isset($_COOKIE[$cookie_name]) && is_array($_COOKIE[$cookie_name])) {
      foreach ($_COOKIE[$cookie_name] as $key => $timestamp) {
        if ($node->webform['submit_interval'] != -1 && $timestamp <= REQUEST_TIME - $node->webform['submit_interval']) {
          // Remove the cookie if past the required time interval.
          setcookie($cookie_name . '[' . $key . ']', '', 0);
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

  $previous_query = db_select('webform_submissions')
    ->condition('nid', $vars['node']->nid)
    ->condition('sid', $vars['submission']->sid, '<');
  $previous_query->addExpression('MAX(sid)');

  $next_query = db_select('webform_submissions')
    ->condition('nid', $vars['node']->nid)
    ->condition('sid', $vars['submission']->sid, '>');
  $next_query->addExpression('MIN(sid)');

  $vars['previous'] = $previous_query->execute()->fetchField();
  $vars['next'] = $next_query->execute()->fetchField();
  $vars['previous_url'] = $start_path . $vars['node']->nid . '/submission/' . $vars['previous'] . ($vars['mode'] == 'form' ? '/edit' : '');
  $vars['next_url'] = $start_path . $vars['node']->nid . '/submission/' . $vars['next'] . ($vars['mode'] == 'form' ? '/edit' : '');
}

/**
 * Preprocess function for webform-submission-navigation.tpl.php
 */
function template_preprocess_webform_submission_information(&$vars) {
  $vars['account'] = user_load($vars['submission']->uid);
}
