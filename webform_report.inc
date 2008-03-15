<?php
// $Id$

/**
 * This file includes helper functions for creating reports for webform.module
 * 
 * @author Nathan Haug <nate@lullabot.com>
 * @author Pontus Ullgren <ullgren@user.sourceforge.net>
 * @copyright Pontus Ullgren 2004
 */

// All functions within this file need the webform_submissions.inc.
include_once(drupal_get_path('module', 'webform') ."/webform_submissions.inc");

/**
 * Retrieve lists of submissions for a given webform.
 */
function webform_results_submissions($node) {
  $header = theme('webform_results_submissions_header', $node);
  $submissions = webform_get_submissions($node->nid, $header);
  return theme('webform_results_submissions', $node, $submissions);
}

/**
 * Theme the header of the submissions table.
 * 
 * This is done in it's own function so that webform can retrieve the header and
 * use it for sorting the results.
 */
function theme_webform_results_submissions_header($node) {
  return array(
    array('data' => t('#'), 'field' => 'sid', 'sort' => 'asc'),
    array('data' => t('Submitted'), 'field' => 'submitted'),
    array('data' => t('User'), 'field' => 'name'),
    array('data' => t('IP Address'), 'field' => 'remote_addr'),
    array('data' => t('Operations'), 'colspan' => user_access('clear webform results') ? '3' : '2'),
  );
}

/**
 * Theme the submissions tab of the webform results page.
 * 
 * @param $node
 *   The node whose results are being displayed.
 * @param $submissions
 *   An array of all submissions for this webform.
 */
function theme_webform_results_submissions($node, $submissions) {
  global $user;
  // This header has to be generated seperately so we can add the SQL necessary
  // to sort the results.
  $header = theme('webform_results_submissions_header', $node);

  $rows = array();
  foreach ($submissions as $sid => $submission) {
    $row = array(
      $sid,
      format_date($submission->submitted, 'small'),
      theme('username', $submission),
      $submission->remote_addr,
      l(t('View'), "node/$node->nid/submission/$sid"),
    );
    if ((user_access("edit own webform submissions") && ($user->uid == $submission->uid)) || user_access("edit webform submissions")) {
      $row[] = l(t('Edit'), "node/$node->nid/submission/$sid/edit");
    }
    else {
      $row[] = t('Edit');
    }
    if (user_access('clear webform results')) {
      $row[] = l(t('Delete'), "node/$node->nid/submission/$sid/delete");
    }
    $rows[] = $row;
  }

  return theme('table', $header, $rows);
}

/**
 * Create a table containing all submitted values for a webform node.
 */
function webform_results_table($node) {
  // Load Components.
  webform_load_components();

  // Get all the submissions for the node.
  $header = theme('webform_results_table_header', $node);
  $submissions = webform_get_submissions($node->nid, $header);

  return theme('webform_results_table', $node, $node->webform['components'], $submissions);
}

function theme_webform_results_table_header($node) {
  return array(
    array('data' => t('#'), 'field' => 'sid', 'sort' => 'asc'),
    array('data' => t('Submitted'), 'field' => 'submitted'),
    array('data' => t('User'), 'field' => 'name'),
    array('data' => t('IP Address'), 'field' => 'remote_addr'),
  );
}

/**
 * Theme the results table displaying all the submissions for a particular node.
 * 
 * @param $node
 *   The node whose results are being displayed.
 * @param $components
 *   An associative array of the components for this webform.
 * @param $submissions
 *   An array of all submissions for this webform.
 */
function theme_webform_results_table($node, $components, $submissions) {
  $header = array();
  $rows = array();
  $cell = array();

  // This header has to be generated seperately so we can add the SQL necessary.
  // to sort the results.
  $header = theme('webform_results_table_header', $node);

  // Generate a row for each submission.
  foreach ($submissions as $sid => $submission) {
    $cell[] = l($sid, 'node/'. $node->nid .'/submission/'. $sid);
    $cell[] = format_date($submission->submitted, "small");
    $cell[] = theme('username', $submission);
    $cell[] = $submission->remote_addr;
    $component_headers = array();

    // Generate a cell for each component.
    foreach ($node->webform['components'] as $component) {
      $table_function = "_webform_table_data_". $component['type'];
      if (function_exists($table_function)) {
        $submission_output = $table_function($submission->data[$component['cid']], $component);
        if ($submission_output !== NULL) {
          $component_headers[] = $component['name'];
          $cell[] = $submission_output;
        }
      }
    }

    $rows[] = $cell;
    unset($cell);
  }
  if (!empty($component_headers)) {
    $header = array_merge($header, $component_headers);
  }

  return theme('table', $header, $rows);
}


