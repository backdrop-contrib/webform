<?php
// $Id$

/**
 * Provides interface and database handling for editing components of a webform.
 * 
 * @author Nathan Haug <nate@lullabot.com>
 */

/**
 * Overview form of all components for this webform.
 */
function webform_components_form($node) {
  $form = array(
    '#tree' => TRUE,
    '#node' => $node,
    'components' => array(),
  );

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $options = array();
  foreach ($node->webform['components'] as $cid => $component) {
    $options[$cid] = check_plain($component['name']);
    $form['components'][$cid]['pid'] = array(
      '#type' => 'hidden',
      '#default_value' => $component['pid'],
    );
    $form['components'][$cid]['weight'] = array(
      '#type' => 'weight',
      '#delta' => count($node->webform['components']) > 10 ? count($node->webform['components']) : 10,
      '#title' => t("Weight"),
      '#default_value' => $component['weight'],
    );
    $form['components'][$cid]['mandatory'] = array(
      '#type' => 'checkbox',
      '#delta' => $component['mandatory'],
      '#title' => t("Mandatory"),
      '#default_value' => $component['mandatory'],
      '#access' => !in_array($component['type'], array('markup', 'fieldset', 'pagebreak')),
    );
  }

  $form['add']['name'] = array(
    '#type' => 'textfield',
    '#size' => 30,
  );

  $component_types = webform_load_components();
  natcasesort($component_types);
  $form['add']['type'] = array(
    '#type' => 'select',
    '#options' => $component_types,
    '#weight' => 3,
  );
  $form['add']['mandatory'] = array(
    '#type' => 'checkbox',
  );
  $form['add']['pid'] = array(
    '#type' => 'hidden',
    '#default_value' => 0,
  );
  $form['add']['weight'] = array(
    '#type' => 'weight',
    '#delta' => count($node->webform['components']) > 10 ? count($node->webform['components']) : 10,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#weight' => 45,
  );

  $form['publish'] = array(
    '#type' => 'submit',
    '#value' => t('Publish'),
    '#weight' => 50,
    '#access' => !$node->status,
  );

  return $form;
}

/**
 * Theme the node components form. Use a table to organize the components.
 *
 * @param $form
 *   The form array.
 * @return
 *   Formatted HTML form, ready for display.
 */
function theme_webform_components_form($form) {
  // Add CSS to display submission info. Don't preprocess because this CSS file is used rarely.
  drupal_add_css(drupal_get_path('module', 'webform') .'/webform.css', 'module', 'all', FALSE);
  drupal_add_js(drupal_get_path('module', 'webform') .'/webform.js', 'module', 'header', FALSE, TRUE, FALSE);

  $node = $form['#node'];

  $headers = array(t('Name'), t('Type'), t('Value'), t('Mandatory'), t('Weight'), array('data' => t('Operations'), 'colspan' => 3));
  $rows = array();
  if (!empty($node->webform['components'])) {
    $component_tree = array();
    $page_count = 1;
    _webform_components_tree_build($node->webform['components'], $component_tree, 0, $page_count);
    $component_tree = _webform_components_tree_sort($component_tree);
    // Build the table rows.
    function _webform_components_form_rows($node, $cid, $component, $level, &$form, &$rows) {
      // Create presentable values.
      if (strlen($component['value']) > 30) {
        $component['value'] = substr($component['value'], 0, 30);
        $component['value'] .= "...";
      }
      $component['value'] = check_plain($component['value']);

      // Remove individual titles from the mandatory and weight fields.
      unset($form['components'][$cid]['mandatory']['#title']);
      unset($form['components'][$cid]['pid']['#title']);
      unset($form['components'][$cid]['weight']['#title']);

      // Build indentation for this row.
      $indents = '';
      for ($n = 1; $n <= $level; $n++) {
        $indents .= '<div class="webform-indentation">&nbsp;</div>';
      }

      // Add each component to a table row.
      $rows[] = array(
        $indents . l($component['name'], 'node/'. $node->nid .'/edit/components/'. $cid, array('title' => t('Edit this component'))),
        $component['type'],
        ($component['value'] == "") ? "-" : $component['value'],
        drupal_render($form['components'][$cid]['mandatory']),
        drupal_render($form['components'][$cid]['pid']) . drupal_render($form['components'][$cid]['weight']),
        l(t('Edit'), 'node/'. $node->nid .'/edit/components/'. $cid),
        l(t('Clone'), 'node/'. $node->nid .'/edit/components/'. $cid .'/clone'),
        l(t('Delete'), 'node/'. $node->nid .'/edit/components/'. $cid .'/delete'),
      );
      if (is_array($component['children'])) {
        foreach ($component['children'] as $cid => $component) {
          _webform_components_form_rows($node, $cid, $component, $level + 1, $form, $rows);
        }
      }
    }
    foreach ($component_tree['children'] as $cid => $component) {
      _webform_components_form_rows($node, $cid, $component, 0, $form, $rows);
    }
  }
  else {
    $rows[] = array(array('data' => t("No Components, add a component below."), 'colspan' => 8));
  }

  // Add a row containing form elements for a new item.
  unset($form['add']['name']['#title'], $form['add_type']['#description']);
  $form['add']['name']['#value'] = t('New component name');
  $form['add']['name']['#attributes']['class'] = 'webform-default-value';
  $rows[] = array(
    drupal_render($form['add']['name']),
    drupal_render($form['add']['type']),
    '',
    drupal_render($form['add']['mandatory']),
    drupal_render($form['add']['pid']) . drupal_render($form['add']['weight']),
    '',
    '',
    '',
  );

  $output = '';
  $output .= theme('table', $headers, $rows);
  $output .= drupal_render($form);
  return $output;
}

