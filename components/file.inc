<?php

/**
 * @file
 * Webform module file component.
 */

/**
 * Implements _webform_defaults_component().
 */
function _webform_defaults_file() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'mandatory' => 0,
    'pid' => 0,
    'weight' => 0,
    'extra' => array(
      'filtering' => array(
        'types' => array('gif', 'jpg', 'png'),
        'addextensions' => '',
        'size' => '2 MB',
      ),
      'scheme' => 'public',
      'directory' => '',
      'progress_indicator' => 'throbber',
      'title_display' => 0,
      'description' => '',
      'attributes' => array(),
      'private' => FALSE,
    ),
  );
}

/**
 * Implements _webform_theme_component().
 */
function _webform_theme_file() {
  return array(
    'webform_edit_file' => array(
      'render element' => 'form',
      'file' => 'components/file.inc',
    ),
    'webform_display_file' => array(
      'render element' => 'element',
      'file' => 'components/file.inc',
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_file($component) {
    $form = array();
    $form['#theme'] = 'webform_edit_file';
    $form['#element_validate'] = array('_webform_edit_file_check_directory');
    $form['#after_build'] = array('_webform_edit_file_check_directory');

    $form['validation']['filtering'] = array(
      '#element_validate' => array('_webform_edit_file_filtering_validate'),
      '#parents' => array('extra', 'filtering'),
    );

    // Find the list of all currently valid extensions.
    $current_types = isset($component['extra']['filtering']['types']) ? $component['extra']['filtering']['types'] : array();

    $types = array('gif', 'jpg', 'png');
    $form['validation']['filtering']['types']['webimages'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Web Images'),
      '#options' => drupal_map_assoc($types),
      '#default_value' => array_intersect($current_types, $types),
    );

    $types = array('bmp', 'eps', 'tif', 'pict', 'psd');
    $form['validation']['filtering']['types']['desktopimages'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Desktop Images'),
      '#options' => drupal_map_assoc($types),
      '#default_value' => array_intersect($current_types, $types),
    );

    $types = array('txt', 'rtf', 'html', 'odf', 'pdf', 'doc', 'docx', 'ppt', 'pptx', 'xls', 'xlsx', 'xml');
    $form['validation']['filtering']['types']['documents'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Documents'),
      '#options' => drupal_map_assoc($types),
      '#default_value' => array_intersect($current_types, $types),
    );

    $types = array('avi', 'mov', 'mp3', 'ogg', 'wav');
    $form['validation']['filtering']['types']['media'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Media'),
      '#options' => drupal_map_assoc($types),
      '#default_value' => array_intersect($current_types, $types),
    );

    $types = array('bz2', 'dmg', 'gz', 'jar', 'rar', 'sit', 'tar', 'zip');
    $form['validation']['filtering']['types']['archives'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Archives'),
      '#options' => drupal_map_assoc($types),
      '#default_value' => array_intersect($current_types, $types),
    );

    $form['validation']['filtering']['addextensions'] = array(
      '#type' => 'textfield',
      '#title' => t('Additional Extensions'),
      '#default_value' => $component['extra']['filtering']['addextensions'],
      '#description' => t('Enter a list of additional file extensions for this upload field, seperated by commas.<br /> Entered extensions will be appended to checked items above.'),
      '#size' => 60,
      '#weight' => 3,
      '#default_value' => $component['extra']['filtering']['addextensions'],
    );

    $form['validation']['filtering']['size'] = array(
      '#type' => 'textfield',
      '#title' => t('Max Upload Size'),
      '#default_value' => $component['extra']['filtering']['size'],
      '#description' => t('Enter the max file size a user may upload such as 2 MB or 800 KB. Your server has a max upload size of @size.', array('@size' => format_size(file_upload_max_size()))),
      '#size' => 10,
      '#weight' => 3,
      '#default_value' => $component['extra']['filtering']['size'],
      '#parents' => array('extra', 'filtering', 'size'),
      '#element_validate' => array('_webform_edit_file_size_validate'),
    );

    $scheme_options = array();
    foreach (file_get_stream_wrappers(STREAM_WRAPPERS_WRITE_VISIBLE) as $scheme => $stream_wrapper) {
      $scheme_options[$scheme] = $stream_wrapper['name'];
    }
    $form['extra']['scheme'] = array(
      '#type' => 'radios',
      '#title' => t('Upload destination'),
      '#options' => $scheme_options,
      '#default_value' => $component['extra']['scheme'],
      '#description' => t('Private file storage has significantly more overhead than public files, but restricts file access to users who can view submissions.'),
      '#weight' => 4,
      '#access' => count($scheme_options) > 1,
    );
    $form['extra']['directory'] = array(
      '#type' => 'textfield',
      '#title' => t('Upload directory'),
      '#default_value' => $component['extra']['directory'],
      '#description' => t('You may optionally specify a sub-directory to store your files.'),
      '#weight' => 5,
      '#field_prefix' => 'webform/',
    );

    $form['display']['progress_indicator'] = array(
      '#type' => 'radios',
      '#title' => t('Progress indicator'),
      '#options' => array(
        'throbber' => t('Throbber'),
        'bar' => t('Bar with progress meter'),
      ),
      '#default_value' => $component['extra']['progress_indicator'],
      '#description' => t('The throbber display does not show the status of uploads but takes up less space. The progress bar is helpful for monitoring progress on large uploads.'),
      '#weight' => 16,
      '#access' => file_progress_implementation(),
      '#parents' => array('extra', 'progress_indicator'),
    );

    // TODO: Make managed_file respect the "size" parameter.
    /*
    $form['display']['width'] = array(
      '#type' => 'textfield',
      '#title' => t('Width'),
      '#default_value' => $component['extra']['width'],
      '#description' => t('Width of the file field.') . ' ' . t('Leaving blank will use the default size.'),
      '#size' => 5,
      '#maxlength' => 10,
      '#weight' => 4,
      '#parents' => array('extra', 'width')
    );
    */

    return $form;
}

/**
 * A Form API element validate function to check filesize is valid.
 */
function _webform_edit_file_size_validate($element) {
  if (!empty($element['#value'])) {
    $set_filesize = parse_size($element['#value']);
    if ($set_filesize == FALSE) {
      form_error($element, t('File size @value is not a valid filesize. Use a value such as 2 MB or 800 KB.', array('@value' => $element['#value'])));
    }
    else {
      $max_filesize = parse_size(file_upload_max_size());
      if ($max_filesize < $set_filesize) {
        form_error($element, t('An upload size of @value is too large, you are allow to upload files @max or less.', array('@value' => $element['#value'], '@max' => format_size($max_filesize))));
      }
    }
  }
}

/**
 * A Form API after build and validate function.
 *
 * Ensure that the destination directory exists and is writable.
 */
function _webform_edit_file_check_directory($element) {
  $scheme = $element['extra']['scheme']['#value'];
  $directory = $element['extra']['directory']['#value'];

  $destination_dir = file_stream_wrapper_uri_normalize($scheme . '://' . $directory . '/webform');

  // Sanity check input to prevent use parent (../) directories.
  if (preg_match('/\.\.[\/\\\]/', $destination_dir . '/')) {
    form_error($element['extra']['directory'], t('The save directory %directory is not valid.', array('%directory' => $directory)));
  }
  else {
    $destination_success = file_prepare_directory($destination_dir, FILE_CREATE_DIRECTORY);
    if (!$destination_success) {
      form_error($element['extra']['directory'], t('The save directory %directory could not be created. Check that the webform files directory is writable.', array('%directory' => $directory)));
    }
  }

  return $element;
}

/**
 * A Form API element validate function.
 *
 * Change the submitted values of the component so that all filtering extensions
 * are saved as a single array.
 */
function _webform_edit_file_filtering_validate($element, &$form_state) {
  // Predefined types.
  $extensions = array();
  foreach (element_children($element['types']) as $category) {
    foreach (array_keys($element['types'][$category]['#value']) as $extension) {
      if ($element['types'][$category][$extension]['#value']) {
        $extensions[] = $extension;
      }
    }
  }

  // Additional types.
  $additional_extensions = explode(',', $element['addextensions']['#value']);
  foreach ($additional_extensions as $extension) {
    $clean_extension = drupal_strtolower(trim($extension));
    if (!empty($clean_extension) && !in_array($clean_extension, $extensions)) {
      $extensions[] = $clean_extension;
    }
  }

  form_set_value($element['types'], $extensions, $form_state);
}

function theme_webform_edit_file($variables) {
  $form = $variables['form'];

  // Add a little JavaScript to check all the items in one type.
  $javascript = '
    function check_category_boxes() {
      var checkValue = !document.getElementById("edit-extra-filtering-types-"+arguments[0]+"-"+arguments[1]).checked;
      for(var i=1; i < arguments.length; i++) {
        document.getElementById("edit-extra-filtering-types-"+arguments[0]+"-"+arguments[i]).checked = checkValue;
      }
    }
 ';
  drupal_add_js($javascript, 'inline');

  // Format the components into a table.
  $per_row = 6;
  $rows = array();
  foreach (element_children($form['validation']['filtering']['types']) as $key => $filtergroup) {
    $row = array();
    $first_row = count($rows);
    if ($form['validation']['filtering']['types'][$filtergroup]['#type'] == 'checkboxes') {
      // Add the title.
      $row[] = $form['validation']['filtering']['types'][$filtergroup]['#title'];
      $row[] = '&nbsp;';
      // Convert the checkboxes into individual form-items.
      $checkboxes = form_process_checkboxes($form['validation']['filtering']['types'][$filtergroup]);
      // Render the checkboxes in two rows.
      $checkcount = 0;
      $jsboxes = '';
      foreach (element_children($checkboxes) as $key) {
        $checkbox = $checkboxes[$key];
        if ($checkbox['#type'] == 'checkbox') {
          $checkcount++;
          $jsboxes .= "'" . $checkbox['#return_value'] . "',";
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
      $jsboxes = drupal_substr($jsboxes, 0, drupal_strlen($jsboxes) - 1);
      $rows[] = array('data' => $row);
      $select_link = ' <a href="javascript:check_category_boxes(\'' . $filtergroup . '\',' . $jsboxes . ')">(select)</a>';
      $rows[$first_row]['data'][1] = array('data' => $select_link, 'width' => 40);
      unset($form['validation']['filtering']['types'][$filtergroup]);
    }
    elseif ($filtergroup != 'size') {
      // Add other fields to the table (ie. additional extensions).
      $row[] = $form['validation']['filtering']['types'][$filtergroup]['#title'];
      unset($form['validation']['filtering']['types'][$filtergroup]['#title']);
      $row[] = array(
        'data' => drupal_render($form['validation']['filtering']['types'][$filtergroup]),
        'colspan' => $per_row + 1,
      );
      unset($form['validation']['filtering']['types'][$filtergroup]);
      $rows[] = array('data' => $row);
    }
  }
  $header = array(array('data' => t('Category'), 'colspan' => '2'), array('data' => t('Types'), 'colspan' => $per_row));

  // Create the table inside the form.
  $form['validation']['filtering']['types']['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows
  );

  $output = drupal_render_children($form);

  return $output;
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_file($component, $value = NULL, $filter = TRUE) {
  // Cap the upload size according to the PHP limit.
  $max_filesize = parse_size(file_upload_max_size());
  $set_filesize = $component['extra']['filtering']['size'];
  if (!empty($set_filesize) && parse_size($set_filesize) < $max_filesize) {
    $max_filesize = parse_size($set_filesize);
  }

  $element = array(
    '#type' => 'managed_file',
    '#title' => $filter ? _webform_filter_xss($component['name']) : $component['name'],
    '#title_display' => $component['extra']['title_display'] ? $component['extra']['title_display'] : 'before',
    '#required' => $component['mandatory'],
    '#default_value' => isset($value[0]) ? $value[0] : NULL,
    '#attributes' => $component['extra']['attributes'],
    '#upload_validators' => array(
      'file_validate_size' => array($max_filesize),
      'file_validate_extensions' => array(implode(' ', $component['extra']['filtering']['types'])),
    ),
    '#pre_render' => array_merge(element_info_property('managed_file', '#pre_render'), array('webform_file_allow_access')),
    '#upload_location' => $component['extra']['scheme'] . '://webform/' . $component['extra']['directory'],
    '#progress_indicator' => $component['extra']['progress_indicator'],
    '#description' => $filter ? _webform_filter_descriptions($component['extra']['description']) : $component['extra']['description'],
    '#weight' => $component['weight'],
    '#theme_wrappers' => array('webform_element'),
    '#translatable' => array('title', 'description'),
  );

  return $element;
}

/**
 * Pre-render callback to allow access to uploaded files.
 *
 * Files that have not yet been saved into a submission must be accessible to
 * the user who uploaded it, but no one else. After the submission is saved,
 * access is granted through the file_usage table. Before then, we use a
 * $_SESSION value to record a user's upload.
 *
 * @see webform_file_download()
 */
function webform_file_allow_access($element) {
  if (!empty($element['#value']['fid'])) {
    $fid = $element['#value']['fid'];
    $_SESSION['webform_files'][$fid] = $fid;
  }

  return $element;
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_file($component, $value, $format = 'html') {
  $fid = isset($value[0]) ? $value[0] : NULL;
  return array(
    '#title' => $component['name'],
    '#value' => $fid ? webform_get_file($fid) : NULL,
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_file',
    '#theme_wrappers' => $format == 'text' ? array('webform_element_text') : array('webform_element'),
    '#format' => $format,
    '#translatable' => array('title'),
  );
}

/**
 * Format the output of text data for this component
 */
function theme_webform_display_file($variables) {
  $element = $variables['element'];

  $file = $element['#value'];
  $url = !empty($file) ? webform_file_url($file->uri) : t('no upload');
  return !empty($file) ? ($element['#format'] == 'text' ? $url : l($file->filename, $url)) : ' ';
}

/**
 * Implements _webform_delete_component().
 */
function _webform_delete_file($component, $value) {
  // Delete an individual submission file.
  if (!empty($value[0]) && ($file = webform_get_file($value[0]))) {
    file_delete($file);
  }
}

/**
 * Implements _webform_attachments_component().
 */
function _webform_attachments_file($component, $value) {
  $file = (array) webform_get_file($value[0]);
  //This is necessary until the next release of mimemail is out, see [#1388786]
  $file['filepath'] = $file['uri'];
  $files = array($file);
  return $files;
}
/**
 * Implements _webform_analysis_component().
 */
function _webform_analysis_file($component, $sids = array()) {
  $query = db_select('webform_submitted_data', 'wsd', array('fetch' => PDO::FETCH_ASSOC))
    ->fields('wsd', array('no', 'data'))
    ->condition('nid', $component['nid'])
    ->condition('cid', $component['cid']);

  if (count($sids)) {
    $query->condition('sid', $sids, 'IN');
  }

  $nonblanks = 0;
  $sizetotal = 0;
  $submissions = 0;

  $result = $query->execute();
  foreach ($result as $data) {
    $file = webform_get_file($data['data']);
    if (isset($file->filesize)) {
      $nonblanks++;
      $sizetotal += $file->filesize;
    }
    $submissions++;
  }

  $rows[0] = array(t('Left Blank'), ($submissions - $nonblanks));
  $rows[1] = array(t('User uploaded file'), $nonblanks);
  $rows[2] = array(t('Average uploaded file size'), ($sizetotal != 0 ? (int) (($sizetotal/$nonblanks)/1024) . ' KB' : '0'));
  return $rows;
}

/**
 * Implements _webform_table_component().
 */
function _webform_table_file($component, $value) {
  $output = '';
  $file = webform_get_file($value[0]);
  if (!empty($file->fid)) {
    $output = '<a href="' . webform_file_url($file->uri) . '">' . check_plain(webform_file_name($file->uri)) . '</a>';
    $output .= ' (' . (int) ($file->filesize/1024) . ' KB)';
  }
  return $output;
}

/**
 * Implements _webform_csv_headers_component().
 */
function _webform_csv_headers_file($component, $export_options) {
  $header = array();
  // Two columns in header.
  $header[0] = array('', '');
  $header[1] = array($component['name'], '');
  $header[2] = array(t('Name'), t('Filesize (KB)'));
  return $header;
}

/**
 * Implements _webform_csv_data_component().
 */
function _webform_csv_data_file($component, $export_options, $value) {
  $file = webform_get_file($value[0]);
  return empty($file->filename) ? array('', '') : array(webform_file_url($file->uri), (int) ($file->filesize/1024));
}

/**
 * Helper function to create proper file names for uploaded file.
 */
function webform_file_name($filepath) {
  if (!empty($filepath)) {
    $info = pathinfo($filepath);
    $file_name = $info['basename'];
  }
  return isset($file_name) ? $file_name : '';
}

/**
 * Helper function to create proper URLs for uploaded file.
 */
function webform_file_url($uri) {
  if (!empty($uri)) {
    $file_url = file_create_url($uri);
  }
  return isset($file_url) ? $file_url : '';
}

/**
 * Helper function to load a file from the database.
 */
function webform_get_file($fid) {
  // Simple check to prevent loading of NULL values, which throws an entity
  // system error.
  return $fid ? file_load($fid) : FALSE;
}

/**
 * Given a submission with file_usage set, add or remove file usage entries.
 */
function webform_file_usage_adjust($submission) {
  if (isset($submission->file_usage)) {
    $files = file_load_multiple($submission->file_usage['added_fids']);
    foreach ($files as $file) {
      file_usage_add($file, 'webform', 'submission', $submission->sid);
    }

    $files = file_load_multiple($submission->file_usage['deleted_fids']);
    foreach ($files as $file) {
      file_usage_delete($file, 'webform', 'submission', $submission->sid);
      file_delete($file);
    }
  }
}

