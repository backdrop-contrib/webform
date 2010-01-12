<?php
// $Id$

/**
 * @file
 * Webform module grid component.
 */

/**
 * Implementation of _webform_defaults_component().
 */
function _webform_defaults_grid() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'email' => 1,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'extra' => array(
      'options' => '',
      'questions' => '',
      'optrand' => 0,
      'qrand' => 0,
      'description' => '',
    ),
  );
}

/**
 * Implementation of _webform_edit_component().
 */
function _webform_edit_grid($component) {
  $form = array();
  $form['extra']['options'] = array(
    '#type' => 'textarea',
    '#title' => t('Options'),
    '#default_value' => $component['extra']['options'],
    '#description' => t('Options to select across the top. One option per line. Key-value pairs may be entered seperated by pipes. i.e. safe_key|Some readable option') . theme('webform_token_help'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -3,
    '#required' => TRUE,
  );
  $form['extra']['questions'] = array(
    '#type' => 'textarea',
    '#title' => t('Questions'),
    '#default_value' => $component['extra']['questions'],
    '#description' => t('Questions list down the left side. One question per line.') . theme('webform_token_help'),
    '#cols' => 60,
    '#rows' => 5,
    '#weight' => -2,
    '#required' => TRUE,
  );
  $form['extra']['optrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Options'),
    '#default_value' => $component['extra']['optrand'],
    '#description' => t('Randomizes the order of options on the top when they are displayed in the form.'),
  );
  $form['extra']['qrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Questions'),
    '#default_value' => $component['extra']['qrand'],
    '#description' => t('Randomize the order of the questions on the side when they are displayed in the form.'),
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_grid($component, $value = NULL, $filter = TRUE) {
  $form_item = array(
    '#title' => $component['name'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_grid',
    '#description' => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
  );

  $questions = _webform_grid_options($component['extra']['questions']);
  $options = _webform_grid_options($component['extra']['options']);

  if ($component['extra']['optrand']) {
    // This maneuver shuffles the array keys, then uses them as
    // the basis for ordering the options.
    $aux = array();
    $keys = array_keys($options);
    shuffle($keys);
    foreach ($keys as $key) {
      $aux[$key] = $options[$key];
      unset($options[$key]);
    }
    $options = $aux;
  }
  if ($component['extra']['qrand']) {
    $aux = array();
    $keys = array_keys($questions);
    shuffle($keys);
    foreach ($keys as $key) {
      $aux[$key] = $questions[$key];
      unset($questions[$key]);
    }
    $questions = $aux;
  }
  foreach ($questions as $question) {
    if ($question != '') {
      // Remove quotes from keys to prevent HTML breakage.
      $form_item[str_replace(array('"', "'"), '', $question)] = array(
        '#title'         => $question,
        '#required'      => $component['mandatory'],
        '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
        '#suffix'        => '</div>',
        '#options'       => $options,
        '#type'          => 'radios',
      );
    }
  }

  if (isset($value)) {
    $cid = 0;
    foreach (element_children($form_item) as $key) {
      $form_item[$key]['#default_value'] = $value[$cid++];
    }
  }

  return $form_item;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_grid($component, $value) {
  $form_item = _webform_render_grid($component, $value);
  foreach (element_children($form_item) as $key) {
    $form_item[$key]['#disabled'] = TRUE;
  }
  return $form_item;
}

/**
 * Implementation of _webform_submit_component().
 */
function _webform_submit_grid($component, $value) {
  $options = drupal_map_assoc(array_flip(_webform_grid_options($component['extra']['options'])));

  // Questions are a bit more tricky, since quotes were removed from them in
  // _webform_render_grid(). Build a list of no_quotes => with_qoutes questions.
  $questions = array();
  foreach (_webform_grid_options($component['extra']['questions']) as $key => $question) {
    $safe_question = str_replace(array('"', "'"), '', $question);
    $questions[$safe_question] = $question;
  }

  if (is_array($value)) {
    foreach ($value as $key => $option_value) {
      if ($option_value !== '') {
        $value[$key] = $options[$option_value];
      }
    }
  }
  elseif ($value !== '') {
    $value = $options[$value];
  }

  // Put the form in the original option order before saving.
  // Return the final data with the quotes back in place.
  $ordered_value = array();
  foreach ($questions as $safe_question => $question) {
    if (isset($value[$safe_question])) {
      $ordered_value[$question] = $value[$safe_question];
    }
  }

  return $ordered_value;
}
/**
 * Format the output of e-mailed data for this component.
 *
 * @param $component
 *   A Webform component array.
 * @param $value
 *   A string or array of the submitted data.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_grid($component, $value) {
  $questions = _webform_grid_options($component['extra']['questions']);
  $output = $component['name'] .":\n";
  foreach ($questions as $key => $question) {
    $output .= '  - '. $question .':'. ($value[$question] == '' ? '' : ' '. $value[$question]) ."\n";
  }
  return $output;
}

/**
 * Implementation of _webform_help_component().
 */
function _webform_help_grid($section) {
  switch ($section) {
    case 'admin/settings/webform#grid_description':
      return t('Allows creation of grid questions, denoted by radio buttons.');
  }
}

/**
 * Implementation of _webform_help_grid().
 */
function _webform_theme_grid() {
  return array(
    'webform_grid' => array(
      'arguments' => array('grid_element' => NULL),
    ),
    'webform_mail_grid' => array(
      'arguments' => array('component' => NULL, 'value' => NULL),
    ),
  );
}

/**
 * Implementation of _webform_analysis_rows_component().
 */
function _webform_analysis_rows_grid($component, $sids = array()) {
  // Generate the list of options and questions.
  $options = _webform_grid_options($component['extra']['options']);
  $questions = array_values(_webform_grid_options($component['extra']['questions']));

  // Generate a lookup table of results.
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (".implode(",", $placeholders).")" : "";
  $query = 'SELECT no, data, count(data) as datacount '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d '.
    " AND data != '' ". $sidfilter .
    ' GROUP BY no, data';
  $result = db_query($query, array_merge(array($component['nid'], $component['cid']), $sids));
  $counts = array();
  while ($data = db_fetch_object($result)) {
    $counts[$data->no][$data->data] = $data->datacount;
  }

  // Create an entire table to be put into the returned row.
  $rows = array();
  $header = array('');

  // Add options as a header row.
  foreach ($options as $option) {
    $header[] = $option;
  }

  // Add questions as each row.
  foreach ($questions as $qkey => $question) {
    $row = array($question);
    foreach ($options as $okey => $option) {
      $row[] = !empty($counts[$qkey][$okey]) ? $counts[$qkey][$okey] : 0;
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('class' => 'webform-grid'));

  return array(array(array('data' => $output, 'colspan' => 2)));
}

/**
 * Implementation of _webform_table_data_component().
 */
function _webform_table_data_grid($component, $value) {
  $questions = array_values(_webform_grid_options($component['extra']['questions']));
  $output = '';
  // Set the value as a single string.
  if (is_array($value)) {
    foreach ($value as $item => $value) {
      if ($value !== '') {
        $output .= $questions[$item] .': '. check_plain($value) .'<br />';
      }
    }
  }
  else {
    $output = check_plain(!isset($value[0]) ? '' : $value[0]);
  }
  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_grid($component) {
  $header = array();
  $header[0] = array('');
  $header[1] = array($component['name']);
  $items = _webform_grid_options($component['extra']['questions']);
  $count = 0;
  foreach ($items as $key => $item) {
    // Empty column per sub-field in main header.
    if ($count != 0) {
      $header[0][] = '';
      $header[1][] = '';
    }
    // The value for this option.
    $header[2][] = $item;
    $count++;
  }

  return $header;
}

/**
 * Implementation of _webform_csv_data_component().
 */
function _webform_csv_data_grid($component, $value) {
  $questions = array_keys(_webform_grid_options($component['extra']['questions']));
  $return = array();
  foreach ($questions as $key => $question) {
    $return[] = isset($value[$key]) ? $value[$key] : '';
  }
  return $return;
}

function theme_webform_grid($grid_element) {
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
    $row = array(check_plain($question_element['#title']));

    // Render each radio button in the row.
    $radios = expand_radios($question_element);
    foreach (element_children($radios) as $key) {
      unset($radios[$key]['#title']);
      $row[] = drupal_render($radios[$key]);
    }
    $rows[] = $row;
  }

  return theme('form_element', $grid_element, theme('table', $header, $rows, array('class' => 'webform-grid')));
}

/**
 * Utility function to convert user-entered values into grid options.
 */
function _webform_grid_options($text) {
  $options = array();
  $rows = array_filter(explode("\n", _webform_filter_values(trim($text))));
  foreach ($rows as $option) {
    $option = trim($option);
    if (preg_match('/^([^|]+)\|(.*)$/', $option, $matches)) {
      $options[$matches[1]] = $matches[2];
    }
    else {
      $options[$option] = $option;
    }
  }
  return $options;
}
