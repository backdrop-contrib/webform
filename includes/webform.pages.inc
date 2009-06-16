<?php
// $Id$

/**
 * @file
 *
 * Menu callbacks and functions for configuring and editing webforms.
 */

/**
 * Main configuration form for editing a webform node.
 */
function webform_configure_form(&$form_state, $node) {
  $form = array();

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  /* Start Edit Form */
  $form['submission'] = array(
    '#type' => 'fieldset',
    '#title' => t('Submission settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -4,
  );

  $form['submission']['confirmation_wrapper']['confirmation'] = array(
    '#type' => 'textarea',
    '#title' => t('Confirmation message or redirect URL'),
    '#description' => t('Message to be shown upon successful submission or a path to a redirect page. Preface message with <em>message:</em> for a simple message that does not require a page refresh. Redirect pages must start with <em>http://</em> for external sites or <em>internal:</em> for an internal path. i.e. <em>http://www.example.com</em> or <em>internal:node/10</em>'),
    '#default_value' => $node->webform['confirmation'],
    '#cols' => 40,
    '#rows' => 10,
  );

  $form['submission']['confirmation_wrapper']['format'] = filter_form($node->webform['confirmation_format'], NULL, array('confirmation_format'));

  // Submission limit settings.
  $form['submission']['submit_limit'] = array(
    '#type' => 'item',
    '#title' => t('Submission limit'),
    '#theme' => 'webform_advanced_submit_limit_form',
    '#description' => t('Limit the number of submissions <em>per user</em>. A user is identified by their user login if logged-in, or by their IP Address and Cookie if anonymous. Use of cookies may be modified in the global <a href="!url">Webform settings</a>.', array('!url' => url('admin/settings/webform'))),
  );
  $form['submission']['submit_limit']['enforce_limit'] = array(
    '#type' => 'radios',
    '#options' => array('no' => t('Unlimited'), 'yes' => 'Limit to !count submission(s) !timespan'),
    '#default_value' => $node->webform['submit_limit'] == -1 ? 'no' : 'yes',
    '#parents' => array('enforce_limit'),
  );
  $form['submission']['submit_limit']['submit_limit'] = array(
    '#type' => 'textfield',
    '#maxlength' => 2,
    '#size' => 2,
    '#default_value' => $node->webform['submit_limit'] != -1 ? $node->webform['submit_limit'] : '',
    '#parents' => array('submit_limit'),
  );
  $form['submission']['submit_limit']['submit_interval'] = array(
    '#type' => 'select',
    '#options' => array(
      '-1' => t('ever'),
      '1600' => t('every hour'),
      '86400' => t('every day'),
      '604800' => t('every week'),
    ),
    '#default_value' => $node->webform['submit_interval'],
    '#parents' => array('submit_interval'),
  );

  /* End Edit Form */

  /* Start per-role submission control */
  $form['role_control'] = array(
    '#type' => 'fieldset',
    '#title' => t('Submission access'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -3,
    '#description' => t('These permissions affect which roles can submit this webform. It does not prevent access to the webform page. If needing to prevent access to the webform page entirely, use a content access module such as <a href="http://drupal.org/project/taxonomy_access">Taxonomy Access</a> or <a href="http://drupal.org/project/node_privacy_byrole">Node Privacy by Role</a>.'),
    '#access' => variable_get('webform_submission_access_control', 1),
  );
  $user_roles = user_roles();
  $form['role_control']['roles'] = array(
    '#default_value' => $node->webform['roles'],
    '#options' => $user_roles,
    '#type' => 'checkboxes',
    '#title' => t('Roles that can submit this webform'),
    '#description' => t('Uncheck all roles to prevent new submissions. The %authenticated role applies to any user signed into the site, regardless of other assigned roles.', array('%authenticated' => $user_roles[2])),
  );
  /* End per-role submission control */

  /* Start E-mail Settings Form */
  $form['mail_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mail settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -2,
    '#theme' => 'webform_mail_settings_form',
  );

  $form['mail_settings']['email'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail to address'),
    '#maxlength' => 255,
    '#default_value' => $node->webform['email'],
    '#description' => t('Form submissions will be e-mailed to this address. Leave blank for none. Multiple e-mail addresses may be separated by commas.'),
  );

  $form['mail_settings']['email_components'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#theme' => 'webform_mail_components_form',
    '#title' => t('Conditional e-mail recipients'),
    '#description' => t('The settings below allow you to send e-mails to multiple recipients based off the value of a component.'),
    '#node' => $node,
  );

  $options = _webform_component_options($node->webform['components'], 'email');
  $default_value = array();
  if (is_array($node->webform['components'])) {
    foreach ($node->webform['components'] as $cid => $component) {
      if (isset($component['extra']['email']) && $component['extra']['email']) {
        $default_value[] = $cid;
      }
    }
  }
  $form['mail_settings']['email_components']['email_components'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $default_value,
    '#parents' => array('email_components'),
  );

  foreach (array('from_name', 'from_address', 'subject') as $field) {
    switch ($field) {
      case 'from_name':
        $default_value = _webform_filter_values(webform_variable_get('webform_default_from_name'), $node);
        $title = t('E-mail from name');
        $description = t('After adding components to this form any email, select, or hidden form element may be selected as the sender\'s name for e-mails.');
        break;
      case 'from_address':
        $default_value = _webform_filter_values(webform_variable_get('webform_default_from_address'), $node);
        $title = t('E-mail from address');
        $description = t('After adding components to this form any textfield, select, or hidden form element may be selected as the sender\'s e-mail address.');
        break;
      case 'subject':
        $default_value = _webform_filter_values(webform_variable_get('webform_default_subject'), $node);
        $title = t('E-mail subject');
        $description = t('After adding components to this form any textfield, select, or hidden form element may be selected as the subject for e-mails.');
        break;
    }

    $form['mail_settings']['email_'. $field .'_option'] = array(
      '#title' => $title,
      '#type' => 'radios',
      '#default_value' => is_numeric($node->webform['email_'. $field]) ? 'component' : ((empty($default_value) || ($node->webform['email_'. $field] != 'default' && isset($node->webform['email_'. $field]))) ? 'custom' : 'default'),
      '#description' => $description,
    );
    if (!empty($default_value)) {
      $form['mail_settings']['email_'. $field .'_option']['#options']['default'] = $default_value;
    }
    $form['mail_settings']['email_'. $field .'_option']['#options']['custom'] = 'custom';
    $form['mail_settings']['email_'. $field .'_option']['#options']['component'] = 'component';

    $form['mail_settings']['email_'. $field .'_custom'] = array(
      '#type' => 'textfield',
      '#size' => 40,
      '#default_value' => (!is_numeric($node->webform['email_'. $field]) && $node->webform['email_'. $field] != 'default') ? $node->webform['email_'. $field] : NULL,
    );
    $options = _webform_component_options($node->webform['components'], $field == 'from_address' ? 'email' : 'string');
    $form['mail_settings']['email_'. $field .'_component'] = array(
      '#type' => 'select',
      '#default_value' =>  is_numeric($node->webform['email_'. $field]) ? $node->webform['email_'. $field] : NULL,
      '#options' => empty($options) ? array('' => 'No available components') : $options,
      '#disabled' => empty($options) ? TRUE : FALSE,
      '#weight' => 6,
    );
  }
  /* End mail settings form */

  /* Start advanced settings form */
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => -1,
  );
  $form['advanced']['teaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show complete form in teaser'),
    '#default_value' => $node->webform['teaser'],
    '#description' => t('Display the entire form in the teaser display of this node.'),
  );
  $form['advanced']['submit_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Submit button text'),
    '#default_value' => $node->webform['submit_text'],
    '#description' => t('By default the submit button on this form will have the label <em>Submit</em>. Enter a new title here to override the default.'),
  );
  if (user_access('use PHP for additional processing')) {
    $form['advanced']['additional_validate'] = array(
      '#type' => 'textarea',
      '#title' => t('Additional Validation'),
      '#description' => t('Enter PHP code to perform additional validation for this form. Include the &lt;?php ?&gt; tags. $form and $form_state are available variables. If validation fails, use the form_set_error function to prevent the form from being submitted. Use the same syntax as a _validate function used in the <a href="http://api.drupal.org/api/file/developer/topics/forms_api.html">Forms API</a>.'),
      '#default_value' => $node->webform['additional_validate'],
      '#cols' => 40,
      '#rows' => 10,
    );

    $form['advanced']['additional_submit'] = array(
      '#type' => 'textarea',
      '#title' => t('Additional Processing'),
      '#description' => t('Enter PHP code to perform additional processing for this form (after the validation). Include the &lt;?php ?&gt; tags. $form and $form_state are available variables, use the same syntax as a _submit function used in the <a href="http://api.drupal.org/api/file/developer/topics/forms_api.html">Forms API</a>.'),
      '#default_value' => $node->webform['additional_submit'],
      '#cols' => 40,
      '#rows' => 10,
    );
  }
  else {
    $form['advanced']['additional_validate'] = array(
      '#type' => 'value',
      '#value' => $node->webform['additional_validate'],
    );

    $form['advanced']['additional_submit'] = array(
      '#type' => 'value',
      '#value' => $node->webform['additional_submit'],
    );
  }
  /* End Advanced Settings Form */

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 300,
  );

  return $form;
}

