<?php
/**
 * @file
 * The Brush context API implementation.
 *
 * This API acts as a storage mechanism for all options, arguments and
 * configuration settings that are loaded into brush.
 *
 * This API also acts as an IPC mechanism between the different brush commands,
 * and provides protection from accidentally overriding settings that are
 * needed by other parts of the system.
 *
 * It also avoids the necessity to pass references through the command chain
 * and allows the scripts to keep track of whether any settings have changed
 * since the previous execution.
 *
 * This API defines several contexts that are used by default.
 *
 * Argument contexts :
 *   These contexts are used by Brush to store information on the command.
 *   They have their own access functions in the forms of
 *   brush_set_arguments(), brush_get_arguments(), brush_set_command(),
 *   brush_get_command().
 *
 *     command : The brush command being executed.
 *     arguments : Any additional arguments that were specified.
 *
 * Setting contexts :
 *   These contexts store options that have been passed to the brush.php
 *   script, either through the use of any of the config files, directly from
 *   the command line through --option='value' or through a JSON encoded string
 *   passed through the STDIN pipe.
 *
 *   These contexts are accessible through the brush_get_option() and
 *   brush_set_option() functions.  See brush_context_names() for a description
 *   of all of the contexts.
 *
 *   Brush commands may also choose to save settings for a specific context to
 *   the matching configuration file through the brush_save_config() function.
 */


/**
 * Return a list of the valid brush context names.
 *
 * These context names are carefully ordered from
 * highest to lowest priority.
 *
 * These contexts are evaluated in a certain order, and the highest priority value
 * is returned by default from brush_get_option. This allows scripts to check whether
 * an option was different before the current execution.
 *
 *   Specified by the script itself :
 *     process  : Generated in the current process.
 *     cli      : Passed as --option=value to the command line.
 *     stdin    : Passed as a JSON encoded string through stdin.
 *     specific : Defined in a command-specific option record, and
 *                set in the command context whenever that command is used.
 *     alias    : Defined in an alias record, and set in the
 *                alias context whenever that alias is used.
 *
 *   Specified by config files :
 *     custom   : Loaded from the config file specified by --config or -c
 *     site     : Loaded from the brushrc.php file in the Backdrop site directory.
 *     backdrop   : Loaded from the brushrc.php file in the Backdrop root directory.
 *     user     : Loaded from the brushrc.php file in the user's home directory.
 *     home.brush Loaded from the brushrc.php file in the $HOME/.brush directory.
 *     system   : Loaded from the brushrc.php file in the system's $PREFIX/etc/brush directory.
 *     brush    : Loaded from the brushrc.php file in the same directory as brush.php.
 *
 *   Specified by the script, but has the lowest priority :
 *     default  : The script might provide some sensible defaults during init.
 */
function brush_context_names() {
  static $contexts = array(
    'process', 'cli', 'stdin', 'specific', 'alias',
    'custom', 'site', 'backdrop', 'user', 'home.brush', 'system',
    'brush', 'default');

  return $contexts;
}

/**
 * Return a list of possible brushrc file locations.
 *
 * @context
 *   A valid brush context from brush_context_names().
 * @prefix
 *   Optional. Specify a prefix to prepend to ".brushrc.php" when looking
 *   for config files. Most likely used by contrib commands.
 * @return
 *   An associative array containing possible config files to load
 *   The keys are the 'context' of the files, the values are the file
 *   system locations.
 */
function _brush_config_file($context, $prefix = NULL, $version = '') {
  $configs = array();
  $base_name = 'brush' . $version . 'rc.php';
  $config_file = $prefix ? $prefix . '.' . $base_name : $base_name;

  // Did the user explicitly specify a config file?
  if ($config = brush_get_option(array('c', 'config'))) {
    if (is_dir($config)) {
      $config = $config . '/brushrc.php';
    }
    $configs['custom'] = $config;
  }

  if ($site_path = brush_get_context('BRUSH_BACKDROP_SITE_ROOT')) {
    $configs['site'] = $site_path . "/" . $config_file;
  }

  if ($backdrop_root = brush_get_context('BRUSH_BACKDROP_ROOT')) {
    $configs['backdrop'] = $backdrop_root . '/' . $config_file;
  }

  // in the user home directory
  if (!is_null(brush_server_home())) {
    $configs['user'] = brush_server_home() . '/.' . $config_file;
  }

  // in $HOME/.brush directory
  if (!is_null(brush_server_home())) {
    $configs['home.brush'] = brush_server_home() . '/.brush/' . $config_file;
  }

  // In the system wide configuration folder.
  $configs['system'] = brush_get_context('ETC_PREFIX', '') . '/etc/brush/' . $config_file;

  // in the brush installation folder
  $configs['brush'] = dirname(__FILE__) . '/../' . $config_file;

  return empty($configs[$context]) ? '' : $configs[$context];
}


