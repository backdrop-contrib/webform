<?php
/**
 * The Excel exporter currently is just a tab-delimited export.
 */
class webform_exporter_excel_delimited extends webform_exporter_delimited {
  function __construct($options) {
    $options['delimiter'] = '\t';
    parent::__construct($options);
  }

  function set_headers($filename) {
    drupal_add_http_header('Content-Type', 'application/x-msexcel');
    drupal_add_http_header('Content-Disposition', "attachment; filename=$filename.xls");
    drupal_add_http_header('Pragma', 'public');
    drupal_add_http_header('Cache-Control', 'max-age=0');
  }
}
