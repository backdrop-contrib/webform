<?php
  // $Id$

  /** 
   * @author Pontus Ullgren <ullgren@user.sourceforge.net>
   * @package module_webform
   * @copyright Pontus Ullgren 2004
   **/
function _webform_page() {
  $header = array(
                  t('Title'), 
                  array('data' => t('View'),
                        'colspan' => '3'),
                  array('data' => t('Operations'),
                        'colspan' => '3')
                  );

  $result = db_query("SELECT nid, title FROM {node} WHERE type='webform'");
   
  while ($node = db_fetch_object($result)) {
    $rows[] = array($node->title,
                    l(t('analysis'),'webform/analysis/'.$node->nid),
                    l(t('table'),'webform/table/'.$node->nid),
                    l(t('download'),'webform/table/'.$node->nid . '/download'),
                    l(t('edit'), 'node/'.$node->nid.'/edit'),
                    l(t('clear'), 'webform/clear/'.$node->nid));
  }
  $content = theme('table', $header, $rows);
  print theme('page', $content, $title);
} // end function _webform_page

function _webform_reporttable() {
  $title = t('webform submissions');

  if ( arg(2) ) {
    // Get all the submitted values for the node
    $query = 'SELECT sd.sid as sid, c.cid as cid, sd.name as name, sd.data as data'.
      ' FROM '.
      ' {webform_submitted_data} sd, {webform_component} c '.
      ' WHERE sd.nid = c.nid '.
      ' AND sd.nid = %d '.
      ' GROUP BY sd.sid, sd.name '.
      ' ORDER BY sd.sid, c.cid ';


    $res = db_query($query, arg(2));

    if(arg(3) == 'download') {

      // TODO: Download the values as a CSV file.
      /**
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
       **/
       $node_info = _webform_get_node_info(arg(2));
       $title_string = $node_info->title;

       $file_name = _webform_print_csv_string($title_string,arg(2),'file'); // This will default to printing the output which saves stack space

  	  //$header_array = array("Content-type: text/html; charset=utf-8","Content-Disposition: attachment; filename=" . $title_string);
	  //file_transfer($title_string,$header_array);

       drupal_set_header("Content-type: text/plain; charset=utf-8");
       drupal_set_header("Content-Disposition: attachment; filename=" . $title_string . ".csv");

       @readfile($file_name);
       exit(0);
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
        else if ( unserialize($field->data) ) {
          $cell[] = theme('item_list', unserialize($field->data));
        }
        else {
          $cell[] = $field->data;
        }
        $previous = $field->sid;
      }
      // and the last one ...
      $rows[] = array_merge(array($previous), $cell);

      $content = theme('table', $header, $rows);

  		//print theme('page', $content, $title);
  		print theme('page', $content, $title);
    }
  }
}

/** _webform_print_csv_string - Decomposes the database to a flat file for a given SID.
 * @param $title - Title for this report. Will be printed as a heading.
 * @param $nid - The drupal node id representing the webform data that is to be extracted.
 * @param $print_now Whether to print the result as it is formed or store it to a string and return it.
 * @param $new_line - A string to be used to render a new line.  Defaults for print on windows based systems
 * @return a reference to a string representing the file in csv format.
 **/

