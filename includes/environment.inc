<?php

/**
 * @file
 * Functions used by brush to query the environment and
 * setting the current configuration.
 */

/**
 * The indicator for a Backdrop installation folder.
 */
define('BRUSH_BACKDROP_BOOTSTRAP', 'core/includes/bootstrap.inc');

/**
 * @name Brush bootstrap phases
 * @{
 * Sequential Brush bootstrapping phases.
 */

/**
 * No bootstrap.
 *
 * This constant is only used to indicate that the bootstrap process has
 * not started yet.  It is not possible to have no bootstrap.
 */
define('BRUSH_BOOTSTRAP_NONE', -1);

/**
 * Use brush_bootstrap_max instead of brush_bootstrap_to_phase
 *
 * This constant is only usable as the value of the 'bootstrap'
 * item of a command object, or as the parameter to
 * brush_bootstrap_to_phase.  It is not a real bootstrap state.
 */
define('BRUSH_BOOTSTRAP_MAX', -2);

/**
 * Only bootstrap Brush, without any Backdrop specific code.
 *
 * Any code that operates on the Brush installation, and not specifically
 * any Backdrop directory, should bootstrap to this phase.
 */
define('BRUSH_BOOTSTRAP_BRUSH', 0);

/**
 * Set up and test for a valid backdrop root, either through the -r/--root options,
 * or evaluated based on the current working directory.
 *
 * Any code that interacts with an entire Backdrop installation, and not a specific
 * site on the Backdrop installation should use this bootstrap phase.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_ROOT',  1);

/**
 * Set up a Backdrop site directory and the correct environment variables to
 * allow Backdrop to find the configuration file.
 *
 * If no site is specified with the -l / --uri options, Brush will assume the
 * site is 'default', which mimics Backdrop's behaviour.
 *
 * If you want to avoid this behaviour, it is recommended that you use the
 * BRUSH_BOOTSTRAP_BACKDROP_ROOT bootstrap phase instead.
 *
 * Any code that needs to modify or interact with a specific Backdrop site's
 * settings.php file should bootstrap to this phase.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_SITE', 2);

/**
 * Load the settings from the Backdrop sites directory.
 *
 * This phase is analagous to the BACKDROP_BOOTSTRAP_CONFIGURATION bootstrap phase in Backdrop
 * itself, and this is also the first step where Backdrop specific code is included.
 *
 * This phase is commonly used for code that interacts with the Backdrop install API,
 * as both install.php and update.php start at this phase.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION', 3);

/**
 * Connect to the Backdrop database using the database credentials loaded
 * during the previous bootstrap phase.
 *
 * This phase is analogous to the BACKDROP_BOOTSTRAP_DATABASE bootstrap phase in
 * Backdrop.
 *
 * Any code that needs to interact with the Backdrop database API needs to
 * be bootstrapped to at least this phase.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_DATABASE', 4);

/**
 * Fully initialize Backdrop.
 *
 * This is analogous to the BACKDROP_BOOTSTRAP_FULL bootstrap phase in
 * Backdrop.
 *
 * Any code that interacts with the general Backdrop API should be
 * bootstrapped to this phase.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_FULL', 5);

/**
 * Log in to the initialiased Backdrop site.
 *
 * This is the default bootstrap phase all commands will try to reach,
 * unless otherwise specified.
 *
 * This bootstrap phase is used after the site has been
 * fully bootstrapped.
 *
 * This phase will log you in to the backdrop site with the username
 * or user ID specified by the --user/ -u option.
 *
 * Use this bootstrap phase for your command if you need to have access
 * to information for a specific user, such as listing nodes that might
 * be different based on who is logged in.
 */
define('BRUSH_BOOTSTRAP_BACKDROP_LOGIN', 6);

/**
 * Supported version of Console Table. This is displayed in the manual install help.
 */
define('BRUSH_TABLE_VERSION', '1.3.1');

/**
 * URL for automatic file download for supported version of Console Table.
 * Also can get from http://download.pear.php.net/package/Console_Table-1.3.1.tgz
 */
define('BRUSH_TABLE_URL', 'https://raw.githubusercontent.com/pear/Console_Table/master/Table.php');


/**
 * Helper function listing phases.
 *
 * For commands that need to iterate through the phases, such as help
 */
function _brush_bootstrap_phases($function_names = FALSE, $init_phases_only = FALSE) {
  static $functions = array(
    BRUSH_BOOTSTRAP_BRUSH                  => '_brush_bootstrap_brush',
    BRUSH_BOOTSTRAP_BACKDROP_ROOT            => '_brush_bootstrap_backdrop_root',
    BRUSH_BOOTSTRAP_BACKDROP_SITE            => '_brush_bootstrap_backdrop_site',
    BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION   => '_brush_bootstrap_backdrop_configuration',
    BRUSH_BOOTSTRAP_BACKDROP_DATABASE        => '_brush_bootstrap_backdrop_database',
    BRUSH_BOOTSTRAP_BACKDROP_FULL            => '_brush_bootstrap_backdrop_full',
    BRUSH_BOOTSTRAP_BACKDROP_LOGIN           => '_brush_bootstrap_backdrop_login');

  $result = array();
  if ($init_phases_only) {
    foreach (array(BRUSH_BOOTSTRAP_BRUSH, BRUSH_BOOTSTRAP_BACKDROP_FULL) as $phase) {
      $result[$phase] = $functions[$phase];
    }
  }
  else {
    $result = $functions;
  }
  if (!$function_names) {
    $result = array_keys($result);
  }
  return $result;
}

/**
 * @} End of Brush bootstrap phases.
 */

/**
 * Bootstrap Brush to the desired phase.
 *
 * This function will sequentially bootstrap each
 * lower phase up to the phase that has been requested.
 *
 * @param phase
 *   The bootstrap phase to bootstrap to.
 *   Any of the following constants :
 *     BRUSH_BOOTSTRAP_BRUSH                = Only Brush.
 *     BRUSH_BOOTSTRAP_BACKDROP_ROOT          = Find a valid Backdrop root.
 *     BRUSH_BOOTSTRAP_BACKDROP_SITE          = Find a valid Backdrop site.
 *     BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION = Load the site's settings.
 *     BRUSH_BOOTSTRAP_BACKDROP_DATABASE      = Initialize the database.
 *     BRUSH_BOOTSTRAP_BACKDROP_FULL          = Initialize Backdrop fully.
 *     BRUSH_BOOTSTRAP_BACKDROP_LOGIN         = Log into Backdrop with a valid user.
 */
