<?php

/**
 * @file
 *   Example policy commandfile. Modify as desired.
 *
 *   Validates commands as they are issued and returns an error 
 *   or changes options when policy is violated.
 *
 *   You can copy this file to any of the following
 *     1. A .brush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/brush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 */

/**
 * Implement of brush_hook_COMMAND_validate().
 * 
 * Prevent catastrophic braino. Note that this file has to be local to the machine
 * that intitiates sql-sync command.
 */
function brush_policy_sql_sync_validate($source = NULL, $destination = NULL) {
  if ($destination == '@prod') {
    return brush_set_error(dt('Per examples/policy.brush.inc, you may never overwrite the production database.'));
  }
}

/**
 * Implement of brush_hook_COMMAND_validate().
 *
 * To test this example without copying, execute `brush --include=./examples updatedb`
 * from within your brush directory.
 * 
 * Unauthorized users may view pending updates but not execute them.
 */
function brush_policy_updatedb_validate() {
  // Check for a token in the request. In this case, we require --token=secret.
  if (!brush_get_option('token') == 'secret') {
    brush_log(dt('Per site policy, you must add a secret --token complete this command. See examples/policy.brush.inc.  If you are running a version of brush prior to 4.3 and are not sure why you are seeing this message, please see http://drupal.org/node/1024824.'), 'warning');
    brush_set_context('BRUSH_AFFIRMATIVE', FALSE);
    brush_set_context('BRUSH_NEGATIVE', TRUE);
  }
}

/**
 * Implementation of brush_hook_COMMAND_validate().
 * 
 * Only sudo tells me to make a sandwich: http://xkcd.com/149/
 */
function brush_policy_make_me_a_sandwich_validate() {
  $name = posix_getpwuid(posix_geteuid());
  if ($name['name'] !== 'root') {
    return brush_set_error('MAKE_IT_YOUSELF', dt('What? Make your own sandwich.'));
  }
}


