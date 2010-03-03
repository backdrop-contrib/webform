<?php
// $Id$

/**
 * @file
 * Webform module grid component.
 */

// Grid depends on functions provided by select.
webform_component_include('select');

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
 * Implementation of _webform_help_grid().
 */
function _webform_theme_grid() {
  return array(
    'webform_grid' => array(
      'arguments' => array('grid_element' => NULL),
    ),
    'webform_display_grid' => array(
      'arguments' => array('element' => NULL),
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
  $form['display']['optrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Options'),
    '#default_value' => $component['extra']['optrand'],
    '#description' => t('Randomizes the order of options on the top when they are displayed in the form.'),
    '#parents' => array('extra', 'optrand')
  );
  $form['display']['qrand'] = array(
    '#type' => 'checkbox',
    '#title' => t('Randomize Questions'),
    '#default_value' => $component['extra']['qrand'],
    '#description' => t('Randomize the order of the questions on the side when they are displayed in the form.'),
    '#parents' => array('extra', 'qrand')
  );
  return $form;
}

/**
 * Implementation of _webform_render_component().
 */
function _webform_render_grid($component, $value = NULL, $filter = TRUE) {
  $element = array(
    '#title' => $component['name'],
    '#required' => $component['mandatory'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_grid',
    '#description' => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
  );

  $questions = _webform_grid_options($component['extra']['questions']);
  $options = _webform_grid_options($component['extra']['options']);

  if ($component['extra']['optrand']) {
    _webform_shuffle_options($options);
  }

  $delta = 0;
  foreach ($questions as $question) {
    $delta++;
    if ($question != '') {
      $element['q' . $delta] = array(
        '#title'         => $question,
        '#required'      => $component['mandatory'],
        '#options'       => $options,
        '#type'          => 'radios',
        '#process'       => array('expand_radios', 'webform_expand_select_ids'),
        '#validated'     => TRUE, // Webform handles validation separately.
      );
    }
  }

  if (isset($value)) {
    $cid = 0;
    foreach (element_children($element) as $key) {
      $element[$key]['#default_value'] = $value[$cid++];
    }
  }

  // Shuffle questions if needed.
  if ($component['extra']['qrand']) {
    $aux = array();
    $keys = element_children($element);
    shuffle($keys);
    foreach ($keys as $key) {
      $aux[$key] = array();
    }
    $element = array_merge($aux, $element);
  }

  return $element;
}

/**
 * Implementation of _webform_display_component().
 */
function _webform_display_grid($component, $value, $format = 'html') {
  $questions = _webform_grid_options($component['extra']['questions']);
  $options = _webform_grid_options($component['extra']['options']);

  $element = array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#component' => $component,
    '#format' => $format,
    '#questions' => $questions,
    '#options' => $options,
    '#theme' => 'webform_display_grid',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#sorted' => TRUE,
  );

  $cid = 0;
  foreach ($questions as $question) {
    if ($question != '') {
      $element[$question] = array(
        '#title' => $question,
        '#value' => isset($value[$cid]) ? $value[$cid] : NULL,
      );
    }
    $cid++;
  }

  return $element;
}

/**
 * Format the text output for this component.
 */
function theme_webform_display_grid($element) {
  $component = $element['#component'];
  $format = $element['#format'];

  if ($format == 'html') {
    $rows = array();
    $header = array(array('data' => '', 'class' => 'webform-grid-question'));
    foreach ($element['#options'] as $option) {
      $header[] = array('data' => _webform_filter_xss($option), 'class' => 'checkbox webform-grid-option');
    }
    foreach (element_children($element) as $key) {
      $row = array();
      $row[] = array('data' => _webform_filter_xss($element[$key]['#title']), 'class' => 'webform-grid-question');
      foreach ($element['#options'] as $option_value => $option_label) {
        if ($option_value == $element[$key]['#value']) {
          $row[] = array('data' => '<strong>X</strong>', 'class' => 'checkbox webform-grid-option');
        }
        else {
          $row[] = array('data' => '&nbsp', 'class' => 'checkbox webform-grid-option');
        }
      }
      $rows[] = $row;
    }

    $option_count = count($header) - 1;
    $output = theme('table', $header, $rows, array('class' => 'webform-grid webform-grid-' . $option_count));
  }
  else {
    $items = array();
    foreach (element_children($element) as $key) {
      $items[] = ' - ' . $element[$key]['#title'] . ': ' . $element[$key]['#value'];
    }
    $output = implode("\n", $items);
  }

  return $output;
}

/**
 * Implementation of _webform_submit_component().
 */
function _webform_submit_grid($component, $value) {
  $options = drupal_map_assoc(array_flip(_webform_grid_options($component['extra']['options'])));
  $questions = _webform_grid_options($component['extra']['questions']);

  // Put the form in the original option order before saving.
  // Return the final data with the quotes back in place.
  $ordered_value = array();
  $delta = 0;
  foreach ($questions as $safe_question => $question) {
    $delta++;
    $ordered_value['q' . $delta] = isset($value['q' . $delta]) ? $value['q' . $delta] : '';
  }

  return $ordered_value;
}

/**
 * Implementation of _webform_analysis_component().
 */
function _webform_analysis_grid($component, $sids = array()) {
  // Generate the list of options and questions.
  $options = _webform_grid_options($component['extra']['options']);
  $questions = array_values(_webform_grid_options($component['extra']['questions']));

  // Generate a lookup table of results.
  $placeholders = count($sids) ? array_fill(0, count($sids), "'%s'") : array();
  $sidfilter = count($sids) ? " AND sid in (" . implode(",", $placeholders) . ")" : "";
  $query = 'SELECT no, data, count(data) as datacount ' .
    ' FROM {webform_submitted_data} ' .
    ' WHERE nid = %d ' .
    ' AND cid = %d ' .
    " AND data != '' " . $sidfilter .
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
    $header[] = _webform_filter_xss($option);
  }

  // Add questions as each row.
  foreach ($questions as $qkey => $question) {
    $row = array(_webform_filter_xss($question));
    foreach ($options as $okey => $option) {
      $row[] = !empty($counts[$qkey][$okey]) ? $counts[$qkey][$okey] : 0;
    }
    $rows[] = $row;
  }
  $output = theme('table', $header, $rows, array('class' => 'webform-grid'));


  return array(array(array('data' => $output, 'colspan' => 2)));
}