function brush_bootstrap($phase, $phase_max = FALSE) {
  static $phases;
  if (!$phases) {
    $phases = _brush_bootstrap_phases(TRUE);
  }
  static $phase_index = 0;

  brush_set_context('BRUSH_BOOTSTRAPPING', TRUE);
  while ($phase >= $phase_index && isset($phases[$phase_index])) {
    if (brush_bootstrap_validate($phase_index)) {
      $current_phase = $phases[$phase_index];
      if (function_exists($current_phase) && !brush_get_error()) {
        brush_log(dt("Brush bootstrap phase : !function()", array('!function' => $current_phase)), 'bootstrap');
        $current_phase();

        // Find any command files that are available during this bootstrap phase.
        _brush_find_commandfiles($phase_index, $phase_max);
      }
      brush_set_context('BRUSH_BOOTSTRAP_PHASE', $phase_index);
    }
    else {
      $errors = brush_get_context('BRUSH_BOOTSTRAP_ERRORS', array());
      foreach ($errors as $code => $message) {
        brush_set_error($code, $message);
      }
    }
    unset($phases[$phase_index++]);
  }
  brush_set_context('BRUSH_BOOTSTRAPPING', FALSE);

  return !brush_get_error();
}

/**
 * Determine whether a given bootstrap phase has been completed
 *
 * @param phase
 *   The bootstrap phase to test
 *
 * @returns
 *   TRUE if the specified bootstrap phase has completed.
 */
function brush_has_boostrapped($phase) {
  $phase_index = brush_get_context('BRUSH_BOOTSTRAP_PHASE');

  return isset($phase_index) && ($phase_index >= $phase);
}

/**
 * Validate whether a bootstrap phases can be reached.
 *
 * This function will validate the settings that will be used
 * during the actual bootstrap process, and allow commands to
 * progressively bootstrap to the highest level that can be reached.
 *
 * This function will only run the validation function once, and
 * store the result from that execution in a local static. This avoids
 * validating phases multiple times.
 *
 * @param phase
 *   The bootstrap phase to validate to.
 *   Any of the following constants :
 *     BRUSH_BOOTSTRAP_BRUSH                = Only Brush.
 *     BRUSH_BOOTSTRAP_BACKDROP_ROOT          = Find a valid Backdrop root.
 *     BRUSH_BOOTSTRAP_BACKDROP_SITE          = Find a valid Backdrop site.
 *     BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION = Load the site's settings.
 *     BRUSH_BOOTSTRAP_BACKDROP_DATABASE      = Initialize the database.
 *     BRUSH_BOOTSTRAP_BACKDROP_FULL          = Initialize Backdrop fully.
 *     BRUSH_BOOTSTRAP_BACKDROP_LOGIN         = Log into Backdrop with a valid user.
 *
 * @return
 *   True if bootstrap is possible, False if the validation failed.
 *
 */
function brush_bootstrap_validate($phase) {
  static $phases;
  static $result_cache = array();
  if (!$phases) {
    $phases = _brush_bootstrap_phases(TRUE);
  }
  static $phase_index = 0;
  if (!array_key_exists($phase, $result_cache)) {
    brush_set_context('BRUSH_BOOTSTRAP_ERRORS', array());
    brush_set_context('BRUSH_BOOTSTRAP_VALUES', array());

    while ($phase >= $phase_index && isset($phases[$phase_index])) {
      $current_phase = $phases[$phase_index] . '_validate';
      if (function_exists($current_phase)) {
        $result_cache[$phase_index] = $current_phase();
      }
      else {
        $result_cache[$phase_index] = TRUE;
      }
      brush_set_context('BRUSH_BOOTSTRAP_VALIDATION_PHASE', $phase_index);
      unset($phases[$phase_index++]);
    }
  }
  return $result_cache[$phase];
}

/**
 * Bootstrap to the specified phase.
 *
 * @param $max_phase_index
 *   Only attempt bootstrap to the specified level.
 */
function brush_bootstrap_to_phase($max_phase_index) {
  // If $max_phase_index is BRUSH_BOOTSTRAP_MAX, then
  // we will bootstrap as far as we can.  brush_bootstrap_max
  // is different than brush_bootstrap_to_phase in that
  // it is not an error if BRUSH_BOOTSTRAP_LOGIN is not reached.
  if ($max_phase_index == BRUSH_BOOTSTRAP_MAX) {
    brush_bootstrap_max();
    return TRUE;
  }

  brush_log(dt("Bootstrap to phase !phase.", array('!phase' => $max_phase_index)), 'bootstrap');
  $phases = _brush_bootstrap_phases();
  $result = TRUE;

  // Try to bootstrap to the maximum possible level, without generating errors
  foreach ($phases as $phase_index) {
    if ($phase_index > $max_phase_index) {
      // Stop trying, since we achieved what was specified.
      break;
    }

    if (brush_bootstrap_validate($phase_index)) {
      if ($phase_index > brush_get_context('BRUSH_BOOTSTRAP_PHASE')) {
        $result = brush_bootstrap($phase_index, $max_phase_index);
      }
    }
    else {
      break;
    }
  }

  return $result;
}

/**
 * Bootstrap to the highest level possible, without triggering any errors.
 *
 * @param $max_phase_index
 *   Only attempt bootstrap to the specified level.
 */
function brush_bootstrap_max($max_phase_index = FALSE) {
  $phases = _brush_bootstrap_phases();
  $phase_index = BRUSH_BOOTSTRAP_BRUSH;
  if (!$max_phase_index) {
    $max_phase_index = count($phases);
  }

  // Try to bootstrap to the maximum possible level, without generating errors
  foreach ($phases as $phase_index) {
    if ($phase_index > $max_phase_index) {
      // Stop trying, since we achieved what was specified.
      break;
    }

    if (brush_bootstrap_validate($phase_index)) {
      if ($phase_index > brush_get_context('BRUSH_BOOTSTRAP_PHASE')) {
        brush_bootstrap($phase_index, $max_phase_index);
      }
    }
    else {
      break;
    }
  }

  return brush_get_context('BRUSH_BOOTSTRAP_PHASE');
}

/**
 * Bootstrap the specified site alias.  The site alias must
 * be a valid alias to a local site.
 *
 * @param $site_record
 *   The alias record for the given site alias.
 *   @see brush_sitealias_get_record().
 * @param $max_phase_index
 *   Only attempt bootstrap to the specified level.
 * @returns TRUE if attempted to bootstrap, or FALSE
 *   if no bootstrap attempt was made.
 */
