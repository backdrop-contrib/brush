<?php

/**
 * @file
 * Documentation of the Brush API.
 *

/**
 * Declare a new command.
 */
function hook_brush_command() {
  // To learn more, run `brush topic docs-commands` and `brush topic docs-examplecommand`
}

/* All brush commands are invoked in a specific order, using
 * brush-made hooks, very similar to the Backdrop hook system. See brush_invoke()
 * for the actual implementation.
 *
 * For any commandfile named "hook", the following hooks are called, in
 * order, for the command "COMMAND":
 *
 * 0. brush_COMMAND_init()
 * 1. brush_hook_COMMAND_validate()
 * 2. brush_hook_pre_COMMAND()
 * 3. brush_hook_COMMAND()
 * 4. brush_hook_post_COMMAND()
 *
 * For example, here are the hook opportunities for a mysite.brush.inc file
 * that wants to hook into the `pm-download` command.
 *
 * 1. brush_mysite_pm_download_validate()
 * 2. brush_mysite_pre_pm_download()
 * 3. brush_mysite_pm_download()
 * 4. brush_mysite_post_pm_download()
 *
 * Note that the brush_COMMAND_init() hook is only for use by the
 * commandfile that defines the command.
 *
 * If any of hook function fails, either by calling brush_set_error
 * or by returning FALSE as its function result, then the rollback
 * mechanism is called.  To fail with an error, call brush_set_error:
 *
 *   return brush_set_error('MY_ERROR_CODE', dt('Error message.'));
 *
 * To allow the user to confirm or cancel a command, use brush_confirm
 * and brush_user_abort:
 *
 *   if (!brush_confirm(dt('Are you sure?'))) {
 *     return brush_user_abort();
 *   }
 *
 * The rollback mechanism will call, in reverse, all _rollback hooks.
 * The mysite command file can implement the following rollback hooks:
 *
 * 1. brush_mysite_post_pm_download_rollback()
 * 2. brush_mysite_pm_download_rollback()
 * 3. brush_mysite_pre_pm_download_rollback()
 * 4. brush_mysite_pm_download_validate_rollback()
 *
 * Before any command is called, hook_brush_init() is also called.
 * hook_brush_exit() is called at the very end of command invocation.
 *
 * @see includes/command.inc
 *
 * @see hook_brush_init()
 * @see brush_COMMAND_init()
 * @see brush_hook_COMMAND_validate()
 * @see brush_hook_pre_COMMAND()
 * @see brush_hook_COMMAND()
 * @see brush_hook_post_COMMAND()
 * @see brush_hook_post_COMMAND_rollback()
 * @see brush_hook_COMMAND_rollback()
 * @see brush_hook_pre_COMMAND_rollback()
 * @see brush_hook_COMMAND_validate_rollback()
 * @see hook_brush_exit()
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Take action before any command is run. Logging an error stops command execution.
 */
function hook_brush_init() {

}

/**
 * Initialize a command prior to validation.  If a command
 * needs to bootstrap to a higher level, this is best done in
 * the command init hook.  It is permisible to bootstrap in
 * any hook, but note that if bootstrapping adds more commandfiles
 * (*.brush.inc) to the commandfile list, the newly-added
 * commandfiles will not have any hooks called until the next
 * phase.  For example, a command that calls brush_bootstrap_max()
 * in brush_hook_COMMAND() would only permit commandfiles from
 * modules enabled in the site to participate in brush_hook_post_COMMAND()
 * hooks.
 */
function brush_COMMAND_init() {
  brush_bootstrap_max();
}

/**
 * Run before a specific command executes.
 *
 * Logging an error stops command execution, and the rollback function (if any)
 * for each hook implementation is invoked.
 *
 * @see brush_hook_COMMAND_validate_rollback()
 */
function brush_hook_COMMAND_validate() {

}

/**
 * Run before a specific command executes. Logging an error stops command execution.
 *
 * Logging an error stops command execution, and the rollback function (if any)
 * for each hook implementation is invoked, in addition to the
 * validate rollback.
 *
 * @see brush_hook_pre_COMMAND_rollback()
 * @see brush_hook_COMMAND_validate_rollback()
 */
function brush_hook_pre_COMMAND() {

}