/**
 * Load brushrc files (if available) from several possible locations.
 */
function brush_load_config($context) {
  brush_load_config_file($context, _brush_config_file($context));
  brush_load_config_file($context, _brush_config_file($context, '', BRUSH_MAJOR_VERSION));
}

function brush_load_config_file($context, $config) {
  if (file_exists($config)) {
    $options = $aliases = $command_specific = $override = array();
    brush_log(dt('Loading brushrc "!config" into "!context" scope.', array('!config' => realpath($config), '!context' => $context)), 'bootstrap');
    $ret = @include_once($config);
    if ($ret === FALSE) {
      brush_log(dt('Cannot open brushrc "!config", ignoring.', array('!config' => realpath($config))), 'warning');
      return FALSE;
    }
    if (!empty($options) || !empty($aliases) || !empty($command_specific) || !empty($override)) {
      $options = array_merge(brush_get_context($context), $options);
      $options['config-file'] = realpath($config);

      //$options['site-aliases'] = array_merge(isset($aliases) ? $aliases : array(), isset($options['site-aliases']) ? $options['site-aliases'] : array());
      unset($options['site-aliases']);
      $options['command-specific'] = array_merge(isset($command_specific) ? $command_specific : array(), isset($options['command-specific']) ? $options['command-specific'] : array());

      brush_set_config_options($context, $options, $override);
    }
  }
}

function brush_set_config_options($context, $options, $override = array()) {
  global $brush_conf_override;

  // Only reset $brush_conf_override if the array is not set, otherwise keep old values and append new values to it.
  if (!isset($brush_conf_override)) {
    $brush_conf_override = array();
  }

  // Copy 'config-file' into 'context-path', converting to an array to hold multiple values if necessary
  if (isset($options['config-file'])) {
    if (isset($options['context-path'])) {
      $options['context-path'] = array_merge(array($options['config-file']), is_array($options['context-path']) ? $options['context-path'] : array($options['context-path']));
    }
    else {
      $options['context-path'] = $options['config-file'];
    }
  }

  // Take out $aliases and $command_specific options
  brush_set_config_special_contexts($options);

  brush_set_context($context, $options);

  // Instruct core not to store queries since we are not outputting them.
  // Don't run poormanscron during brush request.
  $defaults = array(
    'dev_query' => FALSE,
    'cron_safe_threshold' => 0,
  );
  foreach ($defaults as $key => $value) {
    // This can be overridden by a command or a brushrc file if needed.
    if (!isset($brush_conf_override[$key])) {
      $brush_conf_override[$key] = $value;
    }
  }

  /**
   * Allow the brushrc.php file to override $conf settings.
   * This is a separate variable because the $conf array gets
   * initialized to an empty array, in the backdrop bootstrap process,
   * and changes in settings.php would wipe out the brushrc.php settings.
   */
  if (!empty($override)) {
    $brush_conf_override = array_merge($brush_conf_override, $override);
  }
}

/**
 * There are certain options such as 'site-aliases' and 'command-specific'
 * that must be merged together if defined in multiple brush configuration
 * files.  If we did not do this merge, then the last configuration file
 * that defined any of these properties would overwrite all of the options
 * that came before in previously-loaded configuration files.  We place
 * all of them into their own context so that this does not happen.
 */
function brush_set_config_special_contexts(&$options) {
  if (isset($options)) {
    $has_command_specific = array_key_exists('command-specific', $options);
    // Change the keys of the site aliases from 'alias' to '@alias'
    if (array_key_exists('site-aliases', $options)) {
      $user_aliases = $options['site-aliases'];
      $options['site-aliases'] = array();
      foreach ($user_aliases as $alias_name => $alias_value) {
        if (substr($alias_name,0,1) != '@') {
          $alias_name = "@$alias_name";
        }
        $options['site-aliases'][$alias_name] = $alias_value;
      }
    }

    // Copy site aliases and command-specific options into their
    // appropriate caches.
    $special_contexts = brush_get_special_keys();
    foreach ($special_contexts as $option_name) {
      if (isset($options[$option_name])) {
        $cache =& brush_get_context($option_name);
        $cache = array_merge($cache, $options[$option_name]);
        unset($options[$option_name]);
      }
    }
    // If command-specific options were set and if we already have
    // a command, then apply the command-specific options immediately.
    if ($has_command_specific) {
      brush_command_default_options();
    }
  }
}

