<?php
  // $Id$

  /** 
   * This file includes helper functions for creating reports for webform.module
   *
   * @author Pontus Ullgren <ullgren@user.sourceforge.net>
   * @package module_webform
   * @copyright Pontus Ullgren 2004
   **/

/** function _webform_results_submissions() is a database accessor function designed to return lists
 *  of submissions for a given form.
 * @param  integer $nid the node id of the webform
 */
function _webform_results_submissions($nid) {
  // Get all the submitted values for the node
  $query = 'SELECT ws.sid as sid, ws.submitted as submitted '.
    ' FROM '.
    ' {webform_submissions} ws '.
    ' WHERE ws.nid = %d '.
    ' ORDER BY ws.sid ';

  $res = db_query($query, $nid);
  
  $header[] = t('#');
  $header[] = t('Submitted');
  $header[] = array('data' => t('Operations'), 'colspan' => '3');

  while ($field = db_fetch_object($res)) {
    $rows[] = array($field->sid,
                    format_date($field->submitted, 'small'),
                    l(t('View'),"node/$nid",NULL,"sid=".$field->sid,NULL,FALSE),
                    l(t('Delete'),"node/$nid/results/delete/".$field->sid ,NULL,NULL,NULL,FALSE)
                    );
  }
  
  return theme('table', $header, $rows);
}

/* This function creates a table for all ..
 * @param  integer  $nid the node ID of the node to show results for.
 */
function _webform_results_table($nid) {
  include_once(drupal_get_path('module', 'webform')."/webform.inc");

  $first = true;
  $cell = array();
  $rows = array();
  
  // Get all the submitted values for the node
  $query = 'SELECT ws.sid '.
    ' FROM '.
    ' {webform_submissions} ws '.
    ' WHERE '.
    ' ws.nid = %d ';
  
  $res = db_query($query, $nid);
  
  $header[] = t('#');
  $header[] = t('Time');
  
  while ($field = db_fetch_object($res)) {
    $submission = _webform_fetch_submission($field->sid);
    $cell[] = $submission['sid'];
    $cell[] = format_date($submission['submitted'], 'small');
    
    foreach ($submission['data'] as $cid => $data) {
      if($first) {
        $header[] = $data['name'];
      }

      if (count($data['value']) > 1) {
        $cell[] = theme('item_list', $data['value']);
      }
      else {
        $cell[] = $data['value']['0'];
      }
    }
    $first = false;
    $rows[] = $cell;
    unset($cell);
  } 
  return theme('table', $header, $rows);
} // end function _webform_results_table($nid)

function _webform_results_download($nid) {
  include_once(drupal_get_path('module', 'webform')."/webform.inc");

/*
 * The CSV requires that the data be presented in a flat file.  In order 
 * to maximize useability to the Excel community and minimize subsequent 
 * stats or spreadsheet programming this program extracts data from the 
 * various records for a given session and presents them as a single file 
 * where each row represents a single record.
 * The structure of the file is:
 *   Heading Line 1: Gives group overviews padded by empty cells to the 
 *                   next group.  A group may be a question and corresponds 
 *                   to a component in the webform philosophy. Each group 
 *                   overview will have a fixed number of columns beneath it.
 *   Heading line 2: gives column headings
 *   Data line 1 .....
 *   Data line 2 .....
 *   
 * An example of this format is given below.  Note the columns have had spaces 
 * added so the columns line up.  This is not the case with actual file where 
 * a column may be null.  Note also, that multiple choice questions as produced
 * by checkboxes or radio buttons have been presented as "yes" or "no" and the
 * actual choice text is retained only in the header line 2.
 * Data from text boxes and input fields are written out in the body of the table.
 *
 *   Submission Details,    ,   ,      ,Question 1,        ,        ,..,        ,Question 2,        ,        ,..,        ,Question n
 *   timestamp         ,time,SID,userid,Choice 1  ,Choice 2,Choice 3,..,Choice n,Choice 1  ,Choice 2,Choice 3,..,Choice n,Comment
 *   21 Feb 2005       ,1835,23 ,34    ,Yes       ,No      ,No      ,..,No      ,Yes       ,Yes     ,Yes     ,..,Yes     ,My comment
 *   23 Feb 2005       ,1125,24 ,89    ,Yes       ,Yes     ,No      ,..,No      ,Yes       ,Yes     ,Yes     ,..,Yes     ,Hello
 *   ...............................................................................................................
 *   27 Feb 2005       ,1035,56 ,212   ,Yes       ,No      ,No      ,..,No      ,Yes       ,No      ,Yes     ,..,Yes     ,How is this?
 */
  $node = node_load(array('nid' => $nid));

  $file_name = tempnam(variable_get('file_directory_temp', FILE_DIRECTORY_TEMP), 'webform');
  $handle = @fopen($file_name, 'w'); // The @ suppresses errors
  $header[0] .= $node->title.",,,";
  $header[1] .= "Submission Details,,,";
  $header[2] .= "Serial,Timestamp,SID,";
  
  // Compile header information
  foreach($node->webformcomponents_name as $key => $name) {
    $extra = unserialize ($node->webformcomponents_extra[$key]);
    if ( $node->webformcomponents_type[$key] == 'select' ) {
      $header[1] .= "\"$name\",";
      $items = split("\n", $extra['items']);
      $header[0] .= str_pad('', count($items) - 1,',');
      $header[1] .= str_pad('', count($items) - 1,',');
      foreach($items as $k => $v) {
        $v = trim($v);
        $header[2] .= "$v,";
      }
    }
    else {
      $header[1] .= ",";
      $header[2] .= "\"$name\",";
    }
  }

  // Write header information
  $file_record = $header[0] ."\n". $header[1] ."\n". $header[2] . "\n";
  @fwrite($handle,$file_record);

  // Compile data from submissions
  // Get all the submissions
  $query = 'SELECT ws.sid '.
    ' FROM '.
    ' {webform_submissions} ws '.
    ' WHERE '.
    ' ws.nid = %d ';
  
  $res = db_query($query, $nid);
  
  $rowcount = 0;
  while ($field = db_fetch_object($res)) {
    $submission = _webform_fetch_submission($field->sid);
    $row = ++$rowcount.",\"". format_date($submission['submitted'], 'small')."\",".$submission['sid'].',';
    foreach($node->webformcomponents_name as $key => $name) {
      if ( $node->webformcomponents_type[$key] == 'select' ) {
        $extra = unserialize ($node->webformcomponents_extra[$key]);
        $items = split("\n", $extra['items']);
        foreach($items as $k => $v) {
          $v = trim($v);
          $row .= (in_array($v, $submission['data'][$key]['value'])?'Yes':'No').',';
        }
      }
      else {
        $row .= '"'.$submission['data'][$key]['value']['0'].'",';
      }
    }
    // Write data from submissions
    @fwrite($handle, $row."\n");
  }
  // Close the file 
  @fclose($handle);

  drupal_set_header("Content-type: text/plain; charset=utf-8");
  drupal_set_header("Content-Disposition: attachment; filename=" . preg_replace('/\.$/', '', str_replace(' ', '_', $node->title)) . ".csv");

  @readfile($file_name);  // The @ makes it silent
  @unlink($file_name);  // Clean up, the @ makes it silent
  exit(0);
} // end function _webform_results_download