function webform_components_form_submit($form_id, $form_values) {
  $node = node_load($form_values['nid']);

  // Update all mandatory and weight values.
  foreach ($node->webform['components'] as $cid => $component) {
    if ($component['weight'] != $form_values['components'][$cid]['weight'] || $component['mandatory'] != $form_values['components'][$cid]['mandatory']) {
      $component['weight'] = $form_values['components'][$cid]['weight'];
      $component['mandatory'] = $form_values['components'][$cid]['mandatory'];
      $component['nid'] = $node->nid;
      webform_component_update($component);
    }
  }

  if ($_POST['op'] == t('Publish')) {
    $node->status = 1;
    node_save($node);
    drupal_set_message(t('Your webform has been published.'));
    return 'node/'. $node->nid;
  }

  // Initiate a new form element if a name has been specified.
  if (strlen(trim($form_values['add']['name'])) > 0) {
    $component = $form_values['add'];
    return array('node/'. $node->nid .'/edit/components/new/'. $component['type'], 'name='. urlencode($component['name']) .'&mandatory='. $component['mandatory'] .'&pid='. $component['pid'] .'&weight='. $component['weight']);
  }
  else {
    drupal_set_message(t('The component positions and mandatory values have been updated.'));
  }
}

function webform_component_edit_form(&$node, $component, $clone = FALSE) {
  drupal_set_title("Edit component: ". $component['name'] ." (". $component['type'] .")");

  // Print the correct field type specification.
  // We always need: name and description.
  $form = array(
    '#tree' => TRUE,
  );
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $component['type'],
  );
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $component['cid'],
  );
  $form['clone'] = array(
    '#type' => 'value',
    '#value' => $clone,
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#default_value' => $component['name'],
    '#title' => t("Label"),
    '#description' => t('This is used as a descriptive label when displaying this form element.'),
    '#required' => TRUE,
    '#weight' => -2,
  );
  $form['form_key'] = array(
    '#type' => 'textfield',
    '#default_value' => empty($component['form_key']) ? _webform_safe_name($component['name']) : $component['form_key'],
    '#title' => t('Field Key'),
    '#description' => t('Enter a machine readable key for this form element. May contain only lowercase alphanumeric characters and underscores. This key will be used as the name attribute of the form element.'),
    '#required' => TRUE,
    '#weight' => -1,
  );
  $form['extra']['description'] = array(
    '#type' => 'textfield',
    '#default_value' => $component['extra']['description'],
    '#title' => t("Description"),
    '#maxlength' => '512',
    '#description' => t('A short description of the field used as help for the user when he/she uses the form.') .'<br />'. webform_help('webform/helptext#variables'),
    '#weight' => -1,
  );
  $form['mandatory'] = array(
    '#type' => 'checkbox',
    '#title' => t("Mandatory"),
    '#default_value' => ($component['mandatory'] == '1' ? TRUE : FALSE),
    '#description' => t('Check this option if the user must enter a value.'),
    '#weight' => 2,
  );

  if (variable_get('webform_enable_fieldset', true) && is_array($node->webform['components'])) {
    $options = array('0' => t('Root'));
    foreach ($node->webform['components'] as $thiscid => $value) {
      if ($value['type'] == 'fieldset' && $thiscid != $component['cid']) {
        $options[$thiscid] = $value['name'];
      }
    }
    $form['pid'] = array(
      '#type' => 'select',
      '#title' => t("Parent Fieldset"),
      '#default_value' => $component['pid'],
      '#description' => t('Optional. You may organize your form by placing this component inside inside another fieldset.'),
      '#options' => $options,
      '#weight' => 3,
    );
  }

  $form['weight'] = array(
    '#type' => 'weight',
    '#delta' => count($node->webform['components']) > 10 ? count($node->webform['components']) : 10,
    '#title' => t("Weight"),
    '#default_value' => $component['weight'],
    '#description' => t('Optional. In the menu, the heavier items will sink and the lighter items will be positioned nearer the top.'),
    '#weight' => 4,
  );

  // Add the fields specific to this component type:
  webform_load_components(); // Load all component types.
  $edit_function = "_webform_edit_". $component['type'];
  $additional_form_elements = array();
  if (function_exists($edit_function)) {
    $additional_form_elements = $edit_function($component); // Call the component render function.
  }
  else {
    drupal_set_message(t("The webform component of type @type does not have an edit function defined.", array('@type' => $component['type'])));
  }

  // Merge the additional fields with the current fields:
  $extra_fields_copy = $form['extra'];
  $form = array_merge($form, $additional_form_elements);
  $form['extra'] = array_merge((array)$extra_fields_copy, (array)$additional_form_elements['extra']);

  // Add the submit button.
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#weight' => 5,
  );

  return $form;
}