function brush_bootstrap_max_to_sitealias($site_record, $max_phase_index = NULL) {
  if ((array_key_exists('root', $site_record) && !array_key_exists('remote-host', $site_record))) {
    brush_sitealias_set_alias_context($site_record);
    brush_bootstrap_max($max_phase_index);
    return TRUE;
  }
  return FALSE;
}

/**
 * Helper function to collect any errors that occur during the bootstrap process.
 * Always returns FALSE, for convenience.
 */
function brush_bootstrap_error($code, $message = null) {
  $errors = brush_get_context('BRUSH_BOOTSTRAP_ERRORS');
  $errors[$code] = $message;
  brush_set_context('BRUSH_BOOTSTRAP_ERRORS', $errors);
  return FALSE;
}

/**
 * Log PHP errors to the Brush log. This is in effect until Backdrop's error
 * handler takes over.
 */
function brush_error_handler($errno, $message, $filename, $line, $context) {
  // E_DEPRECATED was added in PHP 5.3. Backdrop will not fix all the
  // deprecated errors, but suppresses them. So we suppress them as well.
  if (defined('E_DEPRECATED')) {
    $errno = $errno & ~E_DEPRECATED;
  }

  // "error_reporting" is usually set in php.ini, but may be changed by
  // brush_errors_on() and brush_errors_off().
  if ($errno & error_reporting()) {
    // By default we log notices.
    $type = brush_get_option('php-notices', 'notice');

    // Bitmask value that constitutes an error needing to be logged.
    $error = E_ERROR | E_PARSE | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR;
    if ($errno & $error) {
      $type = 'error';
    }

    // Bitmask value that constitutes a warning being logged.
    $warning = E_WARNING | E_CORE_WARNING | E_COMPILE_WARNING | E_USER_WARNING;
    if ($errno & $warning) {
      $type = 'warning';
    }

    brush_log($message . ' ' . basename($filename) . ':' . $line, $type);

    return TRUE;
  }
}

/**
 * Helper function to store any context settings that are being validated.
 */
function brush_bootstrap_value($context, $value = null) {
  $values =& brush_get_context('BRUSH_BOOTSTRAP_VALUES', array());

  if (!is_null($value)) {
    $values[$context] = $value;
  }

  if (array_key_exists($context, $values)) {
    return $values[$context];
  }

  return null;
}

/*
 * Returns a localizable message about php.ini that
 * varies depending on whether the php_ini_loaded_file()
 * is available or not.
 */
function _brush_php_ini_loaded_file_message() {
  if (function_exists('php_ini_loaded_file')) {
    return dt('Please check your configuration settings in !phpini or in your brush.ini file; see examples/example.brush.ini for details.', array('!phpini' => php_ini_loaded_file()));
  }
  else {
    return dt('Please check your configuration settings in your php.ini file or in your brush.ini file; see examples/example.brush.ini for details.');
  }
}

/**
 * Evalute the environment after an abnormal termination and
 * see if we can determine any configuration settings that the user might
 * want to adjust.
 */
function _brush_postmortem() {
  // Make sure that the memory limit has been bumped up from the minimum default value of 32M.
  $php_memory_limit = brush_memory_limit();
  if (($php_memory_limit > 0) && ($php_memory_limit <= 32*BRUSH_BACKDROP_KILOBYTE*BRUSH_BACKDROP_KILOBYTE)) {
    brush_set_error('BRUSH_MEMORY_LIMIT', dt('Your memory limit is set to !memory_limit; brush needs as much memory to run as Backdrop.  !php_ini_msg', array('!memory_limit' => $php_memory_limit / (BRUSH_BACKDROP_KILOBYTE*BRUSH_BACKDROP_KILOBYTE) . 'M', '!php_ini_msg' => _brush_php_ini_loaded_file_message())));
  }
}

/**
 * Evaluate the environment before command bootstrapping
 * begins.  If the php environment is too restrictive, then
 * notify the user that a setting change is needed and abort.
 */
function _brush_environment_check_php_ini() {
  $ini_checks = array('safe_mode' => '', 'open_basedir' => '', 'disable_functions' => array('exec', 'system'), 'disable_classes' => '');

  // Test to insure that certain php ini restrictions have not been enabled
  $prohibited_list = array();
  foreach ($ini_checks as $prohibited_mode => $disallowed_value) {
    $ini_value = ini_get($prohibited_mode);
    $invalid_value = FALSE;
    if (empty($disallowed_value)) {
      $invalid_value = !empty($ini_value);
    }
    else {
      foreach ($disallowed_value as $test_value) {
        if (strstr($ini_value, $test_value) !== FALSE) {
          $invalid_value = TRUE;
        }
      }
    }
    if ($invalid_value) {
      $prohibited_list[] = $prohibited_mode;
    }
  }
  if (!empty($prohibited_list)) {
    brush_log(dt('The following restricted PHP modes have non-empty values: !prohibited_list. This configuration is incompatible with brush.  !php_ini_msg', array('!prohibited_list' => implode(' and ', $prohibited_list), '!php_ini_msg' => _brush_php_ini_loaded_file_message())), 'error');
  }

  return TRUE;
}

/**
 * Validate that Brush is running in a suitable environment.
 */
function _brush_bootstrap_brush_validate() {
  // @todo _brush_environment_php_ini_checks() always returns TRUE.
  $return = _brush_environment_check_php_ini();
  if ($return !== TRUE) {
    return $return;
  }

  if (brush_environment_table_inc() === FALSE) {
    return FALSE;
  }

  return TRUE;
}

/*
 * To disable the warning about Windows support, set $options['check_os'] = FALSE
 * in brushrc.php. See examples/example.brushrc.php.
 */
function brush_environment_check_os() {
  if (substr(PHP_OS, 0, 3) == 'WIN') {
    $msg = 'Brush has significant limitations on Windows; it is not advisable to use on that platform.';
    brush_log(dt($msg), 'warning');
  }
}

function brush_environment_table_inc() {
  if (file_exists(PHP_LIBDIR.'/Console/Table.php')) {
    $tablefile = PHP_LIBDIR.'/Console/Table.php';
  }
  else {
    $tablefile = BRUSH_BASE_PATH . '/includes/table.inc';
  }
  require $tablefile;
}