function _webform_print_csv_string($title_string,$nid,$mode = 'file',$new_line = "\r\n"){
  // Because the components may have changed during the lifestyle of the form
  // and because we may be using a low version of MySQL et al. The query is done in two
  // passes.  This would be more efficiently done using a VIEW or SUB-SELECT

  // The first query identifies the characteristics of components that are in the data set
  $component_query = 'SELECT wc.cid as cid, wc.name as name, wc.type as type, wc.value as value, '.
    '                        wc.extra as extra, wc.mandatory as mandatory, wc.weight as weight '.
    'FROM {webform_component} wc '.
    'WHERE wc.nid = %d '.
    'ORDER BY wc.weight, wc.name';
  $component_result = db_query($component_query, $nid);

  // First we populate it with the common record variables
  $record_array = array();

  $record_array[0]['serial'] = "Serial";
  $record_array[0]['__timestamp'] = "Unix";
  $record_array[0]['sid'] = "sid";
  $record_array[0]['__userid'] = "userid";
  $record_array[0]['__remotehost'] = "remotehost";
  $record_array[0]['__useragent'] = "useragent";
  $record_array[0]['components'] = array();
  $record_array[0]['index'] = array(); // Converts between name and index;

  $header[0] =  "$title_string,,,,"; // The inner quotes allow the title to include commas
  $header[1] .= "Submission Details,,,,";
  $header[2] .= "Serial,Timestamp,SID,User,Host";

  $index = 0; // Ensures they retain the correct order

  //and second the field structure that is unique to this node
  while ($record = db_fetch_object($component_result)) {
    // It would seem more logical to index on 'cid' rather than 'name' however
    // webform_submited_data does not store cid.
    $record_array[0]['index'][$record->name] = ++$index;
    $record_array[0]['components'][$index]['cid'] = $record->cid;
    $record_array[0]['components'][$index]['type'] = $record->type;
    $record_array[0]['components'][$index]['value'] = $record->value;

    $header[0] .= ',';
    $header[1] .= ',' . $record->name;

    if($record->type == 'select'){

      $record_array[0]['components'][$index]['options'] = unserialize($record->extra);  // This will create the sub dimensions ['multiple'] and ['items']
      $items = explode("\n", _webform_filtervalues($record_array[0]['components'][$index]['options']['items']));
      $record_array[0]['components'][$index]['options']['items'] = $items;
      
      foreach($record_array[0]['components'][$index]['options']['items'] as $key =>$item){
        $record_array[0]['components'][$index]['options']['items'][$key] = trim($item);  // Trim off any linefeeds etc. and put them back in the same place
      }
      
      $header[0] .= str_pad('', count($items) - 1,',');
      $header[1] .= str_pad('', count($items) - 1,',');
      
      foreach($items as $item){
        //print trim($item);
        $header[2] .= ',' . '"'.trim($item) . '"';	// The quotes are added so the items can contain commas.
      }
      
    } else {
      $record_array[0]['components'][$index]['data'][0] = '';
      $header[2] .= ',';
    }
    
    $record_array[0]['components'][$index]['extra'] = $record->extra;
    $record_array[0]['components'][$index]['mandatory'] = $record->mandatory;
  }
  
  // Supplement the $index_array with the field types that are not actually components but
  // that act like them.
  
  $record_array[0]['index']['__userid'] = ++$index;
  $record_array[0]['index']['__remotehost'] = ++$index;
  $record_array[0]['index']['__useragent'] = ++$index;
  $record_array[0]['index']['__timestamp'] = ++$index;
  
  $header[0] .= $new_line;
  $header[1] .= $new_line;
  $header[2] .= $new_line;
  
  $main_query = 'SELECT sd.sid as sid, sd.name as name, sd.data as data '.
    'FROM {webform_submitted_data} sd '.
    'WHERE sd.nid = %d ORDER BY sd.sid';
  
  $main_result = db_query($main_query, $nid);
  $current_sid = 0;
  $serial = 0;
  
  switch($mode){
  case 'print':
    print $header[0] . $header[1] . $header[2];
    break;
  case 'string':
    $records = $header[0] . $header[1] . $header[2];
    break;
  case 'file':
  default:
    $file_record = $header[0] . $header[1] . $header[2];
    $file_name = "./tmp/tmp_output.csv"; // Need to get drupal path to temp rather than just guess
    $handle = @fopen($file_name,"w"); // The @ suppresses errors
    @fwrite($handle,$file_record);
  }
  
  
  // Step through the database pulling out all records.  A given SID will
  // be represented by multiple rows so these are collated into a single meta record
  // given in $record_array[1][] Once this has been populated it is sent off to
  // _webform_make_record_csv_string() to turn it into a string
  
  while ($record = db_fetch_object($main_result)) {
    
    if($record->sid != $current_sid){
      $current_sid = $record->sid;
      if($serial != 0){
        switch($mode){
        case 'print':
          print _webform_make_record_csv_string($record_array,$new_line);
          break;
        case 'string':
          $records .= _webform_make_record_csv_string($record_array,$new_line);
          break;
        case 'file':
        default:
          $file_record = _webform_make_record_csv_string($record_array,$new_line);
          @fwrite($handle,$file_record);
        }
      }
      $record_array[1]['serial'] = ++$serial;
      $record_array[1]['sid'] = $current_sid = $record->sid;
      $record_array[1]['components'] = $record_array[0]['components'];	// Reset the rest of the data
    }
    //print "<p>Setting sid $current_sid :" . $record_array[$index]['sid'];
    $index = $record_array[0]['index'][$record->name];
    switch($record->name){
    case '__timestamp':
      // By using the drupal date functions we can use the users own format. However,
      // Even the 'small' date is pretty ugly for this field.
      // If we assume the 'small' format retains the '-' then we could chop it up into two fields.
      // Time and date.  That way we would retain the users date and time formating preferences but they
      // would be readable by excel
      $record_array[1][$record->name] = format_date($record->data,'small');		// Becareful, longer formats have commas in them.
      break;
    case '__userid':
    case '__remotehost':
    case '__useragent':
      $record_array[1][$record->name] = $record->data;
      break;
    default:
      switch($record_array[0]['components'][$index]['type']){
      case 'select':
        if($record_array[0]['components'][$index]['options']['multiple'] == 'Y'){
          $record_array[1]['components'][$index]['data'] = unserialize($record->data);
        } else {
          $record_array[1]['components'][$index]['data'][0] = $record->data;
        }
        break;
      case 'textarea':
      case 'hidden':
      case 'textentry':
        //print "<br>Setting $record->data on $record->name at CIX:$index";
        $record_array[1]['components'][$index]['data'][0] = $record->data;
        break;
      default:
        
      }
      //print "<p>Setting $index components $record->name :" . $record->data;
    }
  }
  // Get the last one
  if($current_sid > 0){
    $record = _webform_make_record_csv_string($record_array,$new_line);
    switch($mode){
    case 'print':
      print $record;
      break;
    case 'string':
      $records .= $record;
      break;
    case 'file':
    default:
      $file_record = $record;
      @fwrite($handle,$file_record);
    }
  }
  
  //print "Main result = " . count($record_array) . " records.";
  switch($mode){
  case 'print':
    return true;
  case 'string':
    return $records;
  case 'file':
  default:
    @fclose($handle);
    return $file_name;
  }
}

