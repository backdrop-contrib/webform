<?php
  // $Id$

  /** 
   * @author Pontus Ullgren <ullgren@user.sourceforge.net>
   * @package module_webform
   * @copyright Pontus Ullgren 2004
   **/

function _webform_reporttable() {
  $title = t('webform submissions');
  $content = 'kalle';

  if ( arg(2) ) {
    // Get all the submitted values for the node
    $query = 'SELECT sd.sid as sid, c.cid as cid, sd.name as name, sd.data as data'.
      ' FROM '.
      ' webform_submitted_data sd CROSS JOIN webform_component c on sd.nid = c.nid '.
      ' WHERE sd.nid = '.arg(2).' '.
      ' GROUP BY sd.sid, sd.name '.
      ' ORDER BY sd.sid, c.cid ';
    /*
     SELECT sd.sid, c.cid, sd.name, sd.data
     FROM webform_submitted_data sd
     CROSS JOIN webform_component c
     on sd.nid = c.nid
     GROUP BY sd.sid, sd.name
     ORDER BY sd.sid, c.cid 
    */

    $res = db_query($query);

    if(arg(3) == 'download') {
      // TODO: Download the values as a CSV file.
    }
    else {
      // TODO: View the submitted entries as a table.
      $first = true;
      $previous = -1;
      $cell = array();

      $header[] = t('#');
      while ($field = db_fetch_object($res)) {
        if ( ($previous != -1) 
             && ($previous != $field->sid)) {
          
          $rows[] = array_merge(array($previous), $cell);
          unset($cell);
          $first = false;
        }

        if($first) {
          $header[] = $field->name;
        }
        
        if ( $field->name == '__timestamp' ) {
          $cell[] = format_date($field->data, 'small');
        }
        else {
          $cell[] = $field->data;
        }
        $previous = $field->sid;
      }
      // and the last one ...
      $rows[] = array_merge(array($previous), $cell);

      $content = theme('table', $header, $rows);
    }
  }
  else {
    $header = array(
                    t('Title'), 
                    array('data' => t('Operations'),
                          'colspan' => '2')
                    );
    $result = db_query("SELECT nid, title FROM {node} WHERE type='webform'");
    
    while ($node = db_fetch_object($result)) {
      $rows[] = array($node->title,
                      l(t('view submissions'),'webform/table/'.$node->nid),
                      l(t('edit'), 'node/'.$node->nid.'/edit'));
    }
    $content = theme('table', $header, $rows);
  }
  print theme('page', $content, $title);
}
?>