/**
 * Generate a Excel-readable CSV file containing all submissions for a webform.
 * 
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
 * 
 */
function webform_results_download($node) {
  $file_name = tempnam(variable_get('file_directory_temp', FILE_DIRECTORY_TEMP), 'webform');
  $handle = @fopen($file_name, 'w'); // The @ suppresses errors.
  $header[0] .= $node->title .",,,,";
  $header[1] .= "Submission Details,,,,,";
  $header[2] .= "Serial,SID,Time,IP Address,UID,Username";
  // Compile header information.
  webform_load_components(); // Load all components.
  foreach ($node->webform['components'] as $cid => $component) {
    $csv_header_function   = "_webform_csv_headers_". $component['type'];
    if (function_exists($csv_header_function)) {
      // Let each component determine its headers.
      $component_header = $csv_header_function($component);
      $header[0] .= ',"'. str_replace(array('"', '\,'), array('""', '","'), $component_header[0]) .'"';
      $header[1] .= ',"'. str_replace(array('"', '\,'), array('""', '","'), $component_header[1]) .'"';
      $header[2] .= ',"'. str_replace(array('"', '\,'), array('""', '","'), $component_header[2]) .'"';
    }
  }

  // Write header information.
  $file_record = $header[0] ."\n". $header[1] ."\n". $header[2] ."\n";
  @fwrite($handle, $file_record);

  // Get all the submissions for the node.
  $submissions = webform_get_submissions($node->nid);

  // Generate a row for each submission.
  $rowcount = 0;
  foreach ($submissions as $sid => $submission) {
    $row = ++$rowcount .",". $sid .",\"". format_date($submission->submitted, 'small') ."\",\"". $submission->remote_addr ."\",". $submission->uid .",\"". $submission->name ."\"";
    foreach ($node->webform['components'] as $cid => $component) {
      $csv_data_function   = "_webform_csv_data_". $component['type'];
      if (function_exists($csv_data_function)) {
        // Let each component add its data.
        $row .= ',"'. str_replace(array('"', '\,'), array('""', '","'), $csv_data_function($submission->data[$cid], $component)) .'"';
      }
    }
    // Write data from submissions.
    @fwrite($handle, $row ."\n");
  }
  // Close the file.
  @fclose($handle);

  drupal_set_header("Content-type: text/csv; charset=utf-8");
  drupal_set_header("Content-Disposition: attachment; filename=". preg_replace('/\.$/', '', str_replace(' ', '_', $node->title)) .".csv");

  @readfile($file_name);  // The @ makes it silent.
  @unlink($file_name);  // Clean up, the @ makes it silent.
  exit(0);
}

/**
 * Provides a simple analysis of all submissions to a webform.
 */
function webform_results_analysis($node) {
  $rows = array();
  $question_number = 0;

  $headers = array(
    t('Q'),
    array('data' => t('responses'), 'colspan' => '10')
  );

  webform_load_components(); // Load all component types.
  foreach ($node->webform['components'] as $component) {
    $question_number++;

    // Do component specific call.
    $analysis_function = "_webform_analysis_rows_". $component['type'];
    if (function_exists($analysis_function)) {
      $crows = $analysis_function($component);
      if (is_array($crows)) {
        $row[0] = array('data' => '<strong>'. $question_number .'</strong>', 'rowspan' => count($crows) + 1, 'valign' => 'top');
        $row[1] = array('data' => '<strong>'. $component['name'] .'</strong>', 'colspan' => '10');
        $rows = array_merge($rows, array_merge(array($row), $crows));
      }
    }
  }
  return theme('table', $headers, $rows);
}
