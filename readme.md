WEBFORM
===========

CONTENTS OF THIS FILE
---------------------

 - Introduction
 - Requirements
 - Installation
 - Permissions
 - Usage
 - Sponsors

INTRODUCTION
------------

This module adds a webform content type to your Drupal site.
A webform can be a questionnaire, contact or request form. These can be used
by visitor to make contact or to enable a more complex survey than polls
provide. Submissions from a webform are saved in a database table and
can optionally be mailed to e-mail addresses upon submission.

Some good examples could be contests, personalized contact forms, or petitions. Each of these could have a customized form for end-users to fill out. If you need to build a lot of customized, one-off forms, Webform is a more suitable solution than creating content types and using CCK or Field module.


TESTED
-----

Created several webforms with random options for admin and anonymous users for Backdrop 1.x.  Will need heavy user testing to make sure all the options of this premier module work as expected.


KNOWN ISSUES
---------------------

None yet.


REQUIREMENTS
------------

No special requirements.

INSTALLATION
------------

 Install as you would normally install a contributed drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.

1. Copy the entire webform directory the Drupal sites/all/modules directory.

2. Login as an administrator. Enable the module in the "Administer" -> "Functionality"

3. (Optional) Edit the settings under "Administer" -> "Configuration" ->
   "Content authoring" -> "Webform settings"


Upgrading from previous versions
--------------------------------
Note that you must be running the latest 3.x version of Webform (for either
Drupal 6 or Drupal 7) before upgrading to Webform 4.x.

If you have contributed modules, custom modules, or theming on your Webforms,
please read over the documentation for upgrading your code for Webform 4.x at
https://drupal.org/node/1609324.

1. MAKE A DATABASE BACKUP. Upgrading to Webform 4.x makes a signficant number of
   database changes. If you encounter an error and need to downgrade, you must
   restore the previous database. You can make a database backup with your
   hosting provider, using the Backup and Migrate module, or from the command
   line.

2. Copy the entire webform directory the Drupal modules directory, replacing the
   old copy of Webform. DO NOT KEEP THE OLD COPY in the same directory or
   anywhere Drupal could possibily find it. Delete it from the server.

3. Login as an administrative user or change the $update_free_access in
   update.php to TRUE.

4. Run update.php (at http://www.example.com/update.php).


PERMISSIONS
------------
@TODO

USAGE
-----

* Create a webform node at node/add/webform.

Support
-------
Please use the issue queue for filing bugs with this module at
http://drupal.org/project/issues/webform

Webform and you
Webform is second most popular non-utility contrib module (after Views). It is a large, rich module used by hundreds of thousands of sites. If you are using Webform, please give back to our community. We need your help triaging issues, answering support requests, writing patches, reviewing/testing patches.

Please always use the project issue tracker to report bugs, request support and/or request new features. The maintainers neither read the forums nor reply to direct e-mails for support.

Open one issue for each problem/request; don't bundle several issues into one submission. Please search the queue and read the handbook pages to avoid duplicate issues.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Maintainers
-----------

- Quicksketch
<https://www.drupal.org/u/quicksketch>

Current Maintainers on Drupal:

- fenstrat <https://www.drupal.org/u/fenstrat>
- danchadwick <https://www.drupal.org/u/danchadwick>
- torotil <https://www.drupal.org/u/torotil>

Supporting organizations:
This Drupal module is supported by the hosted survey-solution website Webform.com and Lullabot.

Ported to Backdrop by:
 - biolithic <https://github.com/biolithic>