/**
 * Implementation of _webform_table_component().
 */
function _webform_table_grid($component, $value) {
  $questions = array_values(_webform_grid_options($component['extra']['questions']));
  $output = '';
  // Set the value as a single string.
  if (is_array($value)) {
    foreach ($value as $item => $value) {
      if ($value !== '') {
        $output .= _webform_filter_xss($questions[$item]) . ': ' . _webform_filter_xss($value) . '<br />';
      }
    }
  }
  else {
    $output = !isset($value[0]) ? '' : _webform_filter_xss($value[0]);
  }
  return $output;
}

/**
 * Implementation of _webform_csv_headers_component().
 */
function _webform_csv_headers_grid($component, $export_options) {
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
function _webform_csv_data_grid($component, $export_options, $value) {
  $questions = array_keys(_webform_grid_options($component['extra']['questions']));
  $return = array();
  foreach ($questions as $key => $question) {
    $return[] = isset($value[$key]) ? $value[$key] : '';
  }
  return $return;
}

function theme_webform_grid($element) {
  $rows = array();
  $header = array(array('data' => '', 'class' => 'webform-grid-question'));
  $first = TRUE;
  foreach (element_children($element) as $key) {
    $question_element = $element[$key];

    // Set the header for the table.
    if ($first) {
      foreach ($question_element['#options'] as $option) {
        $header[] = array('data' => _webform_filter_xss($option), 'class' => 'checkbox webform-grid-option');
      }
      $first = FALSE;
    }

    // Create a row with the question title.
    $row = array(array('data' => _webform_filter_xss($question_element['#title']), 'class' => 'webform-grid-question'));

    // Render each radio button in the row.
    $radios = expand_radios($question_element);
    foreach (element_children($radios) as $key) {
      unset($radios[$key]['#title']);
      $row[] = array('data' => drupal_render($radios[$key]), 'class' => 'checkbox webform-grid-option');
    }
    $rows[] = $row;
  }

  $option_count = count($header) - 1;
  return theme('form_element', $element, theme('table', $header, $rows, array('class' => 'webform-grid webform-grid-' . $option_count)));
}

/**
 * Utility function to convert user-entered values into grid options.
 */
function _webform_grid_options($text) {
  $options = array();
  $rows = array_filter(explode("\n", _webform_filter_values(trim($text), NULL, NULL, NULL, FALSE)));

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