/**
 * Initial Brush bootstrap phase.
 *
 * During the initialization of Brush,
 * this is the first step where all we are
 * aware of is Brush itself.
 *
 * In this step we will register the shutdown function,
 * parse the command line arguments and store them in their
 * related contexts.
 *
 * Configuration files (brushrc.php) that are
 *   a) Specified on the command line
 *   b) Stored in the root directory of brush.php
 *   c) Stored in the home directory of the system user.
 *
 * Additionally the BRUSH_QUIET and BRUSH_BACKEND contexts,
 * will be evaluated now, as they need to be set very early in
 * the execution flow to be able to take affect/
 */
function _brush_bootstrap_brush() {
  // Create an alias '@none' to represent no Backdrop site
  _brush_sitealias_cache_alias('none', array('root' => '', 'uri' => ''));

  // Set the terminal width, used for wrapping table output.
  // Normally this is exported using tput in the brush script.
  // If this is not present we do an additional check using stty here.
  if (!($columns = getenv('COLUMNS'))) {
    exec('stty size 2>&1', $stty_output, $stty_status);
    if (!$stty_status) $columns = preg_replace('/\d+\s(\d+)/', '$1', $stty_output[0], -1, $stty_count);
    // If stty failed, or we couldn't parse it's output, we assume 80 columns.
    if ($stty_status || !$stty_count) $columns = 80;
  }
  // If a caller wants to reserve some room to add additional
  // information to the brush output via post-processing, the
  // --reserve-margin flag can be used to declare how much
  // space to leave out.  This only affects brush functions
  // such as brush_print_table that wrap the output.
  $columns -= brush_get_option('reserve-margin', 0);
  brush_set_context('BRUSH_COLUMNS', $columns);

  // Statically define a way to call brush again.
  define('BRUSH_COMMAND', brush_find_brush());

  $brush_info = brush_read_brush_info();
  if (!empty($brush_info['version'])) {
    $brush_version = $brush_info['version'];
  }
  else {
    // There is no version indicated in the info file, so it's a development branch.
    $brush_version = '1.x-dev';
  }
  define('BRUSH_VERSION', $brush_version);
  $version_parts = explode('.', BRUSH_VERSION);
  define('BRUSH_MAJOR_VERSION', $version_parts[0]);

  // prime the CWD cache
  brush_cwd();

  /* Copy ETC_PREFIX and SHARE_PREFIX from environment variables if available.
   * This alters where we check for server-wide config and alias files.
   * Used by unit test suite to provide a clean environment.
   */
  if (getenv('ETC_PREFIX')) brush_set_context('ETC_PREFIX', getenv('ETC_PREFIX'));
  if (getenv('SHARE_PREFIX')) brush_set_context('SHARE_PREFIX', getenv('SHARE_PREFIX'));

  brush_set_context('DOC_PREFIX', BRUSH_BASE_PATH);
  if (!file_exists(BRUSH_BASE_PATH . '/README.txt') && file_exists(brush_get_context('SHARE_PREFIX', '/usr') . '/share/doc/brush') . '/README.txt') {
    brush_set_context('DOC_PREFIX', brush_get_context('SHARE_PREFIX', '/usr') . '/share/doc/brush');
  }

  // Load a brushrc.php file in the brush.php's directory.
  brush_load_config('brush');

  // Load a brushrc.php file in the $ETC_PREFIX/etc/brush directory.
  brush_load_config('system');

  // Load a brushrc.php file at ~/.brushrc.php.
  brush_load_config('user');

  // Load a brushrc.php file in the ~/.brush directory.
  brush_load_config('home.brush');

  // Load a custom config specified with the --config option.
  brush_load_config('custom');

  // Process the site alias that specifies which instance
  // of brush (local or remote) this command will operate on.
  // We must do this after we load our config files (so that
  // site aliases are available), but before the rest
  // of the brush and backdrop root bootstrap phases are
  // done, since site aliases may set option values that
  // affect these phases.
  // TODO: Note that this function will call brush_locate_root
  // (from within _brush_sitealias_find_record_for_local_site),
  // and brush_locate_root will be called again when bootstrapping
  // the backdrop root below.  Is there a good way to refactor this
  // so that we do not need to search for the root twice?
  brush_sitealias_check_arg();

  $backend = brush_set_context('BRUSH_BACKEND', brush_get_option(array('b', 'backend')));

  if ($backend) {
    // Load options passed as a JSON encoded string through STDIN.
    $stdin_options = _brush_backend_get_stdin();
    if (is_array($stdin_options)) {
      brush_set_context('stdin', $stdin_options);
    }
  }

  // Pipe implies quiet.
  $quiet = brush_set_context('BRUSH_QUIET', brush_get_option(array('q', 'quiet', 'p', 'pipe')));

  brush_set_context('BRUSH_PIPE', brush_get_option(array('p', 'pipe')));

  // When running in backend mode, all output is buffered, and returned
  // as a property of a JSON encoded associative array.
  if ($backend || $quiet) {
    ob_start();
  }

  _brush_bootstrap_global_options();
}

function _brush_bootstrap_global_options() {
  // Debug implies verbose
  brush_set_context('BRUSH_VERBOSE',     brush_get_option(array('v', 'verbose', 'd', 'debug'), FALSE));
  brush_set_context('BRUSH_DEBUG', brush_get_option(array('d', 'debug')));

  // Backend implies affirmative unless negative is explicitly specified
  brush_set_context('BRUSH_NEGATIVE',    brush_get_option(array('n', 'no'), FALSE));
  brush_set_context('BRUSH_AFFIRMATIVE', brush_get_option(array('y', 'yes'), FALSE) || (brush_get_context('BRUSH_BACKEND') && !brush_get_context('BRUSH_NEGATIVE')));
  brush_set_context('BRUSH_SIMULATE',    brush_get_option(array('s', 'simulate'), FALSE));

  // Suppress colored logging if --nocolor option is explicitly given or if
  // terminal does not support it.
  $nocolor = (brush_get_option(array('nocolor'), FALSE));
  if (!$nocolor) {
    // Check for colorless terminal.  If there is no terminal, then
    // 'tput colors 2>&1' will return "tput: No value for $TERM and no -T specified",
    // which is not numeric and therefore will put us in no-color mode.
    $colors = exec('tput colors 2>&1');
    $nocolor = !($colors === FALSE || (is_numeric($colors) && $colors >= 3));
  }
  brush_set_context('BRUSH_NOCOLOR', $nocolor);
}

