<?php
  /** Simple functions for creating html tags.
   **/

function _html_tag($name, $attributes=NULL) {
  if (is_array($attributes)) {
		$data = $attributes["data"];
		foreach ($attributes as $key => $value) {
			if ($key != "data" && $key != "checked")  {
				$attr .= " $key=\"$value\"";
	  	}
	  	else if ($key == "checked" && $value) {
				$attr .= " checked=\"checked\"";
	  	}
		}
  }
  if ($data) {
		$output = "<$name$attr>$data</$name>";
  }
  else if ( $data === 0 ) {
    $output = "<$name$attr>0</$name>";
  }
  else {
		$output = "<$name$attr />";
  }
  return $output;
}


/** general tags **/
function html_a($str, $url, $atrributes = array()) {
  $atrributes['data'] = $str;
  $atrributes['href'] = $url;
  return _html_tag('a', $atrributes);
}

function html_b($str, $attr = array()) {
  $attr['data'] = $str;
  return _html_tag("b", $attr);
}

function html_br() {
    return _html_tag("br")."\n";
}


function html_p($str, $attr = array()) {
  $attr['data'] = $str;
  return _html_tag("p", $attr)."\n";
}

function html_div($str, $attr = array()) {
	$attr['data'] = $str;
	return _html_tag('div', $attr);
}

function html_span($str, $attr = array()) {
	$attr['data'] = $str;
	return _html_tag('span', $attr);
}

/** Returns an encased tag. $output
  * eg. <tag_name>text</tag_name>
  *
  * @param $tag_name The name of the tag
  * @param $text  The bit in the middle
  * @param $attributes Array of attributes. eg <tag_name attribute_key=attribute_value>text</tag_name>
  * 
**/
function html_encase($tag_name, $text, $attributes=""){
  $attributes["data"] =  $text ; 
  $output = _html_tag($tag_name, $attributes);
  return $output;
} // End function html_encase

/** Returns an html table. $output
  *
  * @param $table_data A double array with grid data. $table_data[row][column]
  *
  * @param $table_headers An array with the Column names
  *
**/
function html_table($table_data, $table_headers=""){

  // Headers
  if (is_array($table_headers)){
    foreach ($table_headers as $thead){
      $text .= ("<th>" . $thead ."</th>");
    }
    $table_output = html_encase("tr", $text);
  }
  // Main data
  foreach ($table_data as $row){
      $text = "";
    foreach ($row as $cell){
      $text .= ("<td>" . $cell . "</td>");
    }
    $table_output .= html_encase("tr", $text);
  }
 
  $output = html_encase("table", $table_output);
  return $output;
} // End function html_table

/** Returns a unnumbered list.
 * @param $list An array of items to list.
 **/
function html_ul($items) {
  if(is_array($items)) {
    foreach ($items as $li) {
      $output .= '  '._html_tag('li', array('data' =>$li))."\n";
    }
  }
  else
    $output .= '  '._html_tag('li', array('data' =>$items))."\n";

  $output = _html_tag('ul', array('data' =>$output))."\n";
  return $output;
}
  
/** Forms **/
function html_form_textfield($description, $name, $value="", $size=10, $maxlength=50) {
    return _html_tag("input", 
		     array("data"=>$description, "type"=>"text", "name"=>$name, "value"=>$value, "size"=>$size, "maxlength"=>$maxlength));
}

function html_form_radio($description, $name, $value=1, $checked=0) {
    return _html_tag("input", array("data"=>$description, "type"=>"radio", "name"=>$name, "value"=>$value, "checked"=>$checked));
}

function html_form_checkbox($description, $name, $checked=0) {
    return _html_tag("input", array("data"=>$description, "type"=>"checkbox", "name"=>$name, "checked"=>$checked));
}

function html_form_submit($name, $description = '', $op = 'op') {
    $output .= _html_tag("input", array("type"=>"submit", "value"=>$name, 'name'=>$op));
    if(!empty($description))
      $output .= _html_tag('div', array('data'=>$description, 'class'=>'description'));
    return $output;
}

function html_form_button($name, $value, $description = '', $type='submit') {
    $output .= _html_tag("input", array("type"=>$type, "value"=>$value, 'name'=>$name));
    if(!empty($description))
      $output .= _html_tag('div', array('data'=>$description, 'class'=>'description'));
    return $output;
}

function html_form_hidden($name, $value) {
   return _html_tag('input', array('type'=>'hidden', 'name'=>$name, 'value'=>$value));
}

function html_form_select($name, $value, $options, $description = 0, $extra = 0, $multiple = 0) {
  if (count($options) > 0) {
    foreach ($options as $key=>$choice) {
      // array('value' => $key, 'selected'=$selected, 'data' => check_form($choice))
      $attr['value'] = $key;
      $attr['data'] = $choice;
      if (is_array($value) && in_array($key, $value) ) {
        $attr['selected'] = 'selected';
      }
      else if ($value == $key) {
        $attr['selected'] = 'selected';
      }
      $select .= _html_tag('option', $attr);
      unset($attr);
    }
    $output .= _html_tag('select', 
                        array('name'=>"$name".($multiple ? "[]" : ""),
                              'data' => $select));
    if(!empty($description))
      $output .= _html_tag('div', array('data'=>$description, 'class'=>'description'));
    return $output;
  }
}

function html_form_weight($name = "weight", $value = 0, $delta = 10, $description = 0, $extra = 0) {
  for ($n = (-1 * $delta); $n <= $delta; $n++) {
    $weights[$n] = "$n";
  }
  return html_form_select($name, $value, $weights, $description, $extra);
}

?>