/**
 * Implementation of the actual brush command.
 *
 * This is where most of the stuff should happen.
 *
 * Logging an error stops command execution, and the rollback function (if any)
 * for each hook implementation is invoked, in addition to pre and
 * validate rollbacks.
 *
 * @see brush_hook_COMMAND_rollback()
 * @see brush_hook_pre_COMMAND_rollback()
 * @see brush_hook_COMMAND_validate_rollback()
 */
function brush_hook_COMMAND() {

}

/**
 * Run after a specific command executes. Logging an error stops command execution.
 *
 * Logging an error stops command execution, and the rollback function (if any)
 * for each hook implementation is invoked, in addition to pre, normal
 * and validate rollbacks.
 *
 * @see brush_hook_post_COMMAND_rollback()
 * @see brush_hook_COMMAND_rollback()
 * @see brush_hook_pre_COMMAND_rollback()
 * @see brush_hook_COMMAND_validate_rollback()
 */
function brush_hook_post_COMMAND() {

}

/**
 * Take action after any command is run.
 */
function hook_brush_exit() {

}

/*
 * A commandfile may choose to decline to load for the current bootstrap
 * level by returning FALSE. This hook must be placed in MODULE.brush.load.inc.
 * @see brush_commandfile_list().
 */
function hook_brush_load() {

}

/**
 * Take action after a project has been downloaded.
 */
function hook_brush_pm_post_download($project, $release) {

}

/**
 * Take action after a project has been updated.
 */
function hook_pm_post_update($release_name, $release_candidate_version, $project_parent_path) {

}

/**
 * Adjust the location that a project should be copied to after being downloaded.
 *
 * See @pm_brush_pm_download_destination_alter().
 */
function hook_brush_pm_download_destination_alter(&$project, $release) {
  if ($some_condition) {
    $project['project_install_location'] = '/path/to/install/to/' . $project['project_dir'];
  }
}

/**
 * Add information to the upgrade project map; this information
 * will be shown to the user when upgrading Backdrop to the next
 * major version if the module containing this hook is enabled.
 *
 * @see brush_upgrade_project_map().
 */
function hook_brush_upgrade_project_map_alter(&$project_map) {
  $project_map['warning']['hook'] = dt("You need to take special action before upgrading this module. See http://mysite.com/mypage for more information.");
}

/**
 * Sql-sync sanitization example.  
 *
 * @see sql_brush_sql_sync_sanitize().
 */
function hook_brush_sql_sync_sanitize($source) {
  brush_sql_register_post_sync_op('my-sanitize-id',
    dt('Reset passwords and email addresses in user table'),
    "update users set pass = MD5('password'), mail = concat('user+', uid, '@localhost') where uid > 0;");
}

/**
 * Take action before modules are disabled in a major upgrade.
 * Note that when this hook fires, it will be operating on a
 * copy of the database.
 */
function brush_hook_pre_site_upgrade_prepare() {
  // site upgrade prepare will disable contrib_extensions and
  // uninstall the uninstall_extension
  $contrib_extensions = func_get_args();
  $uninstall_extensions = explode(',', brush_get_option('uninstall', ''));
}


/**
 * Add help components to a command
 */
function hook_brush_help_alter(&$command) {
  if ($command['command'] == 'sql-sync') {
    $command['options']['myoption'] = "Description of modification of sql-sync done by hook";
    $command['sub-options']['sanitize']['my-sanitize-option'] = "Description of sanitization option added by hook (grouped with --sanitize option)";
  }
}

/**
 * Add/edit options to cache-clear command
 */
function hook_brush_cache_clear(&$types) {
  $types['views'] = 'views_invalidate_cache';
}

/*
 * Make shell aliases and other .bashrc code available during core-cli command.
 *
 * @return
 *   Bash code typically found in a .bashrc file.
 *
 * @see core_cli_bashrc() for an example implementation.
 */
function hook_cli_bashrc() {
  $string = "
    alias siwef='brush site-install wef --account-name=super --account-mail=me@wef'
    alias dump='brush sql-dump --structure-tables-key=wef --ordered-dump'
  ";
  return $string;
}

/**
 * @} End of "addtogroup hooks".
 */
