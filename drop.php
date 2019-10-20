#!/usr/bin/env php
<?php

/**
 * @file
 * drop is a PHP script implementing a command line shell for Backdrop.
 *
 * @requires PHP CLI 5.2.0, or newer.
 */
// Terminate immediately unless invoked as a command line script
if (!drop_verify_cli()) {
  die('drop is designed to run via the command line.');
}

// Check supported version of PHP.
define('DROP_MINIMUM_PHP', '5.2.0');
if (version_compare(phpversion(), DROP_MINIMUM_PHP) < 0) {
  die('Your command line PHP installation is too old. Drop requires at least PHP ' . DROP_MINIMUM_PHP . "\n");
}

define('DROP_BASE_PATH', dirname(__FILE__));


define('DROP_REQUEST_TIME', microtime(TRUE));

require_once DROP_BASE_PATH . '/includes/environment.inc';
require_once DROP_BASE_PATH . '/includes/command.inc';
require_once DROP_BASE_PATH . '/includes/drop.inc';
require_once DROP_BASE_PATH . '/includes/backend.inc';
require_once DROP_BASE_PATH . '/includes/batch.inc';
require_once DROP_BASE_PATH . '/includes/context.inc';
require_once DROP_BASE_PATH . '/includes/sitealias.inc';

drop_set_context('argc', $GLOBALS['argc']);
drop_set_context('argv', $GLOBALS['argv']);

// Set an error handler and a shutdown function
set_error_handler('drop_error_handler');
register_shutdown_function('drop_shutdown');

exit(drop_main());

/**
 * Verify that we are running PHP through the command line interface.
 *
 * This function is useful for making sure that code cannot be run via the web server,
 * such as a function that needs to write files to which the web server should not have
 * access to.
 *
 * @return
 *   A boolean value that is true when PHP is being run through the command line,
 *   and false if being run through cgi or mod_php.
 */
function drop_verify_cli() {
  return (php_sapi_name() == 'cli' || (is_numeric($_SERVER['argc']) && $_SERVER['argc'] > 0));
}

/**
 * The main Drop function.
 *
 * - Parses the command line arguments, configuration files and environment.
 * - Prepares and executes a Backdrop bootstrap, if possible,
 * - Dispatches the given command.
 *
 * @return
 *   Whatever the given command returns.
 */
  function drop_main() {
  $phases = _drop_bootstrap_phases(FALSE, TRUE);
  drop_set_context('DROP_BOOTSTRAP_PHASE', DROP_BOOTSTRAP_NONE);

  // We need some global options processed at this early stage. Namely --debug.
  drop_parse_args();
  _drop_bootstrap_global_options();

  $return = '';
  $command_found = FALSE;

  foreach ($phases as $phase) {
    if (drop_bootstrap_to_phase($phase)) {
      $command = drop_parse_command();

      // Process a remote command if 'remote-host' option is set.
      if (drop_remote_command()) {
        $command_found = TRUE;
        break;
      }

      if (is_array($command)) {
        $bootstrap_result = drop_bootstrap_to_phase($command['bootstrap']);
        drop_enforce_requirement_bootstrap_phase($command);
        drop_enforce_requirement_core($command);
        drop_enforce_requirement_backdrop_dependencies($command);
        drop_enforce_requirement_drop_dependencies($command);

        if ($bootstrap_result && empty($command['bootstrap_errors'])) {
          drop_log(dt("Found command: !command (commandfile=!commandfile)", array('!command' => $command['command'], '!commandfile' => $command['commandfile'])), 'bootstrap');

          $command_found = TRUE;
          // Dispatch the command(s).
          $return = drop_dispatch($command);

          // prevent a '1' at the end of the output
          if ($return === TRUE) {
            $return = '';
          }

          if (drop_get_context('DROP_DEBUG') && !drop_get_context('DROP_QUIET')) {
            drop_print_timers();
          }
          drop_log(dt('Peak memory usage was !peak', array('!peak' => drop_format_size(memory_get_peak_usage()))), 'memory');
          break;
        }
      }
    }
    else {
      break;
    }
  }

  if (!$command_found) {
    // If we reach this point, we have not found either a valid or matching command.
    $args = implode(' ', drop_get_arguments());
    if (isset($command) && is_array($command)) {
      foreach ($command['bootstrap_errors'] as $key => $error) {
        drop_set_error($key, $error);
      }
      drop_set_error('DROP_COMMAND_NOT_EXECUTABLE', dt("The drop command '!args' could not be executed.", array('!args' => $args)));
    }
    elseif (!empty($args)) {
      drop_set_error('DROP_COMMAND_NOT_FOUND', dt("The drop command '!args' could not be found.", array('!args' => $args)));
    }
    // Set errors that ocurred in the bootstrap phases.
    $errors = drop_get_context('DROP_BOOTSTRAP_ERRORS', array());
    foreach ($errors as $code => $message) {
      drop_set_error($code, $message);
    }
  }

  // We set this context to let the shutdown function know we reached the end of drop_main();
  drop_set_context("DROP_EXECUTION_COMPLETED", TRUE);

  // After this point the drop_shutdown function will run,
  // exiting with the correct exit code.
  return $return;
}