/** _webform_make_record_csv_string - Decomposes a single session ID to a flat structure.
 * @param &$record_array - A reference to the filled record array element.
 * @param $new_line - A string to be used to render a new line.
 * @return a string representing the file in csv format.
 **/
function _webform_make_record_csv_string(&$ra,$new_line){
  
  $output =  $ra[1]['serial'] . ",";
  $output .= $ra[1]['__timestamp'] . ",";
  $output .= $ra[1]['sid'] . ",";
  $output .= $ra[1]['__userid'] . ",";
  $output .= $ra[1]['__remotehost'];
  
  foreach($ra[0]['index'] as $name => $index){
    
    switch($name){
    case '__remotehost':
    case '__userid':
    case '__timestamp':
    case '__useragent':
      // Ignore the common elements
      break;
    default:
      if($ra[0]['components'][$index]['type'] == 'select'){
        foreach($ra[0]['components'][$index]['options']['items'] as $choice){
          if(is_array($ra[1]['components'][$index]['data'])){  // When all null, then not array?
            if(in_array($choice,$ra[1]['components'][$index]['data'])){ // This was throwing a warning! [Wrong datatype for second argument]
              $output .= ',YES';
            } else {
              $output .= ',no';
            }
          } else {
            $output .= ',no';
          }
        }
      } else {
        $d = $ra[1]['components'][$index]['data'][0];
        $output .= ",\"$d\""; // The internal slashed quotes allow the field to include commas
      }
    }
  }
  
  $output .= $new_line;
  return $output;
}

/**
 * Pulls the information block for a given node
 * @param $nid - The node requested
 * @return - A structure containing the node info.
 **/
function _webform_get_node_info($nid){
  $node_search_result = db_query("SELECT nid, title FROM {node} WHERE type='webform' AND nid=%d",$nid);
  $node_info = db_fetch_object($node_search_result);
  return $node_info;
}


/** _webform_reportanalysis - Provides simple analysis of a series of webform submission.
 * @return to stdio a themeatic HTML rendering of the page.
 **/