/**
 * Validate the BRUSH_BOOTSTRAP_BACKDROP_ROOT phase.
 *
 * In this function, we will check if a valid Backdrop directory is available.
 * We also determine the value that will be stored in the BRUSH_BACKDROP_ROOT
 * context and BACKDROP_ROOT constant if it is considered a valid option.
 */
function _brush_bootstrap_backdrop_root_validate() {
  $backdrop_root = brush_get_option(array('r', 'root'), brush_locate_root());

  if (empty($backdrop_root)) {
    return brush_bootstrap_error('BRUSH_NO_BACKDROP_ROOT', dt("A Backdrop installation directory could not be found"));
  }
  if (!brush_valid_backdrop_root($backdrop_root)) {
    return brush_bootstrap_error('BRUSH_INVALID_BACKDROP_ROOT', dt("The directory !backdrop_root does not contain a valid Backdrop installation", array('!backdrop_root' => $backdrop_root)));
  }

  brush_bootstrap_value('backdrop_root', $backdrop_root);

  return TRUE;
}

/**
 * Bootstrap Brush with a valid Backdrop Directory.
 *
 * In this function, the pwd will be moved to the root
 * of the Backdrop installation.
 *
 * The BRUSH_BACKDROP_ROOT context and the BACKDROP_ROOT constant are
 * populated from the value that we determined during the validation phase.
 *
 * We also now load the brushrc.php for this specific platform.
 * We can now include files from the Backdrop Tree, and figure
 * out more context about the platform, such as the version of Backdrop.
 */
function _brush_bootstrap_backdrop_root() {
  $backdrop_root = brush_set_context('BRUSH_BACKDROP_ROOT', brush_bootstrap_value('backdrop_root'));
  define('BACKDROP_ROOT', $backdrop_root);

  chdir($backdrop_root);
  brush_load_config('backdrop');
  require_once BACKDROP_ROOT . '/' . BRUSH_BACKDROP_BOOTSTRAP;
  $version = brush_set_context('BRUSH_BACKDROP_VERSION', brush_backdrop_version());
  $major_version = brush_set_context('BRUSH_BACKDROP_MAJOR_VERSION', brush_backdrop_major_version());

  _brush_bootstrap_global_options();

  brush_log(dt("Initialized Backdrop !version root directory at !backdrop_root", array("!version" => $version, '!backdrop_root' => $backdrop_root)));
}

/**
 * VALIDATE the BRUSH_BOOTSTRAP_BACKDROP_SITE phase.
 *
 * In this function we determine the URL used for the command,
 * and check for a valid settings.php file.
 *
 * To do this, we need to set up the $_SERVER environment variable,
 * to allow us to use conf_path to determine what Backdrop will load
 * as a configuration file.
 */
function _brush_bootstrap_backdrop_site_validate() {
  $site_path = brush_site_path();
  $elements = explode('/', $site_path);
  $current = array_pop($elements);
  if (!$current) {
    $current = 'default';
  }
  $uri = 'http://'. $current;

  $brush_uri = brush_bootstrap_value('brush_uri', brush_get_option(array('l', 'uri'), $uri));

  // Fake the necessary HTTP headers that Backdrop needs:
  if ($brush_uri) {
    $backdrop_base_url = parse_url($brush_uri);
    // If there's no url scheme set, add http:// and re-parse the url
    // so the host and path values are set accurately.
    if (!array_key_exists('scheme', $backdrop_base_url)) {
      $brush_uri = 'http://' . $brush_uri;
      $backdrop_base_url = parse_url($brush_uri);
    }
    // Fill in defaults.
    $backdrop_base_url += array(
      'path' => NULL,
      'host' => NULL,
      'port' => NULL,
    );
    $_SERVER['HTTP_HOST'] = $backdrop_base_url['host'];

    if ($backdrop_base_url['port']) {
      $_SERVER['HTTP_HOST'] .= ':' . $backdrop_base_url['port'];
    }
    $_SERVER['SERVER_PORT'] = $backdrop_base_url['port'];

    if (array_key_exists('path', $backdrop_base_url)) {
      $_SERVER['PHP_SELF'] = $backdrop_base_url['path'] . '/index.php';
    }
    else {
      $_SERVER['PHP_SELF'] = '/index.php';
    }
  }
  else {
    $_SERVER['HTTP_HOST'] = 'default';
    $_SERVER['PHP_SELF'] = '/index.php';
  }

  $_SERVER['REQUEST_URI'] = $_SERVER['SCRIPT_NAME'] = $_SERVER['PHP_SELF'];
  $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
  $_SERVER['REQUEST_METHOD']  = NULL;

  $_SERVER['SERVER_SOFTWARE'] = NULL;
  $_SERVER['HTTP_USER_AGENT'] = NULL;

  $site = brush_bootstrap_value('site', $_SERVER['HTTP_HOST']);

  $conf_path = brush_bootstrap_value('conf_path', conf_path(TRUE, TRUE));
  $conf_file = "./$conf_path/settings.php";
  if (!file_exists($conf_file)) {
    return brush_bootstrap_error('BACKDROP_SITE_SETTINGS_NOT_FOUND', dt("Could not find a Backdrop settings.php file at !file.",
       array('!file' => $conf_file)));
  }

  return TRUE;
}

/**
 * Called by _brush_bootstrap_backdrop_site to do the main work
 * of the brush backdrop site bootstrap.
 */
function _brush_bootstrap_do_backdrop_site() {
  $brush_uri = brush_set_context('BRUSH_URI', brush_bootstrap_value('brush_uri'));
  $site = brush_set_context('BRUSH_BACKDROP_SITE', brush_bootstrap_value('site'));
  $conf_path = brush_set_context('BRUSH_BACKDROP_SITE_ROOT', brush_bootstrap_value('conf_path'));

  // Create an alias '@self'
  _brush_sitealias_cache_alias('self', array('root' => brush_get_context('BRUSH_BACKDROP_ROOT'), 'uri' => $brush_uri));

  brush_log(dt("Initialized Backdrop site !site at !site_root", array('!site' => $site, '!site_root' => $conf_path)));
  brush_load_config('site');

  _brush_bootstrap_global_options();
}

/**
 * Initialize a site on the Backdrop root.
 *
 * We now set various contexts that we determined and confirmed to be valid.
 * Additionally we load an optional brushrc.php file in the site directory.
 */
function _brush_bootstrap_backdrop_site() {
  _brush_bootstrap_do_backdrop_site();
  _brush_bootstrap_redo_backdrop_site();
}

