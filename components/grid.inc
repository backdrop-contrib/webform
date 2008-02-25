<?php
// $Id$
/**
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @return
 *   An array of form items to be displayed on the edit component page
 **/
function _webform_edit_grid($currfield) {
  $edit_fields = array();
  $edit_fields['extra']['options'] = array(
    '#type' => 'textarea',
    '#title' => t("Options"),
    '#default_value' => $currfield['extra']['options'],
    '#description' => t('Options to select across the top. One option per line. Key-value pairs may be entered seperated by pipes. i.e. safe_key|Some readable option') .'<br />'. webform_help('webform/helptext#variables'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -3,
    '#required' => TRUE,
  );
  $edit_fields['extra']['questions'] = array(
    '#type' => 'textarea',
    '#title' => t("Questions"),
    '#default_value' => $currfield['extra']['questions'],
    '#description' => t('Questions list down the left side. One question per line.') .'<br />'. webform_help('webform/helptext#variables'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -2,
    '#required' => TRUE,
  );
  $edit_fields['extra']['optrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Options'),
    '#default_value' => $currfield['extra']['optrand'],
    '#description' => t('Randomizes the order of options on the top when they are displayed in the form.'),
  );
  $edit_fields['extra']['qrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Questions'),
    '#default_value' => $currfield['extra']['qrand'],
    '#description' => t('Randomize the order of the questions on the side when they are displayed in the form.'),
  );
  return $edit_fields;
}

function _webform_edit_validate_grid($form_values) {
  $rows = explode("\n", _webform_filtervalues($form_values['extra']['options'], NULL, NULL, FALSE));
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^(.+)?\|(.*)$/', $row, $matches)) {
      if (preg_match('/ |"/', $matches[1])) {
        form_set_error('field][extra][options', t('The options for this grid contain illegal characters (quotes or spaces). Specify your options as <em>safe_value_no_spaces</em>|<em>The Real Value</em>.'));
        return FALSE;
      }
    }
  }
  $rows = explode("\n", _webform_filtervalues($form_values['extra']['questions'], NULL, NULL, FALSE));
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^(.+)?\|(.*)$/', $row, $matches)) {
      if (preg_match('/ |"/', $matches[1])) {
        form_set_error('field][extra][questions', t('The questions for this grid contain illegal characters (quotes or spaces).'));
        return FALSE;
      }
    }
  }
  return true;
}

function _webform_render_grid($component, $random = true) {
  $form_item = array(
    '#title' => $component['name'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_grid',
    '#description'   => _webform_filtervalues($component['extra']['description']),
  );

  $questions = explode("\n", _webform_filtervalues($component['extra']['questions'], NULL, NULL, FALSE));
  $rows = explode("\n", _webform_filtervalues($component['extra']['options'], NULL, NULL, FALSE));

  if ($component['extra']['optrand'] && $random) {
    shuffle($rows);
  }
  foreach ($rows as $row) {
    if ($row != '') {
      $row = trim($row);
      if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
        $options[$matches[1]] = $matches[2];
      }
      else {
        $options[_webform_safe_name($row)] = $row;
      } 
    }
  }
  $cid = 0;
  if ($component['extra']['qrand'] && $random) {
    shuffle($questions);
  }
  foreach ($questions as $question) {
    if ($question != '') {
      $form_item[_webform_safe_name($question)] = array(
        '#title'         => $question,
        '#required'      => $component['mandatory'],
        '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
        '#suffix'        => '</div>',
        '#options'       => $options,
        '#type'          => 'radios',
      );
    }
  }

  return $form_item;
}

/** 
 * Display the result of a grid submission. The output of this function will be
 * displayed under the "results" tab then "submissions"
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema
 * @return
 *   Textual output formatted for human reading.
 **/
function _webform_submission_display_grid($data, $component, $enabled = FALSE) {
  $form_item = _webform_render_grid($component, FALSE);
  $cid = 0;
  foreach (element_children($form_item) as $key) {
    $value = $data['value'][$cid++];
    if ($value !== '0') {     
      if (array_key_exists(_webform_safe_name($value), $form_item[$key]['#options'])) {
        $form_item[$key]['#default_value'] = _webform_safe_name($value);
      }
      else {
        $form_item[$key]['#default_value'] = $value;
      }
    }
    $form_item[$key]['#disabled'] = !$enabled;
  }
  return $form_item;
}

/** 
 * Translates the submitted 'safe' form values back into their un-edited
 * original form.
 * 
 * @param $data
 *   The POST data associated with the component
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema
 * @return
 *   Nothing
 **/
function _webform_submit_grid(&$data, $component) {
  $value = _webform_filtervalues($component['value']);
  $rows = explode("\n", _webform_filtervalues($component['extra']['options']));
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[1];
    }
    else {
      $options[_webform_safe_name($row)] = $row;
    } 
  }
  if (is_array($data)) {
    foreach ($data as $key => $value) {
      if ($value) {
        $data[$key] = $options[$value];
      }
    }
  }
  elseif (!empty($data)) {
    $data = $options[$data];
  }
}
/**
 * Format the output of emailed data for this component.
 *
 * @param $data
 *   A string or array of the submitted data.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_grid($data, $component) {
  $questions = explode("\n", _webform_filtervalues($component['extra']['questions']));
  $output = $component['name'] .":\n";
  foreach ($questions as $key => $question) {
    $output .= '    - '. trim($question) .': '. $data[_webform_safe_name($question)] ."\n";
  }
  return $output;
}

/** 
 * function _webform_help_select
 * Module specific instance of hook_help
 **/
