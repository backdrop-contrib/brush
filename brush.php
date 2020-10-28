#!/usr/bin/env php
<?php

/**
 * @file
 * brush is a PHP script implementing a command line shell for Backdrop.
 *
 * @requires PHP CLI 5.2.0, or newer.
 */
// Terminate immediately unless invoked as a command line script
if (!brush_verify_cli()) {
  die('brush is designed to run via the command line.');
}

// Check supported version of PHP.
define('BRUSH_MINIMUM_PHP', '5.2.0');
if (version_compare(phpversion(), BRUSH_MINIMUM_PHP) < 0) {
  die('Your command line PHP installation is too old. Brush requires at least PHP ' . BRUSH_MINIMUM_PHP . "\n");
}

define('BRUSH_BASE_PATH', dirname(__FILE__));


define('BRUSH_REQUEST_TIME', microtime(TRUE));

require_once BRUSH_BASE_PATH . '/includes/environment.inc';
require_once BRUSH_BASE_PATH . '/includes/command.inc';
require_once BRUSH_BASE_PATH . '/includes/brush.inc';
require_once BRUSH_BASE_PATH . '/includes/backend.inc';
require_once BRUSH_BASE_PATH . '/includes/batch.inc';
require_once BRUSH_BASE_PATH . '/includes/context.inc';
require_once BRUSH_BASE_PATH . '/includes/sitealias.inc';

brush_set_context('argc', $GLOBALS['argc']);
brush_set_context('argv', $GLOBALS['argv']);

// Set an error handler and a shutdown function
set_error_handler('brush_error_handler');
register_shutdown_function('brush_shutdown');

exit(brush_main());


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
function brush_verify_cli() {
  return (php_sapi_name() == 'cli' || (is_numeric($_SERVER['argc']) && $_SERVER['argc'] > 0));
}