/**
 * Re-do the backdrop site bootstrap (and possibly the
 * backdrop root bootstrap) if a site alias was processed
 * after the site bootstrap phase completed.  This will
 * happen when processing "brush sitealias command" for
 * a site alias defined in a brushrc.php file in the
 * default site's brush configuration directory.
 */
function _brush_bootstrap_redo_backdrop_site() {
  // If brush_load_config defined a site alias that did not
  // exist before, then sitealias check arg might now match
  // against one of those aliases.
  if (brush_sitealias_check_arg() === TRUE) {
    $remote_host = brush_get_option('remote-host');
    if (!isset($remote_host)) {
      // Check to see if the backdrop root changed.
      // If it has, we will set remote-host to cause
      // this command to be executed via the backend invoke
      // process.
      $sitealias_backdrop_root = brush_get_option(array('r', 'root'));
      if (($sitealias_backdrop_root != null) && (BACKDROP_ROOT != $sitealias_backdrop_root)) {
        brush_set_option('remote-host', 'localhost');
      }
      else {
        // If we set an alias, then we need to bootstrap the
        // backdrop site once again.  It is possible to re-bootstrap
        // the site at this point because settings.php has not
        // been included yet.
        brush_log(dt("Re-bootstrap backdrop site."));
        _brush_bootstrap_backdrop_site_validate();
        _brush_bootstrap_do_backdrop_site();
      }
    }
  }
}

/**
 * Initialize and load the Backdrop configuration files.
 *
 * We process and store a normalized set of database credentials
 * from the loaded configuration file, so we can validate them
 * and access them easily in the future.
 */
function _brush_bootstrap_backdrop_configuration() {
  global $conf, $brush_conf_override;

  backdrop_bootstrap(BACKDROP_BOOTSTRAP_CONFIGURATION);

  // Unset backdrop error handler and restore brush's one.

  restore_error_handler();

  // Overriding the $conf array from backdrop CONFIGURATION bootstrap with the
  // Overrides we collected on the loaded config files on BRUSH_BOOTSTRAP_BRUSH
  $conf = is_array($conf) && is_array($brush_conf_override) ? array_merge($conf, $brush_conf_override) : $conf;

  // Populate the BRUSH_DB_CREDENTIALS with the fields loaded from the configuration.
  $creds = array();
  switch (brush_backdrop_major_version()) {
     case 5:
     case 6:
       if (!empty($GLOBALS['db_url'])) {
         $url = $GLOBALS['db_url'];
         if (is_array($url)) {
           $url = $url['default'];
         }
         $parts = parse_url($url);
         $parts += array('pass' => '', 'port' => '');
         $creds['driver'] = $parts['scheme'];
         $creds['user'] = urldecode($parts['user']);
         $creds['host'] = $parts['host'];
         $creds['port'] = $parts['port'];
         $creds['pass'] = urldecode($parts['pass']);
         $creds['name'] = trim($parts['path'], '/');
       }
       break;
     case 7:
     default:
       if (!empty($GLOBALS['databases']['default']['default'])) {
         $conn = $GLOBALS['databases']['default']['default'];
         // Fill in defaults to prevent notices.
         $conn += array(
           'username' => NULL,
           'host' => NULL,
           'port' => NULL,
           'password' => NULL,
           'database' => NULL,
         );
         $creds['driver'] = $conn['driver'];
         $creds['user'] = $conn['username'];
         $creds['host'] = $conn['host'];
         $creds['port'] = $conn['port'];
         $creds['name'] = $conn['database'];
         $creds['pass'] = $conn['password'];
       }
       break;
  }

  brush_set_context('BRUSH_DB_CREDENTIALS', $creds);
}

/**
 * Validate the BRUSH_BOOTSTRAP_BACKDROP_DATABASE phase
 *
 * Attempt to making a working database connection using the
 * database credentials that were loaded during the previous
 * phase.
 */
function _brush_bootstrap_backdrop_database_validate() {
  if (!brush_valid_db_credentials()) {
    return brush_bootstrap_error('BRUSH_BACKDROP_DB_ERROR');
  }
  return TRUE;
}

/**
 * Boostrap the Backdrop database.
 */
function _brush_bootstrap_backdrop_database() {
  brush_log(dt("Successfully connected to the Backdrop database."), 'bootstrap');
  backdrop_bootstrap(BACKDROP_BOOTSTRAP_DATABASE);
}

/**
 * Attempt to load the full Backdrop system.
 */
function _brush_bootstrap_backdrop_full() {
  ob_start();
  backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);
  ob_end_clean();

  // If needed, prod module_implements() to recognize our system_watchdog() implementation.
  $dogs = module_implements('watchdog');
  if (!in_array('system', $dogs)) {
    // Note that this resets module_implements cache.
    module_implements('watchdog', FALSE, TRUE);
  }

  _brush_log_backdrop_messages();
}

/**
 * Log into the bootstrapped Backdrop site with a specific
 * username or user id.
 */
function _brush_bootstrap_backdrop_login() {
  $brush_user = brush_set_context('BRUSH_USER', brush_get_option(array('u', 'user'), 0));

  brush_backdrop_login($brush_user);
  _brush_log_backdrop_messages();
}


/**
 * Returns the current working directory.
 *
 * This is the directory as it was when brush was started, not the
 * directory we are currently in. For that, use getcwd() directly.
 */
function brush_cwd() {
  if ($path = brush_get_context('BRUSH_OLDCWD')) {
    return $path;
  }
  // We use PWD if available because getcwd() resolves symlinks, which
  // could take us outside of the Backdrop root, making it impossible to find.
  // $_SERVER['PWD'] isn't set on windows and generates a Notice.
  $path = isset($_SERVER['PWD']) ? $_SERVER['PWD'] : '';
  if (empty($path)) {
    $path = getcwd();
  }

  // Convert windows paths.
  $path = _brush_convert_path($path);

  // Save original working dir case some command wants it.
  brush_set_context('BRUSH_OLDCWD', $path);

  return $path;
}

/**
 * Converts a Windows path (dir1\dir2\dir3) into a Unix path (dir1/dir2/dir3).
 * Also converts a cygwin "drive emulation" path (/cygdrive/c/dir1) into a
 * proper drive path, still with Unix slashes (c:/dir1).
 */
function _brush_convert_path($path) {
  $path = str_replace('\\','/', $path);
  $path = preg_replace('/^\/cygdrive\/([A-Za-z])(.*)$/', '\1:\2', $path);

  return $path;
}