function _webform_reportanalysis() { 
  
  // The current defaults
  // TODO: Move some of these variables to the settings page.
  
  $view_settings['analog_bar']['color'] = '#0000AA';		// The color of the analog bar.
  $view_settings['analog_bar']['resolution'] = 2;				// The bar resolution in %. A bar bit will occur at every x% where x is this value.
  $view_settings['analog_bar']['bar_char'] = '&nbsp;';	// The HTML character used to form the bar.
  $view_settings['options']['default_identifier'] = '&nbsp;<font color=#FF0000>*</font>'; // The HTML stream used to flag a default value
  $view_settings['options']['highlight_color'] = '#AA0000';
  $view_settings['options']['heading_format'] = "<b>%s</b>";
  $view_settings['options']['subnumbering_format'] = "&nbsp;<i><small>%s</small></i>";
  $view_settings['options']['nonuser_option_format'] = "<i>%s</i>";
  
  // Set the filters and views switches
  
  $filters_array = array('u'=>t('Unique Users Only'),'i'=>t('Unique IPs Only'));
  $views_array = array('a'=>t('Analog Bar'),'p'=>t('Percentage'),'t'=>t('Tallies'),'c'=>t('Covariance Matrix'));
  
  //$elements = str_split($filters_and_views_switches);  Remember, Drupal does not work with php5
  
  if($_POST['op'] == 'Set Filters & View Elements'){
    $array_test = $_POST['edit'];
    if(is_array($array_test)){
      if(is_array($array_test['submission_view_elements'])){
        $views_switches = $array_test['submission_view_elements'];
      }else{
        $views_switches = array();	// User has deselected everything including defaults
      }
      if(is_array($array_test['submission_filters'])){
        $filters_switches = $array_test['submission_filters'];
      }else{
        $filters_switches = array();	// User has deselected everything including defaults
      }
    } else {
      // Invalid posted element. Not an array
    }
  } else {
    //This has not come from the "Set Filters & View Elements" button.  Set the default values
    $filters_switches = array();	// No filters currently set as default.
    $views_switches = array('t'); // Tallies.
  }
  
  $post_content  = form_checkboxes("Filters","submission_filters",$filters_switches,$filters_array,"Select the filters to apply to the set of submissions");
  $post_content .= form_checkboxes("View Elements","submission_view_elements",$views_switches,$views_array,"Select the view elements to best highlight the results");
  $post_content .= form_submit('Set Filters & View Elements');
  
  // $content .= form($post_content, 'post'); 
  // This statement which pastes of the HTML to $content now occurs later to allow the 
  // record numbers to be counted and displayed ahead of it.
  
  // The filter mechanism uses a single pass through the database to create an indexed array of
  // valid Session Ids.  This is subsequently compared against the results returned for the main query. This is an inefficient mechanism but has been chosen to allow backward capacity with
  // MYSQL Versions < 4.1
  // A better mechanism would use sub-SELECTs (MySQL v > 4.1) or CREATE VIEW (MySQL v > 5.0)
  
  // Load up the valid nodes array
  
  if(count($filters_switches)){
    
    // The filters are going to be OR'd so we must make them return empty sets when they are off
    $uniqueid_query = (in_array('u',$filters_switches)) ? "__userid" : "@@@Turn off this filter@@@"; 			// The '@@@' is to make it extrememly unlikey that this would ever be user data
    $uniqueip_query = (in_array('i',$filters_switches)) ? "__remotehost" : "@@@Turn off this filter@@@";	// The '@@@' is to make it extrememly unlikey that this would ever be user data
  } else {
    // If there are no filters then they should return "everything".
    $uniqueid_query = "%";
    $uniqueip_query = "%";
  }
  
  // Perform the search to identify valid records.
  
  // Boolean logic does not follow the associative law, hence the brackets: x AND (y OR z)
  $filtered_nodes_search_result = db_query('SELECT DISTINCT max(sd.sid) AS latest_sid '.
                                           'FROM {webform_submitted_data} sd '.
                                           "WHERE sd.nid = %d AND (sd.name LIKE '%s' OR sd.name LIKE '%s') ".
                                           'GROUP By sd.data '.
                                           'ORDER BY sd.sid DESC',
                                           arg(2), $uniqueid_query, $uniqueip_query);
  
  // Set them up in a keyed array so we can find them quickly.
  
  $valid_nodes = array();
  while ($field = db_fetch_object($filtered_nodes_search_result)) {
    $valid_nodes[$field->latest_sid] = $field->latest_sid; 
  }
  
  $valid_records = count($valid_nodes);
  if(!$valid_records) $valid_records = 1; // Should never happen, however kill any divide by zeros just in case.
  
  // Show the number of records in a friendly sentence at the top of the screen.
  $content .= "<p>There are $valid_records records selected with the current filter.<p>";
  
  $content .= form($post_content, 'post');
  $content .= "<br>"; // Some themes jam the form button too close to the next element and it looks ugly.
  
  unset($uniqueid_query,$uniqueip_query);	
  
  // Pull the title of the form from node and make a nice heading for the page
  $node_info = _webform_get_node_info(arg(2));
  $title = "$node_info->title <br>Results Analysis<p>";
  
  // Do the main query
  $query = 'SELECT sd.name as name, sd.sid as sid, c.cid as cid, c.type as type, sd.data as data, c.extra as extra, c.value as value '.
    'FROM {webform_submitted_data} sd, {webform_component} c '.
    'WHERE sd.nid = c.nid '.
    '  AND sd.name = c.name '.
    '  AND sd.nid = %d '.
    'ORDER BY c.cid, sd.data';
  $res = db_query($query, arg(2));
  
  // View the submitted entries as a list with tallies.
  $question_number = 0;
  $question_name = '';
  $new_question = true;
  
  $rows = array();
  $shadow_rows = array();	// Everything in rows[] will be displayed, so shadow_rows[] keeps things
  // that we do not want to display such as raw tallies.
  
  $header = array(t('Q'),t('choice'),array('data' => t('responses'),'colspan' => '2'));
  
  // Setup for the covariance matrix, but only if we need to
  if(in_array('c',$views_switches)){
    $covariance_records = array();	// An array of arrays of SIDs
    $cm_index = array();						// A lookup array to get index into $covariance_records
  }
  
  // This is the main loop that steps through all of the records returned by the
  // SQL query.  This is a superset of the required nodes given in $valid_nodes[]
  
  while ($field = db_fetch_object($res)) {
    
    if(!isset($valid_nodes[$field->sid])){
      // Use the array lookup on the $valid_nodes[] to be a filter.  Loop back if it fails.
      continue;
    }
    
    // It must be a valid record to get this far.
    
    if($field->name != $question_name){
      //We must have hit a new question (component)
      $question_name = $field->name;
      $question_number++;
      $new_question = true;
      $component_type = $field->type;
      
      // Perform common and psuedo-common actions for components
      
      switch($component_type){
      case 'textarea':
      case 'textfield':
      case 'select':
        //$row = array();
        $row[0] = $shadow_row[0] = $question_number;
        $row[1] = $shadow_row[1] = sprintf($view_settings['options']['heading_format'],$field->name);
        $row[2] = $shadow_row[2] = ''; 			// The question heading have nothing in the columns to their right where the
        $row[3] = "";												// data appears in other rows.  So we fill up the view switches cells with ''
        
        $rows[] = $row;
        break;
      }
    }
    
    // Perform specific actions for components.
    // This should be a switch statement but it looks too ugly.
    
    if($component_type == 'textarea' || $component_type == 'textfield'){
      
      if($new_question){
        
        $new_question = false;  // Only one pass here for each question     
        
        // A textarea object does not have subquestions like checkboxes or radio buttons, rather it has nominated analysis characteristics
        // as given in the $choices[].
        
        $choices = array('blank' => t('Left Blank'),'default' => t('Default'), 'value' => t('User entered value'),'average' => t('Average submission length in words (ex blanks)'));
        
        foreach ($choices as $subkey => $choice) {
          $stripped_choice = trim($choice);     // Have to pull off the line-feeds etc.. Should not be any as we created the array just above by hand.
          
          if($subkey == 'default'){
            if(!trim($field->value)){
              // If there is no user default value set, then we don't need a row for it because it will be the same as blank.
              continue;
            }
          }
          
          $skey = "$question_name"."$question_number";	// The question number is included in case there are ever more than one textarea with the same name
          $rkey = "$skey"."$subkey";
          
          $row = array();
          $shadow_row = array();																	// This is the non-display shadow that persists data for calculations
          $row[0] = $shadow_row[0] = '';													// No numbering on options of textareas
          $row[1] = $shadow_row[1] = sprintf($view_settings['options']['nonuser_option_format'],$stripped_choice); 	// The results title, highlighted to differentiate it from the user entered questions of other component types.
          $row[2] = $shadow_row[2] = '';													// The tallies will go here
          $row[3] = '';																						// Any analog bar or percentages will go here
		
          $shadow_row[$subkey] = 0;
          
          $rows[$rkey] = $row;
          $shadow_rows[$skey] = $shadow_row;
		
          unset($row,$rkey,$shadow_row);
        }
      }
      
      // The possible choices for a textarea/textfield are now stored and this may be a first or subsequent call
      reset($rows);
      
      if(!isset($shadow_rows[$skey]['word_count'])) $shadow_rows['word_count'] = 0;
      if(!isset($shadow_rows[$skey]['average'])) $shadow_rows['average'] = 0;
      
      if(trim($field->data)){
        if(trim($field->data != $field->value)){
          // Then it is not empty or the same as the default
          $shadow_rows[$skey]['value']++;
        } else {
          $shadow_rows[$skey]['default']++;
        }
        $shadow_rows[$skey]['word_count'] += str_word_count($field->data);
        //$shadow_rows[$skey]['average'] = round($shadow_rows[$key]['word_count'] / $valid_records);
      } else {
        $shadow_rows[$skey]['blank']++;
      }
      
      //unset($skey);
      
      foreach($choices as $subkey => $choice){
        
        $stripped_choice = trim($choice);     // Have to pull off the line-feeds etc.. Should not be any as we created the array just above by hand.
        
        $skey = "$question_name"."$question_number";	// The question number is included in case there are ever more than one textarea with the same name
        $rkey = "$skey"."$subkey";
        
        if(isset($rows[$rkey])){
          // Check it is set because some may not exist by design, ie when the default is blank
          switch($subkey){
          case 'average':
            $av = round($shadow_rows[$skey]['word_count'] / ($valid_records - $shadow_rows[$skey]['blank']),1);	// 1 decimal place
            $rows[$rkey][2] = _build_tally(&$views_switches,&$view_settings,$av);
            $rows[$rkey][3] = '';		// No percentages or bars on average wordcounts
            break;
          default:
            $rows[$rkey][2] = _build_tally(&$views_switches,&$view_settings,$shadow_rows[$skey][$subkey]);
            $rows[$rkey][3] = _build_percentages(&$views_switches,&$view_settings,$shadow_rows[$skey][$subkey],$valid_records);
            break;
          }
        }
      }
      //unset($skey,$rkey,$subkey);
    }
    
    if($component_type == 'select'){
      
      if($new_question){
        $new_question = false;  // Only one pass here for each question
        
        // select components need to have their options pulled out
        // from the extra field
        
        $extra_data = unserialize($field->extra);
        
        if(is_array($extra_data)){
          
          if($extra_data['multiple'] == 'Y'){
            $multiple_choice = true;		// ie Checkboxes
          } else {
            $multiple_choice = false;		// ie Radio Buttons
          }
          
          $choices = explode("\n", _webform_filtervalues($extra_data['items']));
          
          if(is_array($choices)){
            foreach ($choices as $choice) {
              
              $stripped_choice = trim($choice);     // Have to pull off the line-feeds etc..
              $key = "$question_name"."$stripped_choice";	
              
              $row = array();
              $shadow_row = array();	// This is the non-display shadow that persists data for calculations
              $row[0] = $shadow_row[0] = _build_numbering($views_switches,$view_settings,$question_number);
              //$default_label = ($stripped_choice == trim($field->value)) ? '<span style="color: = ' . $view_settings['options']['highlight_color'] . '"> ' . $view_settings['options']['default_identifier'] . '</span>':'';
              $default_label = ($stripped_choice == trim($field->value)) ? '<span style="color: = red "> ' . $view_settings['options']['default_identifier'] . '</span>':'';
              $row[1] = $shadow_row[1] = $stripped_choice . $default_label; 	// The option title
              $row[2] = '';
              $row[3] = '';
              
              $shadow_row['raw_tally'] = 0;	// Raw tallies are always kept
              
              $rows[$key] = $row;
              $shadow_rows[$key] = $shadow_row;
              
              // Populate the Covariance Matrix index lookup array if required
              if(in_array('c',$views_switches)) $cm_index[$key] = $row[0];
              
              unset($row,$key,$shadow_row);
            }
          }    
        }
      }
      
      // The choices are now stored in $question_rows and this may be a first or subsequent call
      reset($rows);
      
      if($multiple_choice){
        
        // Handling multiple choice checkboxes
        $multiple_data = unserialize($field->data);
        if(is_array($multiple_data)){
          if(count($multiple_data)){  // Stops a zero length array hitting the foreach
            foreach($multiple_data as $check){
              $key = "$question_name".trim($check);
              
              $shadow_rows[$key]['raw_tally']++;
              
              if(in_array('c',$views_switches)){
                $covariance_records[$cm_index[$key]][] = $field->sid;
                $covariance_records[$field->sid][$cm_index[$key]] = true;
                //print "Count -> " . count($covariance_records[$cm_index[$key]]);
              }
              
              $rows[$key][2] = _build_tally($views_switches,$view_settings,$shadow_rows[$key]['raw_tally']);
              $rows[$key][3] = _build_percentages($views_switches,$view_settings,$shadow_rows[$key]['raw_tally'],$valid_records);
            }
          } else {
            // If no checkboxes are selected, then you would expect a zero length array
            // which would end up in here.  It does not because the data contains a simple 0 rather 
            // than a zero length array!
          }
        } else {
          // No checkboxes selected results in a 0 rather than a zero length array.  The
          // is_array() conditional therefore drops the condition here
          //print ("It was not an array at all, it was $field->data and there were $field->tally of them");
        }
      } else {
        
        // Handling normal radio buttons
        $key = "$question_name".trim($field->data);
        $shadow_rows[$key]['raw_tally']++;
        
        if(in_array('c',$views_switches)){
          $covariance_records[$cm_index[$key]][] = $field->sid;
          $covariance_records[$field->sid][$cm_index[$key]] = true;
          //print("<br>--> $field->sid , $cm_index[$key]");
        }
        
        $rows[$key][2] = _build_tally($views_switches,$view_settings,$shadow_rows[$key]['raw_tally']);
        $rows[$key][3] = _build_percentages($views_switches,$view_settings,$shadow_rows[$key]['raw_tally'],$valid_records);
        //$pss = '<div class="bar"><div style="width: '. $ps .'%;" class="foreground"></div>'; // This is the "correct" way, but I couldn't get it to work easily.
        
      }
      end($rows);
      unset($key);
    }
    if (($previous != -1) && ($previous != $field->sid)) {
      //$rows[] = array_merge(array($previous), $cell);
      //unset($cell);
      //$first = false;
    }
    
    if ( $field->name == '__timestamp' ) {
      //$cell[] = format_date($field->data, 'small');
    } else if ( unserialize($field->data) ) {
      //$cell[] = theme('item_list', unserialize($field->data));
    } else {
      //$cell[] = $field->data;
    }
    //$previous = $field->sid;
    
  } //end main loop
  
  // Display the Results Table
  $box_content = "<br>";
  $box_content .= theme('table', $header, $rows);
  
  // Add legend elements below the table
  $box_content .= "<p><br>";
  $box_content .= $view_settings['options']['default_identifier'];
  $box_content .= " = Default Value <br>";
  
  $content .= theme_box("Results Table", $box_content);
  
  // Construct the covariance matrix if requested
  
  if(in_array('c',$views_switches)){
    
    $box_content = "<br>";	// The covariance matrix is themed as a box
    
    $cm_rows = array();
    $cm_header = array();
    
    $cm_row[] = '&nbsp;';			// The corner square!
    $build_header = true;
    
    foreach($shadow_rows as $sr){
      if($sr[0]){
        $cm_row = array();
        
        if($build_header){
          $cm_header[] = "&nbsp;";
        }
        
        $cm_row[0] = "<b>$sr[0]</b>";
        $row_tally = $sr['raw_tally'];
        $max_registered = 0;
        
        foreach($shadow_rows as $sc){
          if($sc[0]){
            if($build_header){
              $cm_header[] = $sc[0];
            }
            $positive_covariance = 0;
            //$negative_covariance = 0;
            foreach($covariance_records[$sc[0]] as $sid){
              if($covariance_records[$sid][$sr[0]]){
                $positive_covariance++;
                if($positive_covariance > $max_registered){
                  $max_registered = $positive_covariance;	 // Find maximum for normalization
                }
              }
            }
            $cm_row[$sc[0]] = $positive_covariance;
          }
        }
        
        $build_header = false;
        
        // Go back and set the normalized values

        //$first = true;
        foreach($shadow_rows as $sc){
          if($sc[0]){
            $pc = $cm_row[$sc[0]];
            $av = round(255 * $pc / $max_registered,1);
            if($max_registered){
              $cm_row[$sc[0]] = '<span style="background-color:' . sprintf("#%'02x%'02x%'02x",$av,$av,$av) . '">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span>';
            }else{
              $cm_row[$sc[0]] = 'N';
            }
          }
        }
        $cm_rows[] = $cm_row;
        unset($cm_row);
      }
    }
    
    $box_content .= theme('table', $cm_header, $cm_rows);
    $content .= theme_box("Covariance Matrix", $box_content);
    
  }
  
  print theme('page', $content, $title);
}