/**
 * Set a specific context.
 *
 * @param context
 *   Any of the default defined contexts.
 * @param value
 *   The value to store in the context
 *
 * @return
 *   An associative array of the settings specified in the request context.
 */
function brush_set_context($context, $value) {
  $cache =& brush_get_context($context);
  $cache = $value;
  return $value;
}


/**
 * Return a specific context, or the whole context cache
 *
 * This function provides a storage mechanism for any information
 * the currently running process might need to communicate.
 *
 * This avoids the use of globals, and constants.
 *
 * Functions that operate on the context cache, can retrieve a reference
 * to the context cache using :
 *     $cache = &brush_get_context($context);
 *
 * This is a private function, because it is meant as an internal
 * generalized API for writing static cache functions, not as a general
 * purpose function to be used inside commands.
 *
 * Code that modifies the reference directly might have unexpected consequences,
 * such as modifying the arguments after they have already been parsed and dispatched
 * to the callbacks.
 *
 * @param context
 *   Optional. Any of the default defined contexts.
 *
 * @return
 *   If context is not supplied, the entire context cache will be returned.
 *   Otherwise only the requested context will be returned.
 *   If the context does not exist yet, it will be initialized to an empty array.
 */
function &brush_get_context($context = NULL, $default = NULL) {
  static $cache = array();
  if (!is_null($context)) {
    if (!isset($cache[$context])) {
      $default = !is_null($default) ? $default : array();
      $cache[$context] = $default;
    }
    return $cache[$context];
  }
  return $cache;
}

/**
 * Set the arguments passed to the brush.php script.
 *
 * This function will set the 'arguments' context of the current running script.
 *
 * When initially called by brush_parse_args, the entire list of arguments will
 * be populated. Once the command is dispatched, this will be set to only the remaining
 * arguments to the command (i.e. the command name is removed).
 *
 * @param arguments
 *   Command line arguments, as an array.
 */
function brush_set_arguments($arguments) {
  brush_set_context('arguments', $arguments);
}

/**
 * Get the arguments passed to the brush.php script.
 *
 * When brush_set_arguments is initially called by brush_parse_args,
 * the entire list of arguments will be populated.
 * Once the command has been dispatched, this will be return only the remaining
 * arguments to the command.
 */
function brush_get_arguments() {
  return brush_get_context('arguments');
}

/**
 * Set the command being executed.
 *
 * Drop_dispatch will set the correct command based on it's
 * matching of the script arguments retrieved from brush_get_arguments
 * to the implemented commands specified by brush_get_commands.
 *
 * @param
 *   A numerically indexed array of command components.
 */
function brush_set_command($command) {
  brush_set_context('command', $command);
}

/**
 * Return the command being executed.
 *
 *
 */
function brush_get_command() {
  return brush_get_context('command');
}
/**
 * Get the value for an option.
 *
 * If the first argument is an array, then it checks whether one of the options
 * exists and return the value of the first one found. Useful for allowing both
 * -h and --host-name
 *
 * @param option
 *   The name of the option to get
 * @param default
 *   Optional. The value to return if the option has not been set
 * @param context
 *   Optional. The context to check for the option. If this is set, only this context will be searched.
 */
function brush_get_option($option, $default = NULL, $context = NULL) {
  $value = NULL;

  if ($context) {
    // We have a definite context to check for the presence of an option.
    $value = _brush_get_option($option, brush_get_context($context));
  }
  else {
    // We are not checking a specific context, so check them in a predefined order of precedence.
    $contexts = brush_context_names();

    foreach ($contexts as $context) {
      $value = _brush_get_option($option, brush_get_context($context));

      if ($value !== NULL) {
        return $value;
      }
    }
  }

  if ($value !== NULL) {
    return $value;
  }

  return $default;
}

/**
 * Get the value for an option and return it as a list.  If the
 * option in question is passed on the command line, its value should
 * be a comma-separated list (e.g. --flag=1,2,3).  If the option
 * was set in a brushrc.php file, then its value may be either a
 * comma-separated list or an array of values (e.g. $option['flag'] = array('1', '2', '3')).
 *
 * @param option
 *   The name of the option to get
 * @param default
 *   Optional. The value to return if the option has not been set
 * @param context
 *   Optional. The context to check for the option. If this is set, only this context will be searched.
 */
