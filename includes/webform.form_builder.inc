<?php

/**
 * Menu callback; displays the main component editing screen.
 */
function webform_form_builder_components($node) {
  module_load_include('inc', 'form_builder', 'includes/form_builder.admin');

  $output = '';
  $output .= form_builder_interface('webform', $node->nid);
  $output .= drupal_get_form('webform_form_builder_save_form', $node->nid);

  return $output;
}

/**
 * Form to save the Form Builder interface.
 *
 * The actual Form Builder interface is displayed as part of the
 * webform_form_builder_components() function.
 */
function webform_form_builder_save_form(&$form_state, $nid) {
  $form = array();

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $nid,
  );
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#submit' => array('webform_form_builder_cancel'),
  );

  return $form;
}

/**
 * Save the current Form Builder interface changes.
 */
function webform_form_builder_save_form_submit($form, $form_state) {
  $node = node_load($form_state['values']['nid']);
  $form_cache = form_builder_cache_load('webform', $node->nid);
  $element_ids = form_builder_preview_prepare($form_cache, 'webform', $node->nid);

  // Save modified or created components.
  foreach ($element_ids as $id) {
    webform_form_builder_save($node, $id, $form_cache);
  }

  // Delete components that have been removed.
  foreach ($node->webform['components'] as $component) {
    $element_id = 'cid_'. $component['cid'];
    if (!in_array($element_id, $element_ids)){
      webform_component_delete($node, $component);
    }
  }

  drupal_set_message(t('Changes to the form have been saved.'));
  form_builder_cache_delete('webform', $node->nid);
}

/**
 * Save the contents of a form into Webform's database tables.
 */
function webform_form_builder_save($node, $element_id, $form) {
  $element = form_builder_get_element($form, $element_id);
  $type = isset($element['#webform']['type']) ? $element['#webform']['type'] : $element['#form_builder']['element_type'];

  // Check for existing components.
  $cid = substr($element_id, 4);
  if (isset($node->webform['components'][$cid])) {
    $component = $node->webform['components'][$cid];
  }
  // Populate a new default component.
  elseif (isset($element['#webform']['is_new'])) {
    $defaults_function = '_webform_defaults_'. $type;
    if (function_exists($defaults_function)) {
      $component = $defaults_function();
      $component['type'] = $type;
    }
  }

  // If the component type isn't found, don't try to save anything.
  if (!isset($component)) {
    return;
  }

  // Set typical component settings.
  $component['name'] = $element['#title'];
  $component['weight'] = $element['#weight'];
  $component['form_key'] = $element['#key'];
  $component['mandatory'] = isset($element['#required']) ? $element['#required'] : 0;
  $component['email'] = 0;
  $component['pid'] = isset($element['#form_builder']['parent_id']) ? substr($element['#form_builder']['parent_id'], 4) : 0;
  $component['nid'] = $node->nid;

  if (isset($element['#default_value'])) {
    $component['value'] = is_array($element['#default_value']) ? implode(',', $element['#default_value']) : $element['#default_value'];
  }

  // Save any specific settings.
  $save_function = '_webform_form_builder_save_'. $type;
  if (function_exists($save_function)) {
    $save_function($component, $element);
  }

  if (!isset($component['cid'])) {
    webform_component_insert($component);
  }
  elseif ($component != $node->webform['components'][$component['cid']]) {
    webform_component_update($component);
  }
}

/**
 * Submit handler for the "Cancel" button.
 */
function webform_form_builder_cancel(&$form, &$form_state) {
  drupal_set_message(t('Changes to the form have been discarded.'));
  form_builder_cache_delete('webform', $form_state['values']['nid']);
}

/**
 * Implementation of hook_form_builder_types().
 *
 * Define the fields within webforms that are editable.
 */
function webform_form_builder_types() {
  $fields = array();

  $components = webform_load_components();
  foreach ($components as $key => $component) {
    $function = '_webform_form_builder_types_'. $key;
    if (function_exists($function)) {
      $fields = array_merge($fields, $function());
    }
  }

  return array('webform' => $fields);
}


/**
 * Implementation of hook_form_builder_properties().
 */
function webform_form_builder_properties($form_type) {
  $properties = array();

  if ($form_type == 'webform') {
    $components = webform_load_components();
    foreach ($components as $key => $component) {
      $function = '_webform_form_builder_properties_'. $key;
      if (function_exists($function)) {
        $properties = array_merge($properties, $function());
      }
    }
  }

  return $properties;
}

/**
 * Implementation of hook_form_builder_load().
 *
 * Load a complete FAPI array based on a form type and ID.
 */
function webform_form_builder_load($form_builder_type, $form_builder_id) {
  if ($form_builder_type == 'webform') {
    // Webform identifies its forms by Node Id.
    $nid = $form_builder_id;
    $node = node_load($nid);
    // Get the unfiltered version of the client form.
    $form = webform_client_form(array(), $node, array(), TRUE, FALSE);
    webform_form_builder_load_process($form['submitted'], $node);
    return $form['submitted'];
  }
}

/**
 * Recursive helper function to populate #form_builder['element_id'] values.
 */
function webform_form_builder_load_process(&$form, $node, $pid = 0) {
    foreach (element_children($form) as $key) {
      $cid = webform_get_cid($node, $key, $pid);
      $form[$key]['#form_builder'] = array(
        'element_id' => 'cid_'. $cid,
      );

      // Add any component-specific loading. Note that all components are
      // invoked here because the component type isn't yet known.
      foreach (webform_load_components() as $component => $name) {
        $load_function = '_webform_form_builder_load_' . $component;
        if (function_exists($load_function)) {
          $load_function($form[$key]);
        }
      }
      webform_form_builder_load_process($form[$key], $node, $cid);
    }
}

/**
 * Implementation of hook_form_builder_add_element_alter().
 *
 * Modify a FAPI element before it is added to the form array.
 */
function webform_form_builder_add_element_alter(&$element, $form_type, $form_id) {
  if ($form_type == 'webform') {
    $element['#webform']['is_new'] = TRUE;
  }
}

/**
 * Implementation of hook_form_builder_preview_alter().
 *
 * The most common use of the preview altering is filtering field descriptions
 * via filter_xss() or other functions. Webform has it's own filtering function
 * for this purpose.
 */
function webform_form_builder_preview_alter(&$element, $form_type, $form_id) {
  if ($form_type == 'webform') {
    // Filter all descriptions for all components.
    if (isset($element['#description'])) {
      $element['#description'] = _webform_filter_descriptions($element['#description'], NULL, NULL, FALSE);
    }

    if (isset($element['#default_value']) && is_string($element['#default_value'])) {
      $element['#value'] = _webform_filter_values($element['#default_value'], NULL, NULL, FALSE);
    }

    // Let components do any extra filtering if needed.
    $type = isset($element['#webform']['type']) ? $element['#webform']['type'] : $element['#form_builder']['element_type'];
    $function = '_webform_form_builder_preview_alter_' . $type;
    if (function_exists($function)) {
      $function($element);
    }
  }
}