/** _build_tally - Creates an HTML string sufficient to render the current tally as
 * appropriate to the current $views_switches[].
 * @param &$views_switches A reference to an array of current views_switches which may toggle elements of the display.
 * @param &$view_settings  A reference to an array of current view_settings which may change the apprearance of elements.
 * @param $t An integer representing the current tally
 * @return an HTML string representing the tally.
 **/

function _build_tally(&$views_switches,&$view_settings,$t){
  // $view_settings[] is not currently used here.  It may later.
  $output_string = in_array('t',$views_switches) ? "$t":"";
  return $output_string;
}

/** _build_numbering - Creates an HTML string sufficient to render the question number as
 * appropriate to the current $views_switches[]. The function creates the number internally by incrementing
 * static variables.
 * @param &$views_switches A reference to an array of current views_switches which may toggle elements of the display.
 * @param &$view_settings  A reference to an array of current view_settings which may change the apprearance of elements.
 * @param $qn An integer representing the current question number
 * @return an HTML string representing the question number.
 **/

function _build_numbering(&$views_switches,&$view_settings,$qn){
  
  static $last_question = -1;
  static $last_subnumber = -1;
  
  if (!in_array('c',$views_switches)) return "";
  
  if($qn != $last_question){
    $last_subnumber = 1;
    $last_question = $qn;
  } else {
    $last_subnumber++;
  }
  
  $number_string  = "$last_question.$last_subnumber";																// Simple #.# format
  $output_string .= sprintf($view_settings['options']['subnumbering_format'],$number_string);
  
  return $output_string;
}