/**
 * Field name validation for the webform unique key. Must be alphanumeric.
 */
function webform_component_edit_form_validate($form_id, $form_values) {
  $node = node_load($form_values['nid']);

  if (!preg_match('!^[a-z0-9_]+$!', $form_values['form_key'])) {
    form_set_error('form_key', t('The field key %field_key is invalid. Please include only lowercase alphanumeric characters and underscores.', array('%field_key' => $form_values['form_key'])));
  }

  foreach ($node->webform['components'] as $cid => $component) {
    if (($component['cid'] != $form_values['cid'] || $form_values['clone']) && ($component['pid'] == $form_values['pid']) && (strcasecmp($component['form_key'], $form_values['form_key']) == 0)) {
      form_set_error('form_key', t('The field key %field_key is already in use by the field labeled %existing_field. Please use a unique key.', array('%field_key' => $form_values['form_key'], '%existing_field' => $component['name'])));
    }
  }

  // Let the field do any additional validation.
  webform_load_components($form_values['type']);
  $validate_function = '_webform_edit_validate_'. $form_values['type'];
  if (function_exists($validate_function)) {
    $validate_function($form_values);
  }
}

function webform_component_edit_form_submit($form_id, $form_values) {
  $node = node_load($form_values['nid']);

  // Remove empty extra values.
  if (is_array($form_values['extra'])) {
    foreach($form_values['extra'] as $key => $value) {
      if (empty($value)) {
        unset($form_values['extra'][$key]);
      }
    }
  }

  // Remove empty attribute values.
  if (is_array($form_values['extra']['attributes'])) {
    foreach($form_values['extra']['attributes'] as $key => $value) {
      if (empty($value)) {
        unset($form_values['extra']['attributes'][$key]);
      }
    }
  }

  if ($form_values['clone']) {
    drupal_set_message(t('Component %name cloned.', array('%name' => $form_values['name'])));
    webform_component_clone($node, $form_values);
  }
  elseif (!empty($form_values['cid'])) {
    drupal_set_message(t('Component %name updated.', array('%name' => $form_values['name'])));
    webform_component_update($form_values);
  }
  else {
    drupal_set_message(t('New component %name added.', array('%name' => $form_values['name'])));
    webform_component_insert($form_values);
  }

  return 'node/'. $form_values['nid'] .'/edit/components';
}

