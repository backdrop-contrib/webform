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
                        'colspan' => '2')
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
  $title = t('webform analysis'); 

  if ( arg(2) ) { 
    // Get all the submitted values for the node
    $query = 'SELECT sd.name as name, c.cid as cid, c.type as type, sd.data as data, c.extra as extra, count(data) as tally FROM webform_submitted_data sd JOIN  webform_component c WHERE sd.nid = c.nid  AND sd.name = c.name AND sd.nid = %d GROUP BY sd.data  ORDER BY c.cid, sd.data';

    $res = db_query($query, arg(2));

    // View the submitted entries as a list.

    //$new_question = true;
    $question_number = 0;
    $question_name = '';
    $new_question = true;
            
    $rows = array();
    
    $header = array(t('Q'),t('choice'),t('responses'));

    while ($field = db_fetch_object($res)) {

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

          $rows[] = array($question_number,"<b>$field->name </b>",'');
                       
          // Pull the choices out of the extra field.          
          $extra_data = unserialize($field->extra);
          if(is_array($extra_data)){
              if($extra_data['multiple'] == 'Y'){
                $multiple_choice = true;    
              } else {
                  $multiple_choice = false;
            }
            $choices = explode("\n", _webform_filtervalues($extra_data['items']));
            if(is_array($choices)){
              foreach ($choices as $choice) {
                  $stripped_choice = trim($choice);     // Have to pull off the line-feeds etc..
                  $key = "$question_name"."$stripped_choice";
                  $row = array('',$stripped_choice,0);  // Tally is set to zero and keyed to choice
                  if(!isset($rows[$key])){
                  $rows[$key] = $row;
                }
                unset($row,$key);
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
              if(count($multiple_data)){
              foreach($multiple_data as $check){
                   $key = "$question_name".trim($check);
                   if(isset($rows[$key][2])){
                         $rows[$key][2] += $field->tally;  // The SQL query is GROUPing on the data, hence we have to
                                                           // use the $field->tally in case there are multiple
                                                           // identical submissions.
                }
              }
            }
                  }
            } else {
                // Handling normal radio buttons
              $key = "$question_name".trim($field->data);
              if(isset($rows[$key][2])){
                 $rows[$key][2] = $field->tally;
              }
          }
        end($rows);
        unset($key);
        end($rows);         
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
      }
      else {
          //$cell[] = $field->data;
      }
        //$previous = $field->sid;
    }
      // and the last one ...
      //$rows[] = array_merge(array($previous), $cell);

    $content = theme('table', $header, $rows);
    
  }
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
