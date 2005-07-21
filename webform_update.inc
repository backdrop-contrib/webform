<?php
// $Id$

/*
function _webform_update_470() function _webform_update_470() {
  $ret = array();
    

   
  
  return $ret[];
}
*/
function _webform_update_470() {
  $ret = array();
  $ret[] = update_sql("CREATE TABLE {webform_tmp} ( ".
                      "  nid int(10) unsigned NOT NULL default '0', ".
                      "  sid int(10) unsigned NOT NULL default '0', ".
                      "  cid int(10) unsigned NOT NULL default '0', ".
                      "  no int(10) unsigned NOT NULL default '0', ".
                      "  data longtext, ".
                      " PRIMARY KEY  (nid,sid,cid,no) ".
                      " ) ");
  $result = db_query("SELECT ws.nid, ws.sid, wc.cid, ws.name, ws.data ".
                     " FROM {webform_submitted_data} ws, {webform_component} wc ".
                     " WHERE ws.nid = wc.nid ".
                     "      AND ws.name = wc.name ");
  
  while ($row = db_fetch_object($result)) {    
    $data = unserialize($row->data);
    if ( is_array($data) ) {
      foreach($data as $k => $v) {
        db_query("INSERT INTO {webform_tmp} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')",
                 $row->nid, $row->sid, $row->cid, $k, $v);
      }
    }
    else {
      db_query("INSERT INTO {webform_tmp} (nid, sid, cid, no, data) VALUES (%d, %d, %d, %d, '%s')",
               $row->nid, $row->sid, $row->cid, 0, $row->data);
    }
  }
  $ret[] = array(1, "Converting old submissions", "<div style=\"color: green;\">OK</div>\n");
  
  $ret[] = update_sql("DROP TABLE {webform_submitted_data}");
  $ret[] = update_sql("ALTER TABLE {webform_tmp} RENAME TO {webform_submitted_data}");

  $ret[] = update_sql("ALTER TABLE {webform_submissions} RENAME TO {webform_submissions_old}");

  $ret[] = update_sql("CREATE TABLE {webform_submissions} ( ".
                      " nid int(10) unsigned NOT NULL default '0', ".
                      " sid int(10) unsigned NOT NULL default '0', ".
                      " submitted int(11)  NOT NULL default '0', ".
                      " PRIMARY KEY  (nid, sid) ".
                      ")" );
  
  return $ret;
}

/**
 * Makes updates to the database structure.
 **/
function _webform_doupdate($installed_version, $current_version) {
  $ret = array();
    // Check to see if we should do a update.
    print "<strong>Upgrading webform from ".$installed_version['build'] .
      " to ". $current_version['build']."</strong><br />\n";

    // Upgrading from original version    
    if ( $installed_version['build'] <= 1 ) {
      $ret[] = array(1 => "<strong>Build 1</strong><br />\n", 2 => "");
      // Add the table webform_submitted_data
      $ret[] = update_sql("CREATE TABLE {webform_submited_data} ".
                          "( nid int(10) unsigned not null, ".
                          "sid int(10) unsigned not null, ".
                          "name varchar(255) not null, ".
                          "data blob, ".
                          "PRIMARY KEY(nid, sid, name))");
 
      // Converting data from old submission table.
      $ret[] = _webform_convert_old_submissions();
    }
    if ( $installed_version['build'] <= 1 ) {
      $ret[] = array(1 => "<strong>Build 4.5.0</strong><br />\n", 2 => "");
      // Change webform_component.extra from varchar(128) -> text
      $ret[] = update_sql("ALTER TABLE {webform_component} MODIFY extra TEXT");
      // Change webform_submited_data.data blob -> longtext
      $ret[] = update_sql("ALTER TABLE {webform_submited_data} MODIFY data LONGTEXT");
    }
    if ( $installed_version['build'] < 460 ) {
      $ret[] = array(1 => "<strong>Build 4.6.0</strong><br />\n", 2 => "");
      // Update webform_submited_data to webform_submitted_data
      $ret[] = update_sql("ALTER TABLE {webform_submited_data} RENAME TO {webform_submitted_data}");
    }
    if ( $installed_version['build'] < 461 ) {
      $ret[] = array(1 => "<strong>Build 4.6.1</strong><br />\n", 2 => "");
      // Update webform.email varchar(50) -> varchar(255)
      $ret[] = update_sql(" ALTER TABLE {webform} MODIFY email varchar(255)");
      // Update from lable to label in webform_component
      $ret[] = update_sql(" UPDATE {webform_component} SET type = 'label' WHERE type = 'lable'");
    }
    if ( $installed_version['build'] < 462 ) {
      $ret[] = array(1 => "<strong>Build 4.6.2</strong><br />\n", 2 => "");
      // Update webform.confirm varchar(255) -> text and change name to "confirmation"
      $ret[] = update_sql(" ALTER TABLE {webform} CHANGE confirm confirmation text");
    }
    if ( $installed_version['build'] < 463 ) {
      $ret[] = array(1 => "<strong>Build 4.6.3</strong><br />\n", 2 => "");
      // No more lazy update ...
      $ret[] = update_sql("ALTER TABLE {webform} ADD COLUMN email_subject varchar(255)");
      $ret[] = update_sql("ALTER TABLE {webform} ADD COLUMN email_from varchar(255)");
    }
    
    if ( $installed_version['build'] < 470 ) {
      $ret = array_merge($ret, _webform_update_470());
    }
    
    // Set the $current_version
    variable_set("webform_version", $current_version);

    print "<pre>\n";
    foreach ( $ret as $return ) {
      print $return[1];
      print $return[2];
    }
    print "</pre>\n";
    
    print "<strong>Done!</strong><br/>\n";
    print l(t('Go back'), 'admin/settings/webform')."<br/>\n";
}

/** 
 * Function to convert the old XML formated submissions into the newer format.
 **/
function _webform_convert_old_submissions() {
  // Select all from webform_submissions.
  
  /* 

  $result = db_query("SELECT nid, sid, uid, UNIX_TIMESTAMP(created) as created, data ".
                   "FROM {webform_submissions}");
  
  while ($row = db_fetch_object($result) ) {
    // TODO: Parse each XML string ...
    // ... and insert the data into the new table.
  }
  */
  return array('1', "Converting old submissions", "<div style=\"color: red;\">Not implemented!</div>\n");
} // end function _webform_convert_old_submissions
?>