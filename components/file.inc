<?php
// $Id$
/**
 * Create a set of form items to be displayed on the form for editing this component.
 * Use care naming the form items, as this correlates directly to the database schema.
 * The component "Name" and "Description" fields are added to every component type and
 * are not necessary to specify here (although they may be overridden if desired).
 * @return
 *   An array of form items to be displayed on the edit component page
 */
function _webform_edit_file($currfield) {
    $edit_fields = array();
    $edit_fields['#theme'] = 'webform_edit_file';
    $edit_fields['extra']['filtering'] = array(
      '#type' => 'fieldset',
      '#collapsible' => true,
      '#collapsed' => false,
      '#title' => t('Upload Filtering'),
      '#description' => t('Select the types of uploads you would like to allow.'),
    );
    $edit_fields['extra']['filtering']['types']['webimages'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Web Images'),
      '#options' => drupal_map_assoc(array('gif', 'jpg', 'png')),
      '#default_value' => isset($currfield['extra']['filtering']) ? $currfield['extra']['filtering']['types']['webimages'] : array('jpg', 'gif', 'png'),
    );
    $edit_fields['extra']['filtering']['types']['desktopimages'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Desktop Images'),
      '#options' => drupal_map_assoc(array('bmp', 'eps', 'tif', 'pict', 'psd')),
      '#default_value' => $currfield['extra']['filtering']['types']['desktopimages'],
    );
    $edit_fields['extra']['filtering']['types']['documents'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Documents'),
      '#options' => drupal_map_assoc(array('txt', 'rtf', 'html', 'odf', 'pdf', 'doc', 'ppt', 'xls', 'xml')),
      '#default_value' => $currfield['extra']['filtering']['types']['documents'],
    );
    $edit_fields['extra']['filtering']['types']['media'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Media'),
      '#options' => drupal_map_assoc(array('avi', 'mov', 'mp3', 'ogg', 'wav')),
      '#default_value' => $currfield['extra']['filtering']['types']['media'],
    );
    $edit_fields['extra']['filtering']['types']['archives'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Archives'),
      '#options' => drupal_map_assoc(array('bz2', 'dmg', 'gz', 'jar', 'rar', 'sit', 'tar', 'zip')),
      '#default_value' => $currfield['extra']['filtering']['types']['archives'],
    );
    $edit_fields['extra']['filtering']['addextensions'] = array(
      '#type' => 'textfield',
      '#title' => t("Additional Extensions"),
      '#default_value' => $currfield['extra']['filtering']['addextensions'],
      '#description' => t('Enter a list of additional file extensions for this upload field, seperated by commas.<br /> Entered extensions will be appended to checked items above.'),
      '#size' => 60,
      '#weight' => 3,
      '#default_value' => $currfield['extra']['filtering']['addextensions'],
    );
    $edit_fields['extra']['filtering']['size'] = array(
      '#type' => 'textfield',
      '#title' => t("Max Upload Size"),
      '#default_value' => $currfield['extra']['filtering']['size'],
      '#description' => t('Enter the max file size a user may upload (in KB).'),
      '#size' => 10,
      '#weight' => 3,
      '#default_value' => strlen($currfield['extra']['filtering']['size']) == 0 ? 800 : $currfield['extra']['filtering']['size'],
    );
    $edit_fields['extra']['savelocation'] = array(
      '#type' => 'textfield',
      '#title' => t("Upload Directory"),
      '#default_value' => $currfield['extra']['savelocation'],
      '#description' => "<div style='display: block'>". t('Webform uploads are always saved in the site files directory. You may optionally specify a subfolder to store your files.') ."</div>",
      '#weight' => 3,
      '#validate' => array('_webform_edit_file_check_directory'), // TODO: Get this validation working.
      '#after_build' => array('_webform_edit_file_check_directory'),
    );
    return $edit_fields;
}

function _webform_edit_file_check_directory($form_element) {
  $destination_dir = variable_get('file_directory_path', 'files') ."/webform/". $form_element['#value'];
  file_check_directory($destination_dir, FILE_CREATE_DIRECTORY, 'savelocation');
  return $form_element;
}

function theme_webform_edit_file($form) {
  // Add a little javascript to check all the items in one type.
  $javascript = '
    <script type="text/javascript">
      function check_category_boxes () {
        var checkValue = !document.getElementById("edit-extra-filtering-types-"+arguments[0]+"-"+arguments[1]).checked;
        for(var i=1; i < arguments.length; i++) {
          document.getElementById("edit-extra-filtering-types-"+arguments[0]+"-"+arguments[i]).checked = checkValue;
        }
      }
  </script>
 ';
  drupal_set_html_head($javascript);

  // Format the components into a table.
  $per_row = 5;
  $rows = array();
  foreach (element_children($form['extra']['filtering']['types']) as $key => $filtergroup) {
    $row = array();
    $first_row = count($rows);
    if ($form['extra']['filtering']['types'][$filtergroup]['#type'] == 'checkboxes') {
      // Add the title.
      $row[] = $form['extra']['filtering']['types'][$filtergroup]['#title'];
      $row[] = "&nbsp;";
      // Convert the checkboxes into individual form-items.
      $checkboxes = expand_checkboxes($form['extra']['filtering']['types'][$filtergroup]);
      // Render the checkboxes in two rows.
      $checkcount = 0;
      $jsboxes = "";
      foreach ($checkboxes as $key => $checkbox) {
        if ($checkbox['#type'] == 'checkbox') {
          $checkcount++;
          $jsboxes .= "'". $checkbox['#return_value'] ."',";
          if ($checkcount <= $per_row) {
            $row[] = array('data' => drupal_render($checkbox));
          }
          elseif ($checkcount == $per_row + 1) {
            $rows[] = array('data' => $row, 'style' => 'border-bottom: none;');
            $row = array(array('data' => '&nbsp;'), array('data' => '&nbsp;'));
            $row[] = array('data' => drupal_render($checkbox));
          }
          else {
            $row[] = array('data' => drupal_render($checkbox));
          }
        }
      }
      // Pretty up the table a little bit.
      $current_cell = $checkcount % $per_row;
      if ($current_cell > 0) {
        $colspan = $per_row - $current_cell   + 1;
        $row[$current_cell + 1]['colspan'] = $colspan;
      }
      // Add the javascript links.
      $jsboxes = substr($jsboxes, 0, strlen($jsboxes) - 1);
      $rows[] = array('data' => $row);
      $select_link = ' <a href="javascript:check_category_boxes(\''. $filtergroup .'\','. $jsboxes .')">(select)</a>';
      $rows[$first_row]['data'][1] = array('data' => $select_link, 'width' => 40);
      unset($form['extra']['filtering']['types'][$filtergroup]);
    }
    elseif ($filtergroup != 'size') {
      // Add other fields to the table (ie. additional extensions).
      $row[] = $form['extra']['filtering']['types'][$filtergroup]['#title'];
      unset($form['extra']['filtering']['types'][$filtergroup]['#title']);
      $row[] = array(
        'data' => drupal_render($form['extra']['filtering']['types'][$filtergroup]),
        'colspan' => $per_row + 1,
      );
      unset($form['extra']['filtering']['types'][$filtergroup]);
      $rows[] = array('data' => $row);
    }
  }
  $header = array(array('data' => t('Category'), 'colspan' => '2'), array('data' => t('Types'), 'colspan' => $per_row));

  // Create the table inside the form.
  $form['extra']['filtering']['types']['table'] = array(
    '#value' => theme('table', $header, $rows)
  );

  $output = drupal_render($form);

  // Prefix the upload location field with the default path for webform.
  $output = str_replace("Upload Directory: </label>", "Upload Directory: </label>". variable_get('file_directory_path', 'files') ."/webform/", $output);

  return $output;
}

/**
 * Build a form item array containing all the properties of this component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of a form item to be displayed on the client-side webform.
 */
function _webform_render_file($component) {
  $form_item = array(
    '#type'          => $component['type'],
    '#title'         => $component['name'],
    //'#required'      => $component['mandatory'], // Drupal core bug with required file uploads
    '#weight'        => $component['weight'],
    '#description'   => _webform_filtervalues($component['extra']['description']),
    '#attributes'    => $component['extra']['attributes'],
    '#tree'          => false, // file_check_upload assumes a flat $_FILES structure.
    '#validate'      => array(
      '_webform_validate_file' => array($component['form_key'], $component['name'], $component['extra']['filtering']),
      '_webform_required_file' => array($component['form_key'], $component['name'], $component['mandatory']), // Custom required routine.
    ),
    '#prefix'        => '<div class="webform-component-'. $component['type'] .'" id="webform-component-'. $component['form_key'] .'">',
    '#suffix'        => '</div>',
  );

  return $form_item;
}

function _webform_required_file($formelement, $form_key, $fieldname, $required = false) {
  if (empty($_FILES['files']['name'][$form_key]) && $required) {
    form_set_error($fieldname, t('%field field is required.', array('%field' => $fieldname)));
  }
}

function _webform_validate_file($formelement, $form_key, $fieldname, $filters) {
  if (empty($_FILES['files']['name'][$form_key])) {
    return;
  }

  // Build a list of extensions.
  $extensions = array();
  if (is_array($filters['types'])) {
    foreach ($filters['types'] as $filtertype) {
      foreach ($filtertype as $extension) {
        $extensions[] = $extension;
      }
    }
  }
  $extras = str_replace(" ", "", $filters['addextensions']);
  if (!empty($extras)) {
    $extensions = array_merge($extensions, explode(",", $extras));
  }

  // Build a human readable list of extensions:
  if (count($extensions) > 1) {
    for ($n = 0; $n < count($extensions) - 1; $n++) {
      $extension_list .= $extensions[$n] .", ";
    }
    $extension_list .= "or ". $extensions[count($extensions)-1];
  }
  else {
    $extension_list = $extensions[0];
  }

  $dot = strrpos($_FILES['files']['name'][$form_key], '.');
  $extension = strtolower(substr($_FILES['files']['name'][$form_key], $dot+1));
  if (!in_array($extension, $extensions)) {
    form_set_error($form_key, t("Files with the '%ext' extension are not allowed, please upload a file with a %exts extension.", array('%ext' => $extension, '%exts' => $extension_list)));
  }

  // Now let's check the file size (limit is set in KB).
  if ($_FILES['files']['size'][$form_key] > $filters['size']*1024) {
    form_set_error($form_key, t("The file '%filename' is too large (%filesize KB). Please upload a file %maxsize KB or smaller.", array('%filename' => $_FILES['files']['name'][$form_key], '%filesize' => (int)($_FILES['files']['size'][$form_key]/1024), '%maxsize' => $filters['size'])));
  }
}

/**
 * Perform additional server-side processing on the submitted data, such as
 * managing an uploaded file.
 * @param $data
 *   The POST data associated with the component.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Nothing.
 */
function _webform_submit_file(&$data, $component) {
  $component['form_key'];
  $upload_dir = variable_get('file_directory_path', 'files') ."/webform/". $component['extra']['savelocation'];
  if (file_check_directory($upload_dir, FILE_CREATE_DIRECTORY)) {
    $file_saved = file_save_upload($component['form_key'], array(), $upload_dir);
    if (!$file_saved) {
      drupal_set_message(t("The uploaded file %filename was unable to be saved. The destination directory may not be writable.", array('%filename' => $data)), "error");
      $data = serialize(array('error' => 'Error while uploading file'));
    }
    else {
      $data = serialize((array)$file_saved);
    }
  }
  else {
    drupal_set_message(t("The uploaded file %filename was unable to be saved. The destination directory does not exist.", array('%filename' => $data), "error"));
  }
}


/**
 * Format the output of emailed data for this component
 *
 * @param mixed $data
 *   A string or array of the submitted data.
 * @param array $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output to be included in the email.
 */
function theme_webform_mail_file($data, $component) {
  $file = unserialize($data);
  $output = $component['name'] .": ". (!empty($file['filepath']) ? file_create_url($file['filepath']) : '') ."\n";
  return $output;
}

/**
 * Display the result of a file submission. The output of this function will be
 * displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_submission_display_file($data, $component, $enabled = false) {
  $filedata = unserialize($data['value'][0]);
  $form_item = _webform_render_file($component);
  $form_item['#type'] = 'textfield';
  $form_item['#default_value'] = empty($filedata['filepath']) ? $filedata['error'] : $filedata['filepath'];
  if ($filedata['filename']) {
    $form_item['#suffix'] = ' <a href="'. file_create_url($filedata['filepath']) .'">Download '. $filedata['filename'] .'</a></div>';
  }
  $form_item['#disabled'] = !$enabled;
  return $form_item;
}

/**
 * Module specific instance of hook_help().
 */
function _webform_help_file($section) {
  switch ($section) {
    case 'admin/settings/webform#file_description':
      $output = t("Allow users to submit files of the configured types.");
      break;
  }
  return $output;
}

/**
 * Module specific instance of hook_theme().
 */
function _webform_theme_file() {
  return array(
    'webform_edit_file' => array(
      'arguments' => array('form' => NULL),
    ),
    'webform_mail_file' => array(
      'arguments' => array('data' => NULL, 'component' => NULL),
    ),
  );
}

/**
 * Calculate and returns statistics about results for this component from all
 * submission to this webform. The output of this function will be displayed
 * under the "results" tab then "analysis".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema
 * @return
 *   An array of data rows, each containing a statistic for this component's
 *   submissions.
 */
function _webform_analysis_rows_file($component) {
  $query = 'SELECT data '.
    ' FROM {webform_submitted_data} '.
    ' WHERE nid = %d '.
    ' AND cid = %d';
  $nonblanks = 0;
  $submissions = 0;
  $wordcount = 0;

  $result = db_query($query, $component['nid'], $component['cid']);
  while ($data = db_fetch_array($result)) {
    $filedata = unserialize($data['data']);
    if ($filedata['filesize']) {
      $nonblanks++;
      $sizetotal += $filedata['filesize'];
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User uploaded file'), $nonblanks);
  $rows[2] = array(t('Average uploaded file size'), ($sizetotal !=0 ? (int)(($sizetotal/$nonblanks)/1024) ." KB" : '0'));
  return $rows;
}

/**
 * Return the result of this component's submission for display in a table. The
 * output of this function will be displayed under the "results" tab then "table".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema
 * @return
 *   Textual output formatted for human reading.
 */
function _webform_table_data_file($data) {
  $filedata = unserialize($data['value']['0']);
  if (!empty($filedata['filename'])) {
    $output = '<a href="'. base_path() . $filedata['filepath'] .'">'. $filedata['filename'] .'</a>';
    $output .= " (". (int)($filedata['filesize']/1024) ." KB)";
  }
  elseif (!empty($filedata['error'])) {
    $output = $filedata['error'];
  }
  else {
    $output = "";
  }
  return $output;
}


/**
 * Return the header information for this component to be displayed in a comma
 * seperated value file. The output of this function will be displayed under the
 * "results" tab then "download".
 * @param $component
 *   An array of information describing the component, directly correlating to
 *   the webform_component database schema.
 * @return
 *   An array of data to be displayed in the first three rows of a CSV file, not
 *   including either prefixed or trailing commas.
 */
function _webform_csv_headers_file($component) {
  $header = array();
  $header[0] = '';
  // Two columns in header.
  $header[1] = $component['name'] .'\,';
  $header[2] = 'Name\,Filesize (KB)';
  return $header;
}

/**
 * Return the result of a file submission. The output of this function will be
 * displayed under the "results" tab then "submissions".
 * @param $data
 *   An array of information containing the submission result, directly
 *   correlating to the webform_submitted_data database schema.
 * @return
 *   Textual output formatted for CSV, not including either prefixed or trailing
 *   commas.
 */
function _webform_csv_data_file($data) {
  $filedata = unserialize($data['value']['0']);
  return empty($filedata['filename']) ? '\,' : $filedata['filename'] .'\,'. (int)($filedata['filesize']/1024);
}