/** _build_percentages - Creates an HTML string sufficient to render an analog bar and percentage label as
 * appropriate to the current $views_switches[].
 * @param &$views_switches A reference to an array of current views_switches which may toggle elements of the display.
 * @param &$view_settings  A reference to an array of current view_settings which may change the apprearance of elements.
 * @param $fraction An integer representing an ordinal number of elements comprising all or part of the total.  0 <= $fraction <= $total
 * @param $total A integer representing the total number of elements in the population.
 * @return an HTML string representing the bar.
 **/

function _build_percentages(&$views_switches,&$view_settings,$fraction,$total){
  
  $output_string = "&nbsp;";
  $calculated_percentage = false;	// The conditional is a tad quicker than doing the maths multiple times
  
  // First some range checking for safety.
  if($fraction < 0) $fraction = 0;
  if($fraction > $total) $fraction = $total;
  
  if(in_array('a',$views_switches)){
    $ps = round($fraction * 100 / $total);	// 100 to make it a percentage
    $calculated_percentage = true;
    $output_string .= "&nbsp";							// We tend to pad with &nbsp; to stop it ever wrapping
    $output_string .= _build_percentage_bar($view_settings,$ps);
    $output_string . "&nbsp";								// We tend to pad with &nbsp; to stop it ever wrapping
  }
  if(in_array('p',$views_switches)){
    if(!$calculated_percentage) $ps = round($fraction * 100 / $total);	// 100 to make it a percentage
    $output_string .= "&nbsp;";							// We tend to pad with &nbsp; to stop it ever wrapping
    $output_string .= sprintf("%d%%",$ps);
  }
  
  return $output_string;
}