/* _webform_results_analysis - Provides simple analysis of a series of webform submission.
 * @return to stdio a themeatic HTML rendering of the page.
 */
function _webform_results_analysis($nid) {
  $query = 'SELECT nid, cid, name, type, value, extra '.
    ' FROM {webform_component} '.
    ' WHERE nid = %d '.
    ' ORDER BY weight';

  $components = db_query($query, $nid);

  $rows = array();
  $question_number=0;

  $headers = array( t('Q'), 
                   array('data' => t('responses'), 'colspan' => '10')
                 );

  while ($component = db_fetch_object($components)) {
    $question_number++;
    
    // Do component specific call
    $crows = module_invoke_all('webform_analysis', $component);
    if ( is_array($crows) ) {
      $row[0] =  array( 'data' => '<strong>'.$question_number.'</strong>', 'rowspan' => count($crows)+1, 'valign' => 'top');
      $row[1] = array( 'data' => '<strong>'.$component->name.'</strong>', 'colspan' => '10');
      $rows = array_merge($rows, array_merge(array($row), $crows));
    }
  }
  if ( is_array($rows) )
    return theme('table', $headers, $rows);
  else
    return '';
}

/** Hook for creating the analysis parts for the default components.
 * @return rows to be displayed on the analysis page.
 */
function webform_webform_analysis($component) {
  switch($component->type) {
  case 'textarea':
  case 'textfield':
  case 'hidden':
  case 'email':
    $query = 'SELECT data '.
      ' FROM webform_submitted_data '.
      ' WHERE nid = %d '.
      '  AND cid = %d';
    $nonblanks = 0;
    $submissions = 0;
    $wordcount = 0;

    $result = db_query($query, $component->nid, $component->cid);
    while ($data = db_fetch_array($result)) {
      if ( strlen(trim($data['data'])) > 0 ) {
        $nonblanks++;
        $wordcount += str_word_count(trim($data['data']));
      }
      $submissions++;
    }
    
    $rows[0] = array( t('Left Blank'), ($submissions-$nonblanks));
    $rows[1] = array( t('User entered value'), $nonblanks);
    $rows[2] = array( t('Average submission length in words (ex blanks)'), 
                      ($nonblanks!=0?($wordcount/$nonblanks):'0'));
    return $rows;
    // break;
  case 'select':
    $query = 'SELECT data, count(data) as datacount '.
      ' FROM webform_submitted_data '.
      ' WHERE nid = %d '.
      '  AND cid = %d '.
      ' GROUP BY data ';
    $result = db_query($query, $component->nid, $component->cid);
    $rows = array();
    while ($data = db_fetch_array($result)) {
      $rows[] = array($data['data'], $data['datacount']);
    }
    return $rows;
    // break;
  case 'scale':
    $query = 'SELECT MAX(data) AS maxgrade, MIN(data) AS mingrade, AVG(data) AS avggrade '.
      ' FROM webform_submitted_data '.
      ' WHERE nid = %d '.
      '  AND cid = %d ';
    $result = db_fetch_array(db_query($query, $component->nid, $component->cid));
    
    $rows[0] = array(t('Max grade'), $result['maxgrade']);
    $rows[1] = array(t('Min grade'), $result['mingrade']);
    $rows[2] = array(t('Average grade'), $result['avggrade']);

    return $rows;
    // break;
  }
}

?>