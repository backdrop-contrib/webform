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
                    l(t('submissions'),'webform//'.$node->nid),
                    l(t('analysis'),'webform/analysis/'.$node->nid),
                    l(t('table'),'webform/table/'.$node->nid),
                    l(t('download'),'webform/download/'.$node->nid),
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
      ' webform_submitted_data sd, webform_component c '.
      ' WHERE sd.nid = c.nid '.
      ' AND sd.nid = %d '.
      ' GROUP BY sd.sid, sd.name '.
      ' ORDER BY sd.sid, c.cid ';

    $res = db_query($query, arg(2));

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
    }
  }
  print theme('page', $content, $title);
}

function _webform_reportanalysis() { 

	// Set the filters and views switches
	
	$filters_array = array('u'=>t('Unique Users Only'),'i'=>t('Unique IPs Only'));
	$views_array = array('a'=>t('Analog Bar'),'p'=>t('Percentage'),'t'=>t('Tallies'));
	

	//$elements = str_split($filters_and_views_switches);  Drupal does not work with php5
  	
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

/*
The filter mechanism uses a single pass through the database to create an indexed array of
valid Session Ids.  This is subsequently compared against the results returned for the main query. This is an inefficient mechanism but has been chosen to allow backward capacity with
MYSQL Versions < 4.1
	
A better mechanism would use sub-SELECTs (MySQL v > 4.1) or CREATE VIEW (MySQL v > 5.0)
*/

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
	$filtered_nodes_search_result = db_query("SELECT DISTINCT max(sd.sid) AS latest_sid FROM `webform_submitted_data` sd WHERE sd.nid = %d AND sd.name LIKE '%s' OR sd.name LIKE '%s' GROUP By sd.data ORDER BY sd.sid DESC",arg(2),$uniqueid_query,$uniqueip_query);

	while ($field = db_fetch_object($filtered_nodes_search_result)) {
		$valid_nodes[$field->latest_sid] = $field->latest_sid; 
	}

	$valid_records = count($valid_nodes);
	$content .= "<p>There are $valid_records records selected with the current filter.<p>";
	$content .= form($post_content, 'post');
	
	unset($uniqueid_query,$uniqueip_query);	
			
  // Pull the title of the form from node                
  $node_search_result = db_query("SELECT nid, title FROM {node} WHERE type='webform' AND nid=%d",arg(2));
	$node_info = db_fetch_object($node_search_result);

	// Make a nice heading for the page
	$title = "$node_info->title | Results Analysis";

	// Do the main query
  $query = 'SELECT sd.name as name, sd.sid as sid, c.cid as cid, c.type as type, sd.data as data, c.extra as extra FROM webform_submitted_data sd, webform_component c WHERE sd.nid = c.nid  AND sd.name = c.name AND sd.nid = %d ORDER BY c.cid, sd.data';
    $res = db_query($query, arg(2));

	// View the submitted entries as a list with tallies.

    $question_number = 0;
    $question_name = '';
    $new_question = true;
            
    $rows = array();
	$shadow_rows = array();	// Everything in rows[] will be displayed, so shadow_rows[] keeps things
				// that we do not want to display such as raw tallies.
    
    $header = array(t('Q'),t('choice'),t('responses'));

    while ($field = db_fetch_object($res)) {

		if(!isset($valid_nodes[$field->sid])){
			continue;
		}
		// It must be a valid record to get this far.
      if($field->name != $question_name){
        //We must have hit a new question (component)
            $question_name = $field->name;
            $question_number++;
            $new_question = true;
            $component_type = $field->type;
        }
        
        if($component_type == 'select'){
            
            if($new_question){
          $new_question = false;  // Only one pass here for each question     

                // select components need to have their options pulled out
                // from the extra field

				$row = array();
				$row[] = $question_number;
				$row[] = "<b>$field->name </b>";
				
				// The question heading have nothing in the columns to their right where the
				// data appears in other rows.  So we fill up the view switches cells with ''

				foreach($views_switches as $vs) $row[] = '';
				
				$rows[] = $row;
                       
          // Pull the choices out of the extra field.          
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
		$row[] = $shadow_row[] = '';	// No numbering on options (yet)
		$row[] = $shadow_row[] = $stripped_choice; 	// The option title

		foreach($views_switches as $vs){
		  switch($vs){
                    case 't':
			$row['t'] = 0;
			break;
		    case 'a':
			$row['a'] = '';
        		break;
                    case 'p':
			$row['p'] = "0%";
                        break;
		    default:
			// Error as all switches should be covered.
		  } 
		}
		$shadow_row['raw_tally'] = 0;	// Raw tallies are always kept

                if(!isset($rows[$key])){
                  $rows[$key] = $row;
                  $shadow_rows[$key] = $shadow_row;
                }
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
		   $ps = round($shadow_rows[$key]['raw_tally'] * 100 / $valid_records);	// The magic numbers here are "5" the resolution, ie 5% = 1 bar bit.
                   $pad = round($ps / 5) * 6 ; // // The magic numbers here are "5" the resolution, ie 5% = 1 bar bit and "6" which is the length of the &nbsp; so we get an even multiple of it.

                   if(isset($rows[$key]['t'])) $rows[$key]['t'] = $shadow_rows[$key]['raw_tally']; 
                   if(isset($rows[$key]['a'])) $rows[$key]['a'] = '<span style="background-color: #0000AA">' . str_pad('',$pad,'&nbsp;') . '</span>';  // Ugly non-robust way of drawing a bar. Color is yet another magic number!
                   if(isset($rows[$key]['p'])) $rows[$key]['p'] = $ps ."%"; 
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
           $ps = round($shadow_rows[$key]['raw_tally'] * 100 / $valid_records);	// The magic numbers here are "5" the resolution, ie 5% = 1 bar bit.
           $pad = round($ps / 5) * 6 ; // // The magic numbers here are "5" the resolution, ie 5% = 1 bar bit and "6" which is the length of the &nbsp; so we get an even multiple of it.
				//$pss = '<div class="bar"><div style="width: '. $ps .'%;" class="foreground"></div>'; // This is the "correct" way, but I couldn't get it to work easily.
				
				if(isset($rows[$key]['t'])) $rows[$key]['t'] = $shadow_rows[$key]['raw_tally'] ;	// There can be only one  
				if(isset($rows[$key]['a'])) $rows[$key]['a'] = '<span style="background-color: #0000AA">' . str_pad('',$pad,'&nbsp;') . '</span>';  // Ugly non-robust way of drawing a bar. Color is another magic number!
				if(isset($rows[$key]['p'])) $rows[$key]['p'] = $ps ."%";
				 
          }
        end($rows);
        unset($key);
        }
      if (($previous != -1) && ($previous != $field->sid)) {
        //$rows[] = array_merge(array($previous), $cell);
        //unset($cell);
        //$first = false;
      }

      if($first) {
          //$header[] = $field->name;
      }
        
      if ( $field->name == '__timestamp' ) {
          //$cell[] = format_date($field->data, 'small');
      } else if ( unserialize($field->data) ) {
          //$cell[] = theme('item_list', unserialize($field->data));
		} else {
          //$cell[] = $field->data;
      }
        //$previous = $field->sid;
    }
      // and the last one ...
      //$rows[] = array_merge(array($previous), $cell);

	$content .= theme('table', $header, $rows);
    
  print theme('page', $content, $title);
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