/** _build_percentage_bar - Creates an analog bar of length k, k:0 <= k <= 100.  The bar is rendered in HTML using <span></span> tags
 * using attributes such as color and character type as provided in $view_settings[].  The output string is not padded and will
 * vary in length with the percentage rendered.  This is by design as it allows values to be attached flush to the end of the bar.
 * @param &$vs A reference to an array of current view_settings.
 * $param $ps The percentage that is to be rendered as a bar.  This number is assumed to be a percentage.
 * @return an HTML string representing the bar.  The string is not padded and will vary in length with the percentage rendered.
 **/
function _build_percentage_bar(&$vs,$ps = 0){

  // First some range checking for safety.
  if($ps < 0) $ps = 0;
  if($ps > 100) $ps = 100;
  
  // Calculate the bar length in HTML characters as required to render it.
  // The bar will appear shorter than this.  For example &nbsp; is 6 characters long but renders as 1 character.
  $pad = round($ps / $vs['analog_bar']['resolution']) * strlen($vs['analog_bar']['bar_char']) ; // We multiply by strlen because we use str_pad() to build the bar and thus we need it to be a multiple of whatever character we use ie: &nbsp; = 6 chars
  $bar_string = '<span style="background-color:' . $vs['analog_bar']['color'] . '">' . str_pad('',$pad,$vs['analog_bar']['bar_char']) . '</span>';  // Ugly non-robust way of drawing a bar.	return $pad;
  return $bar_string;
}

