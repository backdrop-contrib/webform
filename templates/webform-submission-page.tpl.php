<?php
// $Id$

/**
 * @file
 * Customize the navigation shown when editing or viewing submissions.
 *
 * Available variables:
 * - $node: The node object for this webform.
 * - $submission: The contents of the webform submission.
 * - $previous: The previous submission ID.
 * - $next: The next submission ID.
 * - $previous_url: The URL of the previous submission.
 * - $next_url: The URL of the next submission.
 * - $mode: Either "form" or "display". As the navigation is shown in both uses.
 */

drupal_add_css(drupal_get_path('module', 'webform') .'/webform.css', 'theme', 'all', FALSE);
?>

<?php print $submission_navigation; ?>
<?php print $submission_information; ?>

<?php print $submission; ?>

<?php print $submission_navigation; ?>