/**
 * Returns parent directory.
 *
 * @param string
 *   Path to start from.
 *
 * @return string
 *   Parent path of given path.
 */
function _brush_shift_path_up($path) {
  if (empty($path)) {
    return FALSE;
  }
  $path = explode('/', $path);
  // Move one directory up.
  array_pop($path);
  return implode('/', $path);
}

/**
 * Like Backdrop conf_path, but searching from beneath.
 * Allows proper site uri detection in site sub-directories.
 *
 * Essentially looks for a settings.php file.
 *
 * @param string
 *   Search starting path. Defaults to current working directory.
 *
 * @return
 *   Current site path (folder containing settings.php) or FALSE if not found.
 */
function brush_site_path($path = NULL) {
  static $site_path;

  if (!isset($site_path)) {
    $site_path = FALSE;

    $path = empty($path) ? brush_cwd() : $path;
    // Check the current path.
    if (file_exists($path . '/settings.php')) {
      $site_path = $path;
    }
    else {
      // Move up dir by dir and check each.
      while ($path = _brush_shift_path_up($path)) {
        if (file_exists($path . '/settings.php')) {
          $site_path = $path;
          break;
        }
      }
    }

    $site_root = brush_locate_root();
    if (file_exists($site_root . '/sites/sites.php')) {
      $sites = array();
      // This will overwrite $sites with the desired mappings.
      include($site_root . '/sites/sites.php');
      // We do a reverse lookup here to determine the URL given the site key.
      if ($match = array_search($site_path, $sites)) {
        $site_path = $match;
      }
    }

    // Last resort: try from site root
    if (!$site_path) {
      if ($site_root) {
        if (file_exists($site_root . '/sites/default/settings.php')) {
          $site_path = $site_root . '/sites/default';
        }
      }
    }
  }

  return $site_path;
}

/**
 * Exhaustive depth-first search to try and locate the Backdrop root directory.
 * This makes it possible to run brush from a subdirectory of the backdrop root.
 *
 * @param
 *   Search start path. Defaults to current working directory.
 * @return
 *   A path to backdrop root, or FALSE if not found.
 */
function brush_locate_root($start_path = NULL) {
  $backdrop_root = FALSE;

  $start_path = empty($start_path) ? brush_cwd() : $start_path;
  foreach (array(TRUE, FALSE) as $follow_symlinks) {
    $path = $start_path;
    if ($follow_symlinks && is_link($path)) {
      $path = realpath($path);
    }
    // Check the start path.
    if (brush_valid_backdrop_root($path)) {
      $backdrop_root = $path;
      break;
    }
    else {
      // Move up dir by dir and check each.
      while ($path = _brush_shift_path_up($path)) {
        if ($follow_symlinks && is_link($path)) {
          $path = realpath($path);
        }
        if (brush_valid_backdrop_root($path)) {
          $backdrop_root = $path;
          break 2;
        }
      }
    }
  }

  return $backdrop_root;
}

/**
 * Checks whether given path qualifies as a Backdrop root.
 *
 * @param string
 *   Path to check.
 *
 * @return boolean
 *   True if given path seems to be a Backdrop root, otherwise FALSE.
 */
function brush_valid_backdrop_root($path) {
  return !empty($path) && is_dir($path) && file_exists($path . '/' . BRUSH_BACKDROP_BOOTSTRAP);
}

/**
 * Tests the currently loaded database credentials to ensure a database connection can be made.
 */
function brush_valid_db_credentials() {
  $creds = brush_get_context('BRUSH_DB_CREDENTIALS');
  // Do minimal checking that we have the necessary information.
  if (count($creds) == 0) {
    return FALSE;
  }
  $type = $creds['driver'];
  switch (brush_backdrop_major_version()) {
    case 5:
    case 6:
      // Check availability of db extension in PHP and also Backdrop support.
      if (file_exists('./includes/install.'. $type .'.inc')) {
        require_once './includes/install.'. $type .'.inc';
        $function = $type .'_is_available';
        if (!$function()) {
          brush_log(dt('!type extension for PHP is not installed. Check your php.ini to see how you can enable it.', array('!type' => $type)), 'bootstrap');
          return FALSE;
        }
      }
      else {
        brush_log(dt('!type database type is unsupported.', array('!type' => $type)), 'bootstrap');
        return FALSE;
      }
      // Verify connection settings.
      switch ($type) {
        case 'mysql':
          $hostspec = $creds['port'] ? $creds['host'] . ':' . $creds['port'] : $creds['host'];
          $connection = @mysql_connect($hostspec, $creds['user'], $creds['pass']);
          if (!$connection || !mysql_select_db($creds['name'])) {
            brush_log(mysql_error(), 'bootstrap');
            return FALSE;
          }
          break;
        case 'mysqli':
          $connection = mysqli_init();
          @mysqli_real_connect($connection, $creds['host'], $creds['user'], $creds['pass'], $creds['name'], (int)$creds['port']);
          if (mysqli_connect_errno() > 0) {
            brush_log(mysqli_connect_error(), 'bootstrap');
            return FALSE;
          }
          break;
        case 'pgsql':
          $conn_string = sprintf("host=%s user=%s password=%s dbname=%s", $creds['host'], $creds['user'], $creds['pass'], $creds['name']);
          if (isset($creds['port'])) {
            $conn_string .= ' port=' . $creds['port'];
          }
          // Copied from d6's database.pgsql.inc:
          // pg_last_error() does not return a useful error message for database
          // connection errors. We must turn on error tracking to get at a good error
          // message, which will be stored in $php_errormsg.
          $track_errors_previous = ini_get('track_errors');
          ini_set('track_errors', 1);
          $connection = @pg_connect($conn_string);
          if (!$connection) {
            require_once './includes/unicode.inc';
            brush_log(decode_entities($php_errormsg), 'bootstrap');
            // Restore error tracking setting
            ini_set('track_errors', $track_errors_previous);
            return FALSE;
          }
          // Restore error tracking setting
          ini_set('track_errors', $track_errors_previous);
          break;
      }
      break;
    case 7:
    default:
      // Backdrop >=7 requires PDO and brush requires php 5.2, that ships with PDO
      // but it may be compiled with --disable-pdo.
      if (!class_exists('PDO')) {
        brush_log(dt('PDO support is required.'), 'bootstrap');
        return FALSE;
      }
      // Check the database specific driver is available.
      if (!in_array($type, PDO::getAvailableDrivers())) {
        brush_log(dt('!type extension for PHP PDO is not installed. Check your php.ini to see how you can enable it.', array('!type' => $type)), 'bootstrap');
        return FALSE;
      }
      // Build the connection string.
      if ($type === 'sqlite') {
        $constr = 'sqlite:' . $creds['name'];
      }
      else {
        $constr = sprintf("%s:dbname=%s;host=%s", $type, $creds['name'], $creds['host']);
        if (!empty($creds['port'])) {
          $constr .= sprintf(";port=%d", $creds['port']);
        }
      }
      try {
        $db = new PDO($constr, $creds['user'], $creds['pass']);
        $db = null;
      }
      catch (PDOException $e) {
        brush_log($e->getMessage(), 'bootstrap');
        return FALSE;
      }
      break;
  }

  return TRUE;
}

