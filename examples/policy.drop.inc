<?php

/**
 * @file
 *   Example policy commandfile. Modify as desired.
 *
 *   Validates commands as they are issued and returns an error 
 *   or changes options when policy is violated.
 *
 *   You can copy this file to any of the following
 *     1. A .drop folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/drop/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 */

/**
 * Implement of drop_hook_COMMAND_validate().
 * 
 * Prevent catastrophic braino. Note that this file has to be local to the machine
 * that intitiates sql-sync command.
 */
function drop_policy_sql_sync_validate($source = NULL, $destination = NULL) {
  if ($destination == '@prod') {
    return drop_set_error(dt('Per examples/policy.drop.inc, you may never overwrite the production database.'));
  }
}

/**
 * Implement of drop_hook_COMMAND_validate().
 *
 * To test this example without copying, execute `drop --include=./examples updatedb`
 * from within your drop directory.
 * 
 * Unauthorized users may view pending updates but not execute them.
 */
function drop_policy_updatedb_validate() {
  // Check for a token in the request. In this case, we require --token=secret.
  if (!drop_get_option('token') == 'secret') {
    drop_log(dt('Per site policy, you must add a secret --token complete this command. See examples/policy.drop.inc.  If you are running a version of drop prior to 4.3 and are not sure why you are seeing this message, please see http://backdrop.org/node/1024824.'), 'warning');
    drop_set_context('DROP_AFFIRMATIVE', FALSE);
    drop_set_context('DROP_NEGATIVE', TRUE);
  }
}

/**
 * Implementation of drop_hook_COMMAND_validate().
 * 
 * Only sudo tells me to make a sandwich: http://xkcd.com/149/
 */
function drop_policy_make_me_a_sandwich_validate() {
  $name = posix_getpwuid(posix_geteuid());
  if ($name['name'] !== 'root') {
    return drop_set_error('MAKE_IT_YOUSELF', dt('What? Make your own sandwich.'));
  }
}