function brush_get_option_list($option, $default = array(), $context = NULL) {
  $result = brush_get_option($option, $default, $context);

  if (!is_array($result)) {
    $result = explode(',', $result);
  }

  return $result;
}

/**
 * Get the value for an option, but first checks the provided option overrides.
 *
 * The feature of brush_get_option that allows a list of option names
 * to be passed in an array is NOT supported.
 *
 * @param option_overrides
 *   An array to check for values before calling brush_get_option.
 * @param option
 *   The name of the option to get.
 * @param default
 *   Optional. The value to return if the option has not been set.
 * @param context
 *   Optional. The context to check for the option. If this is set, only this context will be searched.
 *
 */
function brush_get_option_override($option_overrides, $option, $value = NULL, $context = NULL) {
  if (array_key_exists($option, $option_overrides)) {
    return $option_overrides[$option];
  }
  else {
    return brush_get_option($option, $value, $context);
  }
}

/**
 * Get all of the values for an option in every context.
 *
 * @param option
 *   The name of the option to get
 * @return
 *   An array whose key is the context name and value is
 *   the specific value for the option in that context.
 */
function brush_get_context_options($option, $flatten = FALSE) {
  $result = array();

  $contexts = brush_context_names();
  foreach ($contexts as $context) {
    $value = _brush_get_option($option, brush_get_context($context));

    if ($value !== NULL) {
      if ($flatten && is_array($value)) {
        $result = array_merge($value, $result);
      }
      else {
        $result[$context] = $value;
      }
    }
  }

  return $result;
}

/**
 * Retrieves a collapsed list of all options
 */
function brush_get_merged_options() {
  $contexts = brush_context_names();
  $cache = brush_get_context();
  $result = array();
  foreach (array_reverse($contexts) as $context) {
    if (array_key_exists($context, $cache)) {
      $result = array_merge($result, $cache[$context]);
    }
  }

  return $result;
}

/**
 * Helper function to recurse through possible option names
 */
function _brush_get_option($option, $context) {
  if (is_array($option)) {
    foreach ($option as $current) {
      if (array_key_exists($current, $context)) {
        return $context[$current];
      }
    }
  }
  elseif (array_key_exists($option, $context)) {
    return $context[$option];
  }

  return NULL;
}

/**
 * Set an option in one of the option contexts.
 *
 * @param option
 *   The option to set.
 * @param value
 *   The value to set it to.
 * @param context
 *   Optional. Which context to set it in.
 * @return
 *   The value parameter. This allows for neater code such as
 *     $myvalue = brush_set_option('http_host', $_SERVER['HTTP_HOST']);
 *   Without having to constantly type out the value parameter.
 */
function brush_set_option($option, $value, $context = 'process') {
  $cache =& brush_get_context($context);
  $cache[$option] = $value;
  return $value;
}

/**
 * A small helper function to set the value in the default context
 */
function brush_set_default($option, $value) {
  return brush_set_option($option, $value, 'default');
}

/**
 * Remove a setting from a specific context.
 *
 * @param
 *   Option to be unset
 * @param
 *   Context in which to unset the value in.
 */
function brush_unset_option($option, $context = NULL) {
  if ($context != NULL) {
    $cache =& brush_get_context($context);
    if (array_key_exists($option, $cache)) {
      unset($cache[$option]);
    }
  }
  else {
    $contexts = brush_context_names();

    foreach ($contexts as $context) {
      brush_unset_option($option, $context);
    }
  }
}

/**
 * Save the settings in a specific context to the applicable configuration file
 * This is useful is you want certain settings to be available automatically the next time a command is executed.
 *
 * @param $context
 *   The context to save
 */
function brush_save_config($context) {
  $filename = _brush_config_file($context);

  if ($filename) {
    $cache = brush_get_context($context);

    $fp = fopen($filename, "w+");
    if (!$fp) {
      return brush_set_error('BRUSH_PERM_ERROR', dt('Brushrc (!filename) could not be written', array('!filename' => $filename)));
    }
    else {
      fwrite($fp, "<?php\n");
      $timestamp = mktime();
      foreach ($cache as $key => $value) {
        $line = "\n\$options['$key'] = ". var_export($value, TRUE) .';';
        fwrite($fp, $line);
      }
      fwrite($fp, "\n");
      fclose($fp);
      brush_log(dt('Brushrc file (!filename) was written successfully', array('!filename' => $filename)));
      return TRUE;
    }

  }
  return FALSE;
}