/**
 * Determine a proper way to call brush again
 *
 * This check if we were called directly or as an argument to some
 * wrapper command (php and sudo are checked now).
 *
 * Calling ./brush.php directly yields the following environment:
 *
 * _SERVER["argv"][0] => ./brush.php
 *
 * Calling php ./brush.php also yields the following:
 *
 * _SERVER["argv"][0] => ./brush.php
 *
 * Note that the $_ global is defined only in bash and therefore cannot
 * be relied upon.
 *
 * We will therefore assume PHP is available in the path and is named
 * "php" for execute ourselves. That is, the #!/usr/bin/env php is
 * working and valid, unless a PHP constant is defined, which can be
 * done by the shell wrapper.
 *
 * The BRUSH_COMMAND constant is initialised to the value of this
 * function when environment.inc is loaded.
 *
 * @see BRUSH_COMMAND
 */
function brush_find_brush() {
 $php = brush_get_option('php');
 if (isset($php)) {
    $brush = $php . " " . realpath($_SERVER['argv'][0]) . " --php='$php'";
  } else {
    $brush = realpath($_SERVER['argv']['0']);
  }
  return $brush;
}

/**
 * Read the brush info file.
 */
function brush_read_brush_info() {
  $brush_info_file = dirname(__FILE__) . '/../brush.info';

  return parse_ini_file($brush_info_file);
}

/**
 * Make a determination whether or not the given
 * host is local or not.
 *
 * @param host
 *   A hostname, 'localhost' or '127.0.0.1'.
 * @return
 *   True if the host is local.
 */
function brush_is_local_host($host) {
  // In order for this to work right, you must use 'localhost' or '127.0.0.1'
  // or the machine returned by 'uname -n' for your 'remote-host' entry in
  // your site alias.
  if (($host == 'localhost') || ($host == '127.0.0.1')) {
    return TRUE;
  }

  // If uname -n returns an fqdn (that is, uname -n == hostname -f),
  // then we will require that it exactly match the host in order
  // to be considered local.  However, the usual convention is for
  // uname -n to return only the node name (that is, uname -n == hostname -a).
  // When this is the case, we will consider $host to be local if the
  // machine portion of it (everything up to the first dot) matches the
  // current value of uname -n.  We prefer uname -n to hostname as
  // the output of uname is more regular than hostname.
  $uname = php_uname('n');
  return (strpos($uname,'.') !== FALSE) ? ($host == $uname) : substr($host . '.',0,strlen($uname)+1) == $uname . '.';
}

/**
 * Return the user's home directory.
 */
function brush_server_home() {
  $home = NULL;
  // $_SERVER['HOME'] isn't set on windows and generates a Notice.
  if (!empty($_SERVER['HOME'])) {
    $home = $_SERVER['HOME'];
  }
  elseif (!empty($_SERVER['HOMEDRIVE']) && !empty($_SERVER['HOMEPATH'])) {
    // home on windows
    $home = $_SERVER['HOMEDRIVE'] . $_SERVER['HOMEPATH'];
  }
  return $home;
}

/*
 * The path to the global cache directory.
 */
function brush_directory_cache() {
  if (getenv('CACHE_PREFIX')) {
    return getenv('CACHE_PREFIX') . '/cache';
  }
  elseif ($home = brush_server_home()) {
    return "$home/.brush/cache";
  }
  else {
    return FALSE;
  }
}

/**
 * Get complete information for all available extensions (modules and themes).
 *
 * @return
 *   An array containing info for all available extensions.
 */
function brush_get_extensions() {
  brush_include_engine('backdrop', 'environment');
  return array_merge(brush_get_modules(), brush_get_themes());
}

/**
 * Test compatibility of a extension with version of backdrop core and php.
 *
 * @param $file Extension object as returned by system_rebuild_module_data().
 * @return TRUE if the extension is incompatible.
 */
function brush_extension_check_incompatibility($file) {
  if (!isset($file->info['backdrop'])
       || $file->info['backdrop'] != BACKDROP_CORE_COMPATIBILITY
       || version_compare(phpversion(), $file->info['php']) < 0) {
    return TRUE;
  }
  return FALSE;
}

/**
 *
 */
function brush_backdrop_required_modules($modules) {
  brush_include_engine('backdrop', 'environment');
  return _brush_backdrop_required_modules($modules);
}

/**
 * Return the default theme. Backdroped.
 *
 * @return
 *  Machine name of the default theme.
 */
function brush_theme_get_default() {
  // Commands like `brush bd module` call this function without bootstraping,
  // so run this check to use config_get().
  if (brush_get_context('BRUSH_BOOTSTRAP_PHASE') < 5) {
    brush_bootstrap(BRUSH_BOOTSTRAP_BACKDROP_FULL);
  }
  $theme_default = config_get('system.core', 'theme_default');
  $theme_default = ucfirst($theme_default);
  return $theme_default;
}

/**
 * Return the administration theme. Backdroped.
 *
 * @return
 *  Machine name of the administration theme.
 */
function brush_theme_get_admin() {
  // Just in case if this function is called without bootstraping,
  // run this check to use config_get().
  if (brush_get_context('BRUSH_BOOTSTRAP_PHASE') < 5) {
    brush_bootstrap(BRUSH_BOOTSTRAP_BACKDROP_FULL);
  }
  if (config_get('system.core', 'admin_theme') == FALSE) {
    // Administration theme is set to Default theme, so returns 0.
    $admin_theme = brush_theme_get_default();
  } else {
    $admin_theme = config_get('system.core', 'admin_theme');
    $admin_theme = ucfirst($admin_theme);
  }
  return $admin_theme;
}
