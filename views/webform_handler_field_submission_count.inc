<?php
// $Id$

/**
 * Field handler to present the submission count of a node to the user.
 */
class webform_handler_field_submission_count extends views_handler_field {
  function construct() {
    parent::construct();
    $this->count_type = $this->definition['count_type'];

    if ($this->count_type == 'node') {
      $this->additional_fields['nid'] = 'nid';
    }
    else if ($this->count_type == 'users') {
      $this->additional_fields['uid'] = 'uid';
    }
  }

  function option_definition() {
    $options = parent::option_definition();
    $options['label'] = array('default' => '# of Submissions', 'translatable' => TRUE);
    return $options;
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    global $user;

    if ($this->count_type == 'node') {
      module_load_include('inc', 'webform', 'includes/webform.submissions');
      $node = node_load($values->{$this->aliases['nid']});
      if (webform_results_access($node, 'access webform results')) {
        $count = webform_get_submission_count($node->nid);
        $output = l($count, "node/$node->nid/webform-results");
      }
      else {
        $count = webform_get_submission_count($node->nid, $user->uid);
        $output = l($count, "node/$node->nid/submissions");
      }
    }
    else if ($this->count_type == 'users') {
      $sql = "SELECT COUNT(sid) FROM {webform_submissions} WHERE uid = %d";
      $output = db_result(db_query($sql, $values->{$this->aliases['uid']}));
    }

    return $output;
  }
}
