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

  $header = array();
  $rows = array();
  $cell = array();

  $header[] = t('#');
  $header[] = t('Time');

  // Get all the component cid and names for the node
  $query = 'SELECT cid, name, type, extra FROM {webform_component} WHERE nid = %d ORDER BY weight, name';
  $res = db_query($query, $nid);
  while($component = db_fetch_array($res)) {
    $components[] = $component;
  }

  // Get all the submissions for the node
  $query = 'SELECT ws.sid FROM {webform_submissions} ws WHERE ws.nid = %d ';
  $res = db_query($query, $nid);

  // A row for each submission
  while ($field = db_fetch_array($res)) {
    $submission = _webform_fetch_submission($field['sid']);
    $cell[] = $submission['sid'];
    $cell[] = $submission['submitted'];
    $componentHeaders = array();

    // A cell for each component
    _webform_load_components();
    foreach($components as $component) {
      $component['extra'] = unserialize($component['extra']);
      $tableFunction = "_webform_table_data_".$component['type'];
      if (function_exists($tableFunction)) {
        $submissionOutput = $tableFunction($submission['data'][$component['cid']],$component);
        if ($submissionOutput !== NULL) {
          $componentHeaders[] = $component['name'];
          $cell[]   = $submissionOutput;
        }
      }
    }
    
    $rows[] = $cell;
    unset($cell);
  } 
  $header = array_merge($header,$componentHeaders);

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
  $header[0] .= $node->title.",,";
  $header[1] .= "Submission Details,,";
  $header[2] .= "Serial,Time,SID";
  // Compile header information
  _webform_load_components(); // Load all components
  if (is_array($node->webformcomponents)) {
    foreach($node->webformcomponents as $cid => $component) {
      $csvHeaderFunction   = "_webform_csv_headers_".$component['type'];
      if (function_exists($csvHeaderFunction)) {
        // Let each component determine its headers
        $componentHeader = $csvHeaderFunction($component);
        $header[0] .= ',"'.str_replace(array('"','\,'),array('""','","'),$componentHeader[0]).'"';
        $header[1] .= ',"'.str_replace(array('"','\,'),array('""','","'),$componentHeader[1]).'"';
        $header[2] .= ',"'.str_replace(array('"','\,'),array('""','","'),$componentHeader[2]).'"';
        
      }
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
    $row = ++$rowcount.",\"". format_date($submission['submitted'], 'small')."\",".$submission['sid'];
    foreach($node->webformcomponents as $cid => $component) {      
      $csvDataFunction   = "_webform_csv_data_".$component['type'];
      if (function_exists($csvDataFunction)) {
        // Let each component add its data
        $row .= ',"'.str_replace(array('"','\,'),array('""','","'),$csvDataFunction($submission['data'][$cid],$component)).'"';
        
      }
    }
    // Write data from submissions
    @fwrite($handle, $row."\n");
  }
  // Close the file 
  @fclose($handle);

  drupal_set_header("Content-type: text/csv; charset=utf-8");
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
    ' ORDER BY weight, name';

  $components = db_query($query, $nid);

  $rows = array();
  $question_number=0;

  $headers = array( t('Q'), 
                   array('data' => t('responses'), 'colspan' => '10')
                 );

  _webform_load_components (); // Load all component types
  while ($component = db_fetch_array($components)) {
    $question_number++;
    
    // Do component specific call
    $analysisFunction = "_webform_analysis_rows_".$component['type'];
    if (function_exists($analysisFunction)) {
      $crows = $analysisFunction($component);       
      if (is_array($crows) ) {
        $row[0] = array( 'data' => '<strong>'.$question_number.'</strong>', 'rowspan' => count($crows)+1, 'valign' => 'top');
        $row[1] = array( 'data' => '<strong>'.$component['name'].'</strong>', 'colspan' => '10');
        $rows = array_merge($rows, array_merge(array($row), $crows));
      }
    }
  }
  return theme('table', $headers, $rows);
}
