<?php
// $Id$

/**
 * @file webform_poll.php
 * 
 * Webform and Poll modules both claim the dynamic path node/x/results. Because
 * the menu system in Drupal 6 will load the access callback of the first module
 * for any modules that claim that path, this can lead to some problems. This
 * is a menu_alter hack that allows both webform and poll module to co-exist
 * claiming the same path and returning the correct access check or page
 * callback depending on the type of node loaded.
 */

/**
 * Implementation of hook_menu_alter().
 */
function webform_menu_alter(&$callbacks) {
  // Poll module and webform both claim the node/x/results callback, but because
  // poll is lazy about it and just claim the space for *all* nodes, we need
  // to give poll a special menu loader that only loads for polls.
  $callbacks['node/%node/results']['page callback'] = 'webform_menu_poll_page';
  $callbacks['node/%node/results']['page arguments'] = array(1);
  $callbacks['node/%node/results']['access callback'] = 'webform_menu_poll_access';
  $callbacks['node/%node/results']['access arguments'] = array(1);
}

/**
 * Page callback to switch betwen the correct node type.
 */
function webform_menu_poll_page($node) {
  if ($node->type == 'poll') {
    return poll_results($node);
  }
  else {
    return webform_results($node);
  }
}

/**
 * Access callback to switch betwen the correct node type.
 */
function webform_menu_poll_access($node) {
  if ($node->type == 'poll') {
    return _poll_menu_access($node, 'access content', TRUE);
  }
  else {
    return user_access('access webform results');
  }
}