/**
 * Validate handler for webform_configure_form().
 */
function webform_configure_form_validate($form, &$form_state) {
  // Ensure the entered e-mail addresses are valid.
  if (!empty($form_state['values']['email'])) {
    $emails = explode(',', $form_state['values']['email']);
    foreach ($emails as $email) {
      if (!valid_email_address(trim($email))) {
        form_set_error('email', t('The entered email address %address is not a valid address.', array('%address' => $email)));
        break;
      }
    }
  }
  if ($form_state['values']['email_from_address_option'] == 'custom') {
    if (!valid_email_address($form_state['values']['email_from_address_custom'])) {
      form_set_error('email_from_address_custom', t('The entered email address %address is not a valid address.', array('%address' => $node->webform['email_from_address_custom'])));
    }
  }
}

/**
 * Submit handler for webform_configure_form().
 */
function webform_configure_form_submit($form, &$form_state) {
  $node = node_load($form_state['values']['nid']);

  // Save the confirmation.
  $node->webform['confirmation'] = $form_state['values']['confirmation'];
  $node->webform['confirmation_format'] = $form_state['values']['confirmation_format'];

  // Save roles.
  $node->webform['roles'] = $form_state['values']['roles'];

  // Save e-mail.
  $node->webform['email'] = $form_state['values']['email'];

  // Add the conditional e-mail recipients to components.
  foreach ($form_state['values']['email_components'] as $cid => $checked) {
    $node->webform['components'][$cid]['extra']['email'] = $checked;
  }

  // Merge the e-mail name, address, and subject options into single values.
  foreach (array('from_name', 'from_address', 'subject') as $field) {
    $option = $form_state['values']['email_'. $field .'_option'];
    if ($option == 'default') {
      $node->webform['email_'. $field] = 'default';
    }
    else {
      $node->webform['email_'. $field] = $form_state['values']['email_'. $field .'_'. $option];
    }
  }

  // Set the Show complete form in teaser setting.
  $node->webform['teaser'] = $form_state['values']['teaser'];

  // Set the submit limit to -1 if set to unlimited.
  if ($form_state['values']['enforce_limit'] == 'no') {
    $node->webform['submit_limit'] = -1;
    $node->webform['submit_interval'] = -1;
  }
  else {
    $node->webform['submit_limit'] = $form_state['values']['submit_limit'];
    $node->webform['submit_interval'] = $form_state['values']['submit_interval'];
  }

  // Set submit button text.
  $node->webform['submit_text'] = $form_state['values']['submit_text'];

  // Set additional validation and submission code.
  $node->webform['additional_validate'] = $form_state['values']['additional_validate'];
  $node->webform['additional_submit'] = $form_state['values']['additional_submit'];

  node_save($node);
}

