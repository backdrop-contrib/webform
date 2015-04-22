WEBFORM
===================

CONTENTS OF THIS FILE
---------------------

 - Introduction
 - Tested
 - Known Issues
 - Special Thanks
 - Requirements
 - Installation
 - Coming From Drupal?
 - Usage
 - License
 - Credits
 - Maintainers

INTRODUCTION
------------

This module adds a webform content type to your Backdrop site.

A webform can be a questionnaire, contact or request form. These can be used by visitor to make contact or to enable a more complex survey than polls provide. Submissions from a webform are saved in a database table and can optionally be mailed to e-mail addresses upon submission.

Some good examples could be contests, personalized contact forms, or petitions. Each of these could have a customized form for end-users to fill out.

If you need to build a lot of customized, one-off forms, Webform is a more suitable solution than creating content types and using CCK or Field module.

TESTED
-----

Created several webforms with random options for admin and anonymous users for Backdrop 1.x.  Will need heavy user testing to make sure all the options of this premier module work as expected.

KNOWN ISSUES
---------------------

This line exists:
if (config_get('webform.settings', 'webform_install_create_content_type'))

in file webform.install

Because of https://github.com/backdrop-contrib/webform/issues/1
and
https://www.drupal.org/node/1263584

SPECIAL THANKS
--------------

This Backdrop module is supported by the hosted survey-solution website Webform.com and Lullabot.


REQUIREMENTS
------------

none

INSTALLATION
------------

Install this module using the official Backdrop CMS instructions at https://backdropcms.org/guide/modules

Edit the settings under "Administer" -> "Configuration" -> "Content authoring" -> "Webform settings"

COMING FROM DRUPAL?
-------------------

Nothing substantially different.

PERMISSIONS
------------

@todo


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

LICENSE
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.

CREDITS
-----------

This module is based on the Webform module for Drupal, originally written and maintained by a large number of contributors, including:

- Quicksketch <https://www.drupal.org/u/quicksketch>

Current Maintainers on Drupal:

- fenstrat <https://www.drupal.org/u/fenstrat>
- danchadwick <https://www.drupal.org/u/danchadwick>
- torotil <https://www.drupal.org/u/torotil>

MAINTAINERS
-----------

- Quicksketch <https://www.drupal.org/u/quicksketch>

Ported to Backdrop by:

 - biolithic <https://github.com/biolithic>