function _webform_help_grid($section) {
  switch ($section) {
    case 'admin/settings/webform#grid_description':
      $output = t("Allows creation of grid questions, denoted by radio buttons.");
      break;
  }
  return $output;
}

/**
 * Module specific instance of hook_theme().
 */
function _webform_theme_grid() {
  return array(
    'webform_grid' => array(
      'arguments' => array('grid_element' => NULL),
    ),
    'webform_mail_grid' => array(
      'arguments' => array('data' => NULL, 'component' => NULL),
    ),
  );
}

/** 
 * Calculate and returns statistics about results for this component from all
 * submission to this webform. The output of this function will be displayed
 * under the "results" tab then "analysis".
 * 
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of data rows, each containing a statistic for this component's
 *   submissions.
 **/
function _webform_analysis_rows_grid($component) {
  // Generate the list of options.
  $rows = explode("\n", _webform_filtervalues($component['extra']['options']));
  $options = array();
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[$row] = $row;
    } 
  }

  // Generate the list of questions.
  $rows = explode("\n", _webform_filtervalues($component['extra']['questions']));
  $questions = array();
  $cid = 0;
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $questions[$matches[1]] = $matches[2];
    }
    else {
      $questions[$cid++] = $row;
    } 
  }

  // Generate a lookup table of results.
  $query = 'SELECT no, data, count(data) as datacount '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '.
    " AND data != '0' AND data != '' ".
    ' GROUP BY no, data';
  $result = db_query($query, $component['nid'], $component['cid']);
  $counts = array();
  while ($data = db_fetch_object($result)) {
    $counts[$data->no][$data->data] = $data->datacount;
  }

  // Create an entire table to be put into the returned row.
  $rows = array();
  $header = array('') + $options;
  foreach ($questions as $qkey => $question) {
    $row = array($question);
    foreach ($options as $okey => $option) {
      $row[] = !empty($counts[$qkey][$option]) ? $counts[$qkey][$option] : 0;
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows);

  return array(array(array('data' => $output, 'colspan' => 2)));
}

/** 
 * function _webform_table_data_select
 * Return the result of this component's submission for display in a table. The output of this function will be displayed under the "results" tab then "table"
 * @param $data An array of information containing the submission result, directly correlating to the webform_submitted_data database schema
 * @returns Textual output formatted for human reading.
 **/
function _webform_table_data_grid($data, $component) {
  $questions = explode("\n", $component['extra']['questions']);
  // Set the value as a single string
  if (is_array($data['value'])) {
    foreach ($data['value'] as $item => $value) {
      if ($value !== '0') {
        $output .= check_plain(trim($questions[$item])) .': '. check_plain($value) ."<br />";
      }
    }
  }
  else {
    $output = check_plain(empty($data['value']['0']) ? "" : $data['value']['0']);
  }
  return $output;
}

/** 
 * Return the header information for this component to be displayed in a comma
 * seperated value file. The output of this function will be displayed under
 * the "results" tab then "download".
 * 
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of data to be displayed in the first three rows of a CSV file, not
 *   including either prefixed or trailing commas.
 **/
function _webform_csv_headers_grid($component) {
  $header = array();
  $header[0] = '';
  $header[1] = $component['name'];
  $items = split("\n", $component['extra']['questions']);
  foreach ($items as $item) {
    $header[2] .= "\,". $item;
    // Empty column per sub-field in main header.
    $header[1] .= "\,";
  }
  // Remove the preceding extra comma.
  $header[2] = substr($header[2], 2);
  // Remove the trailing column.
  $header[1] = substr($header[1], 0, -2);
  return $header;
}

/** 
 * Return the result of a textfield submission. The output of this function will
 * be displayed under the "results" tab then "submissions".
 * 
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @return
 *   Textual output formatted for CSV, not including either prefixed or trailing
 *   commas.
 **/
function _webform_csv_data_grid($data, $component) {
  $rows = explode("\n", _webform_filtervalues($component['extra']['questions']));
  $questions = array();
  $cid = 0;
  foreach ($rows as $row) {
    $row = trim($row);
    if (preg_match('/^([^"|]+)\|(.*)$/', $row, $matches)) {
      $questions[$matches[1]] = $matches[2];
    }
    else {
      $questions[$cid++] = $row;
    } 
  }
  foreach ($questions as $item => $question) {
    $output .= "\," . $data['value'][$item];
  }
  // Remove the preceding extra comma
  $output = substr($output, 2);
  return $output;
}

function theme_webform_grid(&$grid_element) {
  $rows = array();
  $header = array('');
  $first = TRUE;
  foreach (element_children($grid_element) as $key) {
    $question_element = $grid_element[$key];

    // Set the header for the table.
    if ($first) {
      foreach ($question_element['#options'] as $option) {
        $header[] = $option;
      }
      $first = FALSE;
    }

    // Create a row with the question title.
    $row = array($question_element['#title']);

    // Render each radio button in the row.
    $radios = expand_radios($question_element);
    foreach (element_children($radios) as $key) {
      unset($radios[$key]['#title']);
      $row[] = drupal_render($radios[$key]);
    }
    $rows[] = $row;
  }

  return theme('form_element', $grid_element, theme('table', $header, $rows));
}