/**
 * Encode a string according to RFC 1522 for use in headers if it
 * contains 8-bit characters or anything that looks like it should
 * be encoded.
 * Nicked from the SquirrelMail project.
 * @param $string
 */
function _webform_encodeHeader ($string) {
  $charset = 'utf-8';
  
  if (strtolower($charset) == 'iso-8859-1') {
    $string = str_replace("\240",' ',$string);
  }
  
  // Encode only if the string contains 8-bit characters or =?
  $j = strlen($string);
  $max_l = 75 - strlen($charset) - 7;
  $aRet = array();
  $ret = '';
  $iEncStart = $enc_init = false;
  $cur_l = $iOffset = 0;
  for($i = 0; $i < $j; ++$i) {
    switch($string{$i})
      {
      case '=':
      case '<':
      case '>':
      case ',':
      case '?':
      case '_':
	if ($iEncStart === false) {
	  $iEncStart = $i;
	}
	$cur_l+=3;
	if ($cur_l > ($max_l-2)) {
	  /* if there is an stringpart that doesn't need encoding, add it */
	  $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
	  $aRet[] = "=?$charset?Q?$ret?=";
	  $iOffset = $i;
	  $cur_l = 0;
	  $ret = '';
	  $iEncStart = false;
	} else {
	  $ret .= sprintf("=%02X",ord($string{$i}));
	}
	break;
      case '(':
      case ')':
	if ($iEncStart !== false) {
	  $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
	  $aRet[] = "=?$charset?Q?$ret?=";
	  $iOffset = $i;
	  $cur_l = 0;
	  $ret = '';
	  $iEncStart = false;
	}
	break;
      case ' ':
	if ($iEncStart !== false) {
	  $cur_l++;
	  if ($cur_l > $max_l) {
	    $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
	    $aRet[] = "=?$charset?Q?$ret?=";
	    $iOffset = $i;
	    $cur_l = 0;
	    $ret = '';
	    $iEncStart = false;
	  } else {
	    $ret .= '_';
	  }
	}
	break;
      default:
	$k = ord($string{$i});
	if ($k > 126) {
	  if ($iEncStart === false) {
	    // do not start encoding in the middle of a string, also take the rest of the word.
	    $sLeadString = substr($string,0,$i);
	    $aLeadString = explode(' ',$sLeadString);
	    $sToBeEncoded = array_pop($aLeadString);
	    $iEncStart = $i - strlen($sToBeEncoded);
	    $ret .= $sToBeEncoded;
	    $cur_l += strlen($sToBeEncoded);
	  }
	  $cur_l += 3;
	  /* first we add the encoded string that reached it's max size */
	  if ($cur_l > ($max_l-2)) {
	    $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
	    $aRet[] = "=?$charset?Q?$ret?= "; /* the next part is also encoded => separate by space */
	    $cur_l = 3;
	    $ret = '';
	    $iOffset = $i;
	    $iEncStart = $i;
	  }
	  $enc_init = true;
	  $ret .= sprintf("=%02X", $k);
	} else {
	  if ($iEncStart !== false) {
	    $cur_l++;
	    if ($cur_l > $max_l) {
	      $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
	      $aRet[] = "=?$charset?Q?$ret?=";
	      $iEncStart = false;
	      $iOffset = $i;
	      $cur_l = 0;
	      $ret = '';
	    } else {
	      $ret .= $string{$i};
	    }
	  }
	}
	break;
      }
  }
  
  if ($enc_init) {
    if ($iEncStart !== false) {
      $aRet[] = substr($string,$iOffset,$iEncStart-$iOffset);
      $aRet[] = "=?$charset?Q?$ret?=";
    } else {
      $aRet[] = substr($string,$iOffset);
    }
    $string = implode('',$aRet);
  }
  return $string;
}

/** 
 * Create quoted_printable text.
 * Nicked from the PHP manual.
 **/
function _webform_qp_enc($input, $line_max = 79) {

  $hex = array('0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F');
  $lines = preg_split("/(?:\r\n|\r|\n)/", $input);
  $eol = "\r\n";
  $escape = "=";
  $output = "";
  
  while( list(, $line) = each($lines) ) {
    //$line = rtrim($line); // remove trailing white space -> no =20\r\n necessary
    $linlen = strlen($line);
    $newline = "";
    for($i = 0; $i < $linlen; $i++) {
      $c = substr($line, $i, 1);
      $dec = ord($c);
      if ( ($dec == 32) && ($i == ($linlen - 1)) ) { // convert space at eol only
	$c = "=20";
      } elseif ( ($dec == 61) || ($dec < 32 ) || ($dec > 126) ) { // always encode "\t", which is *not* required
	$h2 = floor($dec/16); $h1 = floor($dec%16);
	$c = $escape.$hex["$h2"].$hex["$h1"];
      }
      if ( (strlen($newline) + strlen($c)) >= $line_max ) { // CRLF is not counted
	$output .= $newline.$escape.$eol; // soft line break; " =\r\n" is okay
	$newline = "";
      }
      $newline .= $c;
    } // end of for
    $output .= $newline.$eol;
  }
  return trim($output);  
}
?>