function webform_component_delete_form($node, $cid) {
  $form = array();
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );
  $form['cid'] = array(
    '#type' => 'value',
    '#value' => $cid,
  );
  $form['name'] = array(
    '#type' => 'value',
    '#value' => $node->webform['components'][$cid]['name'],
  );

  if ($node->webform['components'][$cid]['type'] == 'fieldset') {
    $question = t('Delete the %name fieldset?', array('%name' => $node->webform['components'][$cid]['name']));
    $description = t('This will immediately delete the %name fieldset and all children elements within %name from the %webform webform. This cannot be undone.', array('%name' => $node->webform['components'][$cid]['name'], '%webform' => $node->title));
  }
  else {
    $question = t('Delete the %name component?', array('%name' => $node->webform['components'][$cid]['name']));
    $description = t('This will immediately delete the %name component from the %webform webform. This cannot be undone.', array('%name' => $node->webform['components'][$cid]['name'], '%webform' => $node->title));
  }

  return confirm_form($form, $question, 'node/'. $node->nid .'/components', $description, t('Delete'));
}

function webform_component_delete_form_submit($form_id, $form_values) {
  drupal_set_message(t('Component %name deleted.', array('%name' => $form_values['name'])));
  webform_component_delete($form_values['nid'], $form_values['cid']);
  return 'node/'. $form_values['nid'] .'/edit/components';
}

/**
 * Insert a new component into the database.
 * 
 * @param $component
 *   A full component containing fields from the component form.
 */
function webform_component_insert($component) {
  db_lock_table('webform_component');
  $component['cid'] = db_result(db_query('SELECT MAX(cid) FROM {webform_component} WHERE nid = %d', $component['nid'])) + 1;
  db_query('INSERT INTO {webform_component} (nid, cid, pid, form_key, name, type, value, extra, mandatory, weight) VALUES (%d, %d, %d, "%s", "%s", "%s", "%s", "%s", %d, %d)', $component['nid'], $component['cid'], $component['pid'], $component['form_key'], $component['name'], $component['type'], $component['value'], serialize($component['extra']), $component['mandatory'], $component['weight']);
  db_unlock_tables('webform_component');
  return $component['cid'];
}

/**
 * Update an existing component with new values.
 * 
 * @param $component
 *   A full component containing a nid, cid, and all other fields from the
 *   component form. Additional properties are stored in the extra array.
 */
function webform_component_update($component) {
  return db_query('UPDATE {webform_component} SET pid = %d, form_key = "%s", name = "%s", type = "%s", value = "%s", extra = "%s", mandatory = %d, weight = %d WHERE nid = %d AND cid = %d', $component['pid'], $component['form_key'], $component['name'], $component['type'], $component['value'], serialize($component['extra']), $component['mandatory'], $component['weight'], $component['nid'], $component['cid']);
}

function webform_component_delete($nid, $cid) {
  db_query('DELETE FROM {webform_component} WHERE nid = %d AND cid = %d', $nid, $cid);
  db_query('DELETE FROM {webform_submitted_data} WHERE nid = %d AND cid = %d', $nid, $cid);

  // Delete all elements under this element.
  $result = db_query('SELECT cid FROM {webform_component} WHERE nid = %d AND pid = %d', $nid, $cid);
  while ($row = db_fetch_array($result)) {
    webform_component_delete($nid, $row['cid']);
  }
}

/**
 * Recursively insert components into the database.
 * @param $node
 *   The node object containing the current webform.
 * @param $component
 *   A full component containing fields from the component form.
 */
function webform_component_clone(&$node, $component) {
  $new_cid = webform_component_insert($component);
  if ($component['type'] == 'fieldset') {
    foreach ($node->webform['components'] as $cid => $child_component) {
      if ($child_component['pid'] == $component['cid']) {
        $child_component['pid'] = $new_cid;
        webform_component_clone($node, $child_component);
      }
    }
  }
}