/**
* The main Brush function.
*
* - Parses the command line arguments, configuration files and environment.
* - Prepares and executes a Backdrop bootstrap, if possible,
* - Dispatches the given command.
*
* @return
*   Whatever the given command returns.
*/
function brush_main() {
  $phases = _brush_bootstrap_phases(FALSE, TRUE);
  brush_set_context('BRUSH_BOOTSTRAP_PHASE', BRUSH_BOOTSTRAP_NONE);

  // We need some global options processed at this early stage. Namely --debug.
  brush_parse_args();
  _brush_bootstrap_global_options();

  $return = '';
  $command_found = FALSE;

  foreach ($phases as $phase) {
    if (brush_bootstrap_to_phase($phase)) {
      $command = brush_parse_command();

      // Process a remote command if 'remote-host' option is set.
      if (brush_remote_command()) {
        $command_found = TRUE;
        break;
      }

      if (is_array($command)) {
        $bootstrap_result = brush_bootstrap_to_phase($command['bootstrap']);
        brush_enforce_requirement_bootstrap_phase($command);
        brush_enforce_requirement_core($command);
        brush_enforce_requirement_backdrop_dependencies($command);
        brush_enforce_requirement_brush_dependencies($command);

        if ($bootstrap_result && empty($command['bootstrap_errors'])) {
          brush_log(bt("Found command: !command (commandfile=!commandfile)", array('!command' => $command['command'], '!commandfile' => $command['commandfile'])), 'bootstrap');

          $command_found = TRUE;
          // Dispatch the command(s).
          $return = brush_dispatch($command);

          // prevent a '1' at the end of the output
          if ($return === TRUE) {
            $return = '';
          }

          if (brush_get_context('BRUSH_DEBUG') && !brush_get_context('BRUSH_QUIET')) {
            brush_print_timers();
          }
          brush_log(bt('Peak memory usage was !peak', array('!peak' => brush_format_size(memory_get_peak_usage()))), 'memory');
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
    $args = implode(' ', brush_get_arguments());
    if (isset($command) && is_array($command)) {
      foreach ($command['bootstrap_errors'] as $key => $error) {
        brush_set_error($key, $error);
      }
      brush_set_error('BRUSH_COMMAND_NOT_EXECUTABLE', bt("The brush command '!args' could not be executed.", array('!args' => $args)));
    }
    elseif (!empty($args)) {
      brush_set_error('BRUSH_COMMAND_NOT_FOUND', bt("The brush command '!args' could not be found.", array('!args' => $args)));
    }
    // Set errors that ocurred in the bootstrap phases.
    $errors = brush_get_context('BRUSH_BOOTSTRAP_ERRORS', array());
    foreach ($errors as $code => $message) {
      brush_set_error($code, $message);
    }
  }

  // We set this context to let the shutdown function know we reached the end of brush_main();
  brush_set_context("BRUSH_EXECUTION_COMPLETED", TRUE);

  // After this point the brush_shutdown function will run,
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
 * result of brush_get_error() if it wasn't.
 */
function brush_shutdown() {
  // Mysteriously make $user available during sess_write(). Avoids a NOTICE.
  global $user;

  if (!brush_get_context('BRUSH_EXECUTION_COMPLETED', FALSE) && !brush_get_context('BRUSH_USER_ABORT', FALSE)) {
    $php_error_message = '';
    if ($error = error_get_last()) {
      $php_error_message = "\n" . bt('Error: !message in !file, line !line', array('!message' => $error['message'], '!file' => $error['file'], '!line' => $error['line']));
    }
    // We did not reach the end of the brush_main function,
    // this generally means somewhere in the code a call to exit(),
    // was made. We catch this, so that we can trigger an error in
    // those cases.
    brush_set_error("BRUSH_NOT_COMPLETED", bt("Brush command terminated abnormally due to an unrecoverable error.!message", array('!message' => $php_error_message)));
    // Attempt to give the user some advice about how to fix the problem
    _brush_postmortem();
  }

  $phase = brush_get_context('BRUSH_BOOTSTRAP_PHASE');
  if (brush_get_context('BRUSH_BOOTSTRAPPING')) {
    switch ($phase) {
      case BRUSH_BOOTSTRAP_BACKDROP_FULL :
        ob_end_clean();
        _brush_log_backdrop_messages();
        brush_set_error('BRUSH_BACKDROP_BOOTSTRAP_ERROR');
        break;
    }
  }

  if (brush_get_context('BRUSH_BACKEND')) {
    brush_backend_output();
  }
  elseif (brush_get_context('BRUSH_QUIET')) {
    ob_end_clean();
  }

  // If we are in pipe mode, emit the compact representation of the command, if available.
  if (brush_get_context('BRUSH_PIPE')) {
    brush_pipe_output();
  }

  /**
   * For now, brush skips end of page processing. Doing so could write
   * cache entries to module_implements and lookup_cache that don't match web requests.
   */
  // backdrop_page_footer();
  register_shutdown_function('brush_return_status');
}

function brush_return_status() {
  exit((brush_get_error()) ? BRUSH_FRAMEWORK_ERROR : BRUSH_SUCCESS);
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
function brush_backdrop_login($brush_user) {
  $user = is_numeric($brush_user) ? user_load($brush_user) : user_load_by_name($brush_user);
  if (empty($user)) {
    if (is_numeric($brush_user)) {
      $message = bt('Could not login with user ID #@user.', array('@user' => $brush_user));
      if ($brush_user === 0) {
        $message .= ' ' . bt('This is typically caused by importing a MySQL database dump from a faulty tool which re-numbered the anonymous user ID in the users table. See !link for help recovering from this situation.', array('!link' => 'http://drupal.org/node/1029506'));
      }
    }
    else {
      $message = bt('Could not login with user account `@user\'.', array('@user' => $brush_user));
    }
    return brush_set_error('BACKDROP_USER_LOGIN_FAILED', $message);
  }
  else {
    $name = $user->name ? $user->name : state_get('anonymous', t('Anonymous'));
    brush_log(bt('Successfully logged into Backdrop as !name', array('!name' => $name . " (uid=$user->uid)")), 'bootstrap');
  }

  return TRUE;
}