/**
 * Theme the component options for sending e-mails.
 */
function theme_webform_mail_components_form($form) {
  drupal_add_css(drupal_get_path('module', 'webform') .'/webform.css');
  $node = $form['#node'];
  $header = array(
    array('data' => t('To'), 'class' => 'webform-checkbox'),
    t('Name'),
    t('Type'),
  );
  $rows = array();
  foreach (element_children($form['email_components']) as $cid) {
    $title = $form['email_components'][$cid]['#title'];
    unset($form['email_components'][$cid]['#title']);
    $rows[] = array(
      array('data' => drupal_render($form['email_components'][$cid]), 'class' => 'webform-checkbox'),
      $title,
      $node->webform['components'][$cid]['type'],
    );
  }
  if (empty($rows)) {
    $rows[] = array(array('colspan' => 5, 'data' => t('No components yet in this webform.')));
  }

  $form['#children'] = theme('table', $header, $rows);
  return drupal_render($form);
}

/**
 * Theme the Webform mail settings section of the node form.
 */
function theme_webform_mail_settings_form($form) {
  drupal_add_js(drupal_get_path('module', 'webform') .'/webform.js');

  // Loop through fields, rendering them into radio button options.
  foreach (array('from_name', 'from_address', 'subject') as $field) {
    foreach (array('custom' => t('Custom'), 'component' => t('Component')) as $option => $title) {
      $form['email_'. $field .'_'. $option]['#attributes']['class'] = 'webform-set-active';
      $form['email_'. $field .'_option'][$option]['#title'] = $title .': '. drupal_render($form['email_'. $field .'_'. $option]);
    }
    // For spacing consitency, every option is wrapped in container-inline.
    foreach (element_children($form['email_'. $field .'_option']) as $option) {
      $form['email_'. $field .'_option'][$option]['#prefix'] = '<div class="container-inline">';
      $form['email_'. $field .'_option'][$option]['#suffix'] = '</div>';
    }
    // Wrap the default option in a placeholder tag..
    if (isset($form['email_'. $field .'_option']['#options']['default'])) {
      $form['email_'. $field .'_option']['default']['#title'] = t('Default') .': '. theme('placeholder', $form['email_'. $field .'_option']['default']['#title']);
    }
  }

  return drupal_render($form);
}

/**
 * Theme the submit limit fieldset on the webform node form.
 */
function theme_webform_advanced_submit_limit_form($form) {
  $form['submit_limit']['#attributes']['class'] = 'webform-set-active';
  $form['submit_interval']['#attributes']['class'] = 'webform-set-active';
  $replacements = array(
    '!count' => drupal_render($form['submit_limit']),
    '!timespan' => drupal_render($form['submit_interval']),
  );
  $form['enforce_limit']['no']['#prefix'] = '<div class="container-inline">';
  $form['enforce_limit']['no']['#suffix'] = '</div>';
  $form['enforce_limit']['yes']['#prefix'] = '<div class="container-inline">';
  $form['enforce_limit']['yes']['#suffix'] = '</div>';
  $form['enforce_limit']['yes']['#title'] = t('Limit to !count submission(s) !timespan', $replacements);
  return drupal_render($form);
}
