<?php

/**
 * Base class defining the common methods available to exporters.
 */

/**
 *
 */
class webform_exporter {
  public $options = array();
  public $export_wordwrap;

  /**
   * Constructor for webform_exporter classes.
   *
   * @param $options
   *   The array of export options as provided by the user-interface.
   */
  public function __construct($options) {
    $this->options = $options;
    $this->export_wordwrap = webform_variable_get('webform_export_wordwrap');
  }

  /**
   * Determines whether a cell is eligible for word-wrapping.
   *
   * This is based upon position in file and the contents of cell.
   *
   * Return true when the global word-wrapping option is enabled and the cell
   * is anything other than the first column in either of the first two rows.
   * By default, these rows are long and are intended to overlap the columns
   * to the right. Also returns true when the cell contains a return character.
   *
   * @param int $row
   *   Row number, counting from 0.
   * @param int $column
   *   Column number, counting from 0.
   * @param string $value
   *   The value of the cell.
   *
   * @return bool
   *   Whether the cell position is eligible for wordwrapping.
   */
  public function wrappable($row, $column, $value) {
    return strpos($value, "\n") !== FALSE ||
      $this->export_wordwrap && ($row > 2 || $column > 0 || $this->options['header_keys'] < 0);
  }

  /**
   * Add a single row to the export file.
   *
   * @param $file_handle
   *   A PHP file handle to the export file.
   * @param array $data
   *   An array of formatted data for this row. One cell per item.
   * @param int $row_count
   *   The current number of rows in the export file.
   */
  public function add_row(&$file_handle, array $data, $row_count) {
  }

  /**
   * Provide headers to the page when an export file is being downloaded.
   *
   * @param string $filename
   *   The name of the file being downloaded, for example, export.xls.
   */
  public function set_headers($filename) {
    backdrop_add_http_header('Content-Type', 'application/force-download');
    backdrop_add_http_header('Pragma', 'public');
    backdrop_add_http_header('Cache-Control', 'max-age=0');
  }

  /**
   * Get full filename of the export file, including the extension.
   *
   * @param string $filename
   *   The base filename of the export file.
   *
   * @return string
   *   The full filename of the export file, including the extension.
   */
  public function get_filename($filename) {
    return $filename;
  }

  /**
   * Write the start of the export file.
   *
   * @param $file_handle
   *   A PHP file handle to the export file.
   */
  public function bof(&$file_handle) {
  }

  /**
   * Write the end of the export file.
   *
   * @param $file_handle
   *   A PHP file handle to the export file.
   * @param int $row_count
   * @param int $col_count
   */
  public function eof(&$file_handle, $row_count, $col_count) {
  }

  /**
   * Allow final processing of the results.
   *
   * @param $results
   *   An array of result data, including:
   *   - node: The node whose results are being downloaded.
   *   - file_name: The full file path of the generated file.
   *   - row_count: The final number of rows in the generated file.
   */
  public function post_process(&$results) {
  }

}