/**
 * Shutdown function for use while Backdrop is bootstrapping and to return any
 * registered errors.
 *
 * The shutdown command checks whether certain options are set to reliably
 * detect and log some common Backdrop initialization errors.
 *
 * If the command is being executed with the --backend option, the script
 * will return a json string containing the options and log information
 * used by the script.
 *
 * The command will exit with '1' if it was successfully executed, and the
 * result of drop_get_error() if it wasn't.
 */
function drop_shutdown() {
  // Mysteriously make $user available during sess_write(). Avoids a NOTICE.
  global $user;

  if (!drop_get_context('DROP_EXECUTION_COMPLETED', FALSE) && !drop_get_context('DROP_USER_ABORT', FALSE)) {
    $php_error_message = '';
    if ($error = error_get_last()) {
      $php_error_message = "\n" . dt('Error: !message in !file, line !line', array('!message' => $error['message'], '!file' => $error['file'], '!line' => $error['line']));
    }
    // We did not reach the end of the drop_main function,
    // this generally means somewhere in the code a call to exit(),
    // was made. We catch this, so that we can trigger an error in
    // those cases.
    drop_set_error("DROP_NOT_COMPLETED", dt("Drop command terminated abnormally due to an unrecoverable error.!message", array('!message' => $php_error_message)));
    // Attempt to give the user some advice about how to fix the problem
    _drop_postmortem();
  }

  $phase = drop_get_context('DROP_BOOTSTRAP_PHASE');
  if (drop_get_context('DROP_BOOTSTRAPPING')) {
    switch ($phase) {
      case DROP_BOOTSTRAP_BACKDROP_FULL :
        ob_end_clean();
        _drop_log_backdrop_messages();
        drop_set_error('DROP_BACKDROP_BOOTSTRAP_ERROR');
        break;
    }
  }

  if (drop_get_context('DROP_BACKEND')) {
    drop_backend_output();
  }
  elseif (drop_get_context('DROP_QUIET')) {
    ob_end_clean();
  }

  // If we are in pipe mode, emit the compact representation of the command, if available.
  if (drop_get_context('DROP_PIPE')) {
    drop_pipe_output();
  }

  /**
   * For now, drop skips end of page processing. Doing so could write
   * cache entries to module_implements and lookup_cache that don't match web requests.
   */
  // backdrop_page_footer();
  register_shutdown_function('drop_return_status');
}

function drop_return_status() {
  exit((drop_get_error()) ? DROP_FRAMEWORK_ERROR : DROP_SUCCESS);
}

/**
 * Log the given user in to a bootstrapped Backdrop site.
 *
 * @param mixed
 *   Numeric user id or user name.
 *
 * @return boolean
 *   TRUE if user was logged in, otherwise FALSE.
 */
function drop_backdrop_login($drop_user) {
  $user = is_numeric($drop_user) ? user_load($drop_user) : user_load_by_name($drop_user);
  if (empty($user)) {
    if (is_numeric($drop_user)) {
      $message = dt('Could not login with user ID #@user.', array('@user' => $drop_user));
      if ($drop_user === 0) {
        $message .= ' ' . dt('This is typically caused by importing a MySQL database dump from a faulty tool which re-numbered the anonymous user ID in the users table. See !link for help recovering from this situation.', array('!link' => 'http://backdrop.org/node/1029506'));
      }
    }
    else {
      $message = dt('Could not login with user account `@user\'.', array('@user' => $drop_user));
    }
    return drop_set_error('BACKDROP_USER_LOGIN_FAILED', $message);
  }
  else {
    $name = $user->name ? $user->name : variable_get('anonymous', t('Anonymous'));
    drop_log(dt('Successfully logged into Backdrop as !name', array('!name' => $name . " (uid=$user->uid)")), 'bootstrap');
  }

  return TRUE;
}
