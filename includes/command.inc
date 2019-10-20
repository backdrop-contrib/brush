<?php

/**
 * @defgroup dispatching Command dispatching functions.
 * @{
 *
 * These functions handle command dispatching, and can
 * be used to programatically invoke drop commands in
 * different ways.
 */

/**
 * Invoke drop api calls.
 *
 * Executes the specified command with the specified arguments on the
 * currently bootstrapped site using the current option contexts.
 * Note that drop_invoke will not bootstrap any further than the
 * current command has already bootstrapped; therefore, you should only invoke
 * commands that have the same (or lower) bootstrap requirements.
 *
 * Call the correct hook for all the modules that implement it.
 * Additionally, the ability to rollback when an error has been encountered is also provided.
 * If at any point during execution, the drop_get_error() function returns anything but 0,
 * drop_invoke() will trigger $hook_rollback for each of the hooks that implement it,
 * in reverse order from how they were executed.  Rollbacks are also triggered any
 * time a hook function returns FALSE.
 *
 * This function will also trigger pre_$hook and post_$hook variants of the hook
 * and its rollbacks automatically.
 *
 * HOW DROP HOOK FUNCTIONS ARE NAMED:
 *
 * The name of the hook is composed from the name of the command and the name of
 * the command file that the command definition is declared in.  The general
 * form for the hook filename is:
 *
 *      drop_COMMANDFILE_COMMANDNAME
 *
 * In many cases, drop commands that are functionally part of a common collection
 * of similar commands will all be declared in the same file, and every command
 * defined in that file will start with the same command prefix.  For example, the
 * command file "pm.drop.inc" defines commands such as "pm-enable" and "pm-disable".
 * In the case of "pm-enable", the command file is "pm", and and command name is
 * "pm-enable".  When the command name starts with the same sequence of characters
 * as the command file, then the repeated sequence is dropped; thus, the command
 * hook for "pm-enable" is "drop_pm_enable", not "drop_pm_pm_enable".
 *
 * @param command
 *   The drop command to execute.
 * @return
 *   A boolean specifying whether or not the command was successfully completed.
 */
function drop_invoke($command) {
  $args = func_get_args();
  array_shift($args);

  return drop_invoke_args($command, $args);
}

/**
 * As drop_invoke, but args are passed in as an array
 * rather than as individual function parameters.
 *
 * @see drop_invoke()
 */
function drop_invoke_args($command, $args) {
  return _drop_invoke_args($command, $args, drop_get_commandfile_for_command($command));
}

/**
 * Variant of drop_invoke_args that allows the
 * commandfile to be specified correctly; this allows
 * the hook functions to be correctly determined for
 * commands that provide a 'callback' function.
 *
 * This function should not be called directly; use
 * @see drop_dispatch() and @see drop_invoke_args()
 */
function _drop_invoke_args($command, $args, $defined_in_commandfile) {
  drop_command_include($command);
  // Generate the base name for the hook by converting all
  // dashes in the command name to underscores.
  $hook = str_replace("-", "_", $command);

  // Call the hook init function, if it exists.
  // If a command needs to bootstrap, it is advisable
  // to do so in _init; otherwise, new commandfiles
  // will miss out on participating in any stage that
  // has passed or started at the time it was discovered.
  $func = 'drop_' . $hook . '_init';
  if (function_exists($func)) {
    drop_log(dt("Calling drop command init function: !func", array('!func' => $func)), 'bootstrap');
    call_user_func_array($func, $args);
    _drop_log_backdrop_messages();
    if (drop_get_error()) {
      drop_log(dt('The command @command could not be initialized.', array('@command' => $command)), 'error');
      return FALSE;
    }
  }

  $rollback = FALSE;
  $completed = array();
  $available_rollbacks = array();
  $all_available_hooks = array();

  // Iterate through the different hook variations
  $variations = array($hook . "_validate", "pre_$hook", $hook, "post_$hook");
  foreach ($variations as $var_hook) {
    // Get the list of command files.
    // We re-fetch the list every time through
    // the loop in case one of the hook function
    // does something that will add additional
    // commandfiles to the list (i.e. bootstrapping
    // to a higher phase will do this).
    $list = drop_commandfile_list();

    // Run all of the functions available for this variation
    foreach ($list as $commandfile => $filename) {
      $func = sprintf("drop_%s_%s", $commandfile, $var_hook);
      if (($defined_in_commandfile == $commandfile) && ($commandfile . "_" == substr($var_hook . "_",0,strlen($commandfile)+ 1))) {
        $oldfunc = $func;
        $func = sprintf("drop_%s", $var_hook);
        if (($oldfunc != $func) && (function_exists($oldfunc))) {
          drop_log(dt("The drop command hook naming conventions changed in March 2010; the function !oldfunc must be renamed to !func in order to be called.", array('!oldfunc' => $oldfunc, '!func' => $func)), "error");
        }
      }
      if (function_exists($func)) {
        $all_available_hooks[] = $func . ' [* Defined in ' . $filename . ']';
        $available_rollbacks[] = $func . '_rollback';
        $completed[] = $func;
        $result = call_user_func_array($func, $args);
        // Only the 'main' callback can send data to backend.
        if ($var_hook == $hook) {
          // If the hook already called drop_backend_set_result,
          // then return that value. If it did not, then the return
          // value from the hook will be the value returned from
          // this routine.
          $return = drop_backend_get_result();
          if (empty($return)) {
            drop_backend_set_result($result);
            $return = $result;
          }
        }
        _drop_log_backdrop_messages();
        if (drop_get_error() || ($result === FALSE)) {
          $rollback = TRUE;
          // break out of the foreach variations and foreach list
          break 2;
        }
      }
      else {
        $all_available_hooks[] = $func;
      }
    }
  }

  // If no hook functions were found, print a warning.
  if (empty($completed)) {
    $default_command_hook = sprintf("drop_%s_%s", $defined_in_commandfile, $hook);
    if (($defined_in_commandfile . "_" == substr($hook . "_",0,strlen($defined_in_commandfile)+ 1))) {
      $default_command_hook = sprintf("drop_%s", $hook);
    }
    drop_log(dt("No hook functions were found for !command.", array('!command' => $command)), 'warning');
    drop_log(dt("The primary drop_invoke() hook for !command is !default_func().  Please implement this function.  Run with --show-invoke to see all available hooks.", array('!command' => $command, '!default_func' => $default_command_hook)), 'warning');
  }
  if (drop_get_option('show-invoke')) {
    // We show all available hooks up to and including the one that failed (or all, if there were no failures)
    drop_log(dt("Available drop_invoke() hooks for !command: !available", array('!command' => $command, '!available' => "\n" . implode("\n", $all_available_hooks))), 'ok');
  }
  if (drop_get_option('show-invoke') && !empty($available_rollbacks)) {
    drop_log(dt("Available rollback hooks for !command: !rollback", array('!command' => $command, '!rollback' => "\n" . implode("\n", $available_rollbacks))), 'ok');
  }

  // something went wrong, we need to undo
  if ($rollback) {
    if (drop_get_option('confirm-rollback', FALSE)) {
      // Optionally ask for confirmation, --yes and --no are ignored from here on as we are about to finish this process.
      drop_set_context('DROP_AFFIRMATIVE', FALSE);
      drop_set_context('DROP_NEGATIVE', FALSE);
      $rollback = drop_confirm(dt('Do you want to rollback? (manual cleanup might be required otherwise)'));
    }

    if ($rollback) {
      foreach (array_reverse($completed) as $func) {
        $rb_func = $func . '_rollback';
        if (function_exists($rb_func)) {
          call_user_func_array($rb_func, $args);
          _drop_log_backdrop_messages();
          drop_log(dt("Changes made in !func have been rolled back.", array('!func' => $func)), 'rollback');
        }
      }
    }
  }

  return !$rollback;
}

/**
 * Given a command record, dispatch it as if it were
 * the original command.  Executes in the currently
 * bootstrapped site using the current option contexts.
 * Note that drop_dispatch will not bootstrap any further than the
 * current command has already bootstrapped; therefore, you should only invoke
 * commands that have the same (or lower) bootstrap requirements.
 *
 * @param command
 *   A full $command such as returned by drop_get_commands().
 * @param arguments
 *   An array of argument values.
 *
 * @see drop_topic_docs_topic().
 */
function drop_dispatch($command, $arguments = array()) {
  drop_set_command($command);
  $return = FALSE;

  // Set warning for Windows users. We have already loaded site-specific droprc.
  drop_environment_check_os();

  if ($command) {
    // Add arguments, if this has not already been done.
    // (If the command was fetched from drop_parse_command,
    // then you cannot provide arguments to drop_dispatch.)
    if (empty($command['arguments'])) {
      _drop_prepare_command($command, $arguments);
    }

    // Add command-specific options, if applicable
    drop_command_default_options($command);

    // Print a warning if someone tries to use a deprecated alias.
    if (isset($command['deprecated'])) {
      drop_log(dt('Warning: The command name "!deprecated" is deprecated.  Please use a recommended form instead (!recommended).', array('!deprecated' => $command['deprecated-name'], '!recommended' => implode(',', array_merge(array($command['command']), $command['aliases'])))), 'warning');
    }
    // Call the callback function of the active command.
    $return = call_user_func_array($command['callback'], $command['arguments']);
  }

  // Add a final log entry, just so a timestamp appears.
  drop_log(dt('Command dispatch complete'), 'notice');

  return $return;
}

/**
 * Invoke a command in a new process, targeting the site specified by
 * the provided site alias record.
 *
 * @param array $site_alias_record
 *  The site record to execute the command on.
 * @param string $command_name
 *  The command to invoke.
 * @param array $commandline_args
 *  The arguments to pass to the command.
 * @param array $commandline_options
 *  The options (e.g. --select) to provide to the command.
 * @return
 *   If the command could not be completed successfully, FALSE.
 *   If the command was completed, this will return an associative
 *   array containing the results of the API call.
 *   @see drop_backend_get_result()
 *
 * This function may also be called via its deprecated function signature
 * (drop-3.x compatible): drop_invoke_process($command_name, $arg1, $arg2, ...);
 * Instead of this old form, prefer: drop_invoke_process("@self", $command_name, array($arg1, $arg2, ...));
 */
function drop_invoke_process($site_alias_record /*, $command_name, $commandline_args = array(), $commandline_options = array(), $backend_options = array() */) {
  if (is_array($site_alias_record) || ($site_alias_record[0] == '@')) {
    $args = func_get_args();
    array_shift($args);
    $command_name = array_shift($args);
    $commandline_args = empty($args) ? array() : array_shift($args);
    $commandline_options = empty($args) ? array() : array_shift($args);
    // Forwards-compatibility with drop_invoke_process in Drop-5.x.
    $backend_options = empty($args) ? array() : array_shift($args);
    foreach ($backend_options as $key => $value) {
      $commandline_options['#' . $key] = $value;
    }
    $integrate = TRUE;
    return drop_invoke_sitealias_args($site_alias_record, $command_name, $commandline_args, $commandline_options, $integrate);
  }
  else {
    $args = func_get_args();
    $command_name = array_shift($args);
    return drop_invoke_process_args($command_name, $args);
  }
}

/**
 * Invoke a command in a new process.
 *
 * @param command_name
 *   The drop command to execute.
 * @return
 *   If the command could not be completed successfully, FALSE.
 *   If the command was completed, this will return an associative
 *   array containing the results of the API call.
 *   @see drop_backend_get_result()
 * @deprecated; @see drop_invoke_process("@self", $command_name, $commandline_args, $commandline_options) for a better option
 */
function drop_invoke_process_args($command_name, $commandline_args, $commandline_options = array()) {
  return drop_backend_invoke_args($command_name, $commandline_args, $commandline_options);
}

/**
 * Invoke a command in a new process, targeting the site specified by
 * the provided site alias record.
 *
 * @param array $site_alias_record
 *  The site record to execute the command on.
 * @param string $command_name
 *  The command to invoke.
 * @return
 *   If the command could not be completed successfully, FALSE.
 *   If the command was completed, this will return an associative
 *   array containing the results of the API call.
 *   @see drop_backend_get_result()
 * @deprecated; @see  drop_invoke_process($site_alias_record, $command_name, array($arg1, $arg2, ...)) for a better option
 */
function drop_invoke_sitealias($site_alias_record, $command_name) {
  $args = func_get_args();
  array_shift($args);
  array_shift($args);

  return drop_invoke_sitealias_args($site_alias_record, $command_name, $args);
}

/**
 * Invoke a command in a new process, targeting the site specified by
 * the provided site alias record.
 *
 * @param array $site_alias_record
 *  The site record to execute the command on.
 * @param string $command_name
 *  The command to invoke.
 * @param array $commandline_args
 *  The arguments to pass to the command.
 * @param array $commandline_options
 *  The options (e.g. --select) to provide to the command.
 * @param integrate
 *  @see drop_backend_invoke
 * @return
 *   If the command could not be completed successfully, FALSE.
 *   If the command was completed, this will return an associative
 *   array containing the results of the API call.
 *   @see drop_backend_get_result()
 * @deprecated; @see drop_invoke_process($site_alias_record, $command_name, $commandline_args, $commandline_options) for a better option
 */
function drop_invoke_sitealias_args($site_alias_record, $command_name, $commandline_args, $commandline_options = array(), $integrate = FALSE) {
  // If the first parameter is not a site alias record,
  // then presume it is an alias name, and try to look up
  // the alias record.
  if (!is_array($site_alias_record)) {
    $site_alias_record = drop_sitealias_get_record($site_alias_record);
  }
  return drop_do_site_command($site_alias_record, $command_name, $commandline_args, $commandline_options, $integrate);
}

/**
 * Get the options that were passed to the current command.
 *
 * This function returns an array that contains all of the options
 * that are appropriate for forwarding along to one of the drop_invoke_*_args
 * functions.
 *
 * @return
 *   An associative array of option key => value pairs.
 */
function drop_redispatch_get_options() {
  // Start off by taking everything from the site alias and command line
  // ('cli' context)
  $options = array_merge(drop_get_context('alias'), drop_get_context('cli'));
  $options = array_diff_key($options, array_flip(drop_sitealias_site_selection_keys()));
  unset($options['command-specific']);
  unset($options['path-aliases']);
  // If we can parse the current command, then examine all contexts
  // in order for any option that is directly related to the current command
  $command = drop_parse_command();
  if (is_array($command)) {
    foreach ($command['options'] as $key => $value) {
      // Strip leading --
      $key = ltrim($key, '-');
      $value = drop_get_option($key);
      if (isset($value)) {
        $options[$key] = $value;
      }
    }
  }
  // 'php', if needed, will be included in DROP_COMMAND.  If DROP_COMMAND
  // is not used (e.g. when calling a remote instance of drop), then --php
  // should not be passed along.
  unset($options['php']);
  // If --bootstrap-to-first-arg is specified, do not
  // pass it along to remote commands.
  unset($options['bootstrap-to-first-arg']);

  return $options;
}

/**
 * @} End of "defgroup dispatching".
 */

/**
 * @file
 * The drop command engine.
 *
 * Since drop can be invoked independently of a proper Backdrop
 * installation and commands may operate across sites, a distinct
 * command engine is needed.
 *
 * It mimics the Backdrop module engine in order to economize on
 * concepts and to make developing commands as familiar as possible
 * to traditional Backdrop module developers.
 */

/**
 * Parse console arguments.
 */
function drop_parse_args() {
  $args = drop_get_context('argv');
  // TODO: commandfiles should be able to extend this list.
  static $arg_opts = array('c', 'u', 'r', 'l', 'i');

  // Check to see if we were executed via a "#!/usr/bin/env drop" script
  drop_adjust_args_if_shebang_script($args);

  // Now process the command line arguments.  We will divide them
  // into options (starting with a '-') and arguments.
  $arguments = $options = array();

  for ($i = 1; $i < count($args); $i++) {
    $opt = $args[$i];
    // Is the arg an option (starting with '-')?
    if ($opt{0} == "-" && strlen($opt) != 1) {
      // Do we have multiple options behind one '-'?
      if (strlen($opt) > 2 && $opt{1} != "-") {
        // Each char becomes a key of its own.
        for ($j = 1; $j < strlen($opt); $j++) {
          $options[substr($opt, $j, 1)] = true;
        }
      }
      // Do we have a longopt (starting with '--')?
      elseif ($opt{1} == "-") {
        if ($pos = strpos($opt, '=')) {
          $options[substr($opt, 2, $pos - 2)] = substr($opt, $pos + 1);
        }
        else {
          $options[substr($opt, 2)] = true;
        }
      }
      else {
        $opt = substr($opt, 1);
        // Check if the current opt is in $arg_opts (= has to be followed by an argument).
        if ((in_array($opt, $arg_opts))) {
          if (($args[$i+1] == NULL) || ($args[$i+1] == "") || ($args[$i + 1]{0} == "-")) {
            drop_set_error('DROP_INVALID_INPUT', "Invalid input: -$opt needs to be followed by an argument.");
          }
          $options[$opt] = $args[$i + 1];
          $i++;
        }
        else {
          $options[$opt] = true;
        }
      }
    }
    // If it's not an option, it's a command.
    else {
      $arguments[] = $opt;
    }
  }
  // If no arguments are specified, then the command will
  // be either 'help' or 'version' (the later if --version is specified)
  if (!sizeof($arguments)) {
    if (array_key_exists('version', $options)) {
      $arguments = array('version');
    }
    else {
      $arguments = array('help');
    }
  }

  // Handle the "@shift" alias, if present
  drop_process_bootstrap_to_first_arg($arguments);

  drop_set_arguments($arguments);
  drop_set_context('cli', $options);
}

/**
 * Get the short commandfile name that matches the
 * command.
 *
 * @param $command
 *   The name of the command (e.g. search-index)
 * @return String
 *   The short commandfile name where that command was
 *   defined (e.g. search, not search.drop.inc)
 */
function drop_get_commandfile_for_command($command) {
  $commandfile = FALSE;

  $commands = drop_get_commands();
  if (array_key_exists($command, $commands)) {
    $commandfile = $commands[$command]['commandfile'];
  }

  return $commandfile;
}

/**
 * Pop an argument off of drop's argument list
 */
function drop_shift() {
  $arguments = drop_get_arguments();
  $result = NULL;
  if (!empty($arguments)) {
    // The php-script command uses the DROP_SHIFT_SKIP
    // context to cause drop_shift to skip the 'php-script'
    // command and the script path argument when it is
    // called from the user script.
    $skip_count = drop_get_context('DROP_SHIFT_SKIP');
    if (is_numeric($skip_count)) {
      for ($i = 0; $i < $skip_count; $i++) {
        array_shift($arguments);
      }
      $skip_count = drop_set_context('DROP_SHIFT_SKIP', 0);
    }
    $result = array_shift($arguments);
    drop_set_arguments($arguments);
  }
  return $result;
}

/**
 * Special checking for "shebang" script handling.
 *
 * If there is a file 'script.php' that begins like so:
 *   #!/path/to/drop
 * Then $args will be:
 *   /path/to/drop /path/to/script userArg1 userArg2 ...
 * If it instead starts like this:
 *   #!/path/to/drop --flag php-script
 * Then $args will be:
 *   /path/to/drop "--flag php-script" /path/to/script userArg1 userArg2 ...
 * (Note that execve does not split the parameters from
 * the shebang line on whitespace; see http://en.wikipedia.org/wiki/Shebang_%28Unix%29)
 * When drop is called via one of the "shebang" lines above,
 * the first or second parameter will be the full path
 * to the "shebang" script file -- and if the path to the
 * script is in the second position, then we will expect that
 * the argument in the first position must begin with a
 * '@' (alias) or '-' (flag).  Under ordinary circumstances,
 * we do not expect that the drop command must come before
 * any argument that is the full path to a file.  We use
 * this assumption to detect "shebang" script execution.
 */
function drop_adjust_args_if_shebang_script(&$args) {
  if (_drop_is_drop_shebang_script($args[1])) {
    // If $args[1] is a drop "shebang" script, we will insert
    // the option "--bootstrap-to-first-arg" and the command
    // "php-script" at the beginning of  @args, so the command
    // line args become:
    //   /path/to/drop --bootstrap-to-first-arg php-script /path/to/script userArg1 userArg2 ...
    drop_set_option('bootstrap-to-first-arg', TRUE);
    array_splice($args, 1, 0, array('php-script'));
    drop_set_context('DROP_SHEBANG_SCRIPT', TRUE);
  }
  elseif (((strpos($args[1], ' ') !== FALSE) || (!ctype_alnum($args[1][0]))) && (_drop_is_drop_shebang_script($args[2]))) {
    // If $args[2] is a drop "shebang" script, we will insert
    // the space-exploded $arg[1] in place of $arg[1], so the
    // command line args become:
    //   /path/to/drop scriptArg1 scriptArg2 ... /path/to/script userArg1 userArg2 ...
    // If none of the script arguments look like a drop command,
    // then we will insert "php-script" as the default command to
    // execute.
    $script_args = explode(' ', $args[1]);
    $has_command = FALSE;
    foreach ($script_args as $script_arg) {
      if (preg_match("/^[a-z][a-z0-9-]*$/",$script_arg)) {
        $has_command = TRUE;
      }
    }
    if (!$has_command) {
      $script_args[] = 'php-script';
    }
    array_splice($args, 1, 1, $script_args);
    drop_set_context('DROP_SHEBANG_SCRIPT', TRUE);
  }
}

/**
 * Process the --bootstrap-to-first-arg option, if it is present.
 *
 * This option checks to see if the first user-provided argument is an alias
 * or site specification; if it is, it will be shifted into the first argument
 * position, where it will specify the site to bootstrap. The result of this
 * is that if your shebang line looks like this:
 *
 * #!/path/to/drop --bootstrap-to-first-arg php-script
 *
 * Then when you run that script, you can optionally provide an alias such
 * as @dev as the first argument (e.g. $ ./mydropscript.php @dev scriptarg1
 * scriptarg2). Since this is the behavior that one would usually want,
 * it is default behavior for a canonical script. That is, a script
 * with a simple shebang line, like so:
 *
 * #!/path/to/drop
 *
 * will implicitly have "--bootstrap-to-first-arg" and "php-script" prepended, and will therefore
 * behave exactly like the first example. To write a script that does not
 * use --bootstrap-to-first-arg, then the drop command or at least one flag must be explicitly
 * included, like so:
 *
 * #!/path/to/drop php-script
 */
function drop_process_bootstrap_to_first_arg(&$arguments) {
  if (drop_get_option('bootstrap-to-first-arg', FALSE)) {
    $shift_alias_pos = 1 + (drop_get_context('DROP_SHEBANG_SCRIPT') === TRUE);
    if (sizeof($arguments) >= $shift_alias_pos) {
      $shifted_alias = $arguments[$shift_alias_pos];
      $alias_record = drop_sitealias_get_record($shifted_alias);
      if (!empty($alias_record)) {
        // Move the alias we shifted from its current position
        // in the argument list to the front of the list
        array_splice($arguments, $shift_alias_pos, 1);
        array_unshift($arguments, $shifted_alias);
      }
    }
  }
}

/**
 * Get a list of all implemented commands.
 * This invokes hook_drop_command().
 *
 * @return
 *   Associative array of currently active command descriptors.
 *
 */
function drop_get_commands() {
  $commands = $available_commands = array();
  $list = drop_commandfile_list();
  foreach ($list as $commandfile => $path) {
    if (drop_command_hook($commandfile, 'drop_command')) {
      $function = $commandfile . '_drop_command';
      $result = $function();
      foreach ((array)$result as $key => $command) {
        // Add some defaults and normalize the command descriptor
        $command += drop_command_defaults($key, $commandfile, $path);
        // Translate command.
        drop_command_translate($command);
        // If command callback function name begins with "drop_$commandfile_",
        // then fix up the command entry so that drop_invoke will be
        // called by way of drop_command.  This will cause all
        // of the applicable hook functions to be called for the
        // command when it is invoked.  If the callback function does
        // -not- begin with its commandfile name, then it will be
        // called directly by drop_dispatch, and no hook functions
        // will be called (e.g. you cannot hook drop_print_file).
        if ($command['callback'] != 'drop_command') {
          $required_command_prefix = 'drop_' . $commandfile . '_';
          if ((substr($command['callback'], 0, strlen($required_command_prefix)) == $required_command_prefix)) {
            $command['command-hook'] = substr($command['callback'], strlen('drop_'));
            $command['callback'] = 'drop_command';
          }
        }

        $commands[$key] = $command;
        // For every alias, make a copy of the command and store it in the command list
        // using the alias as a key
        if (isset($command['aliases']) && count($command['aliases'])) {
          foreach ($command['aliases'] as $alias) {
            $commands[$alias] = $command;
            $commands[$alias]['is_alias'] = TRUE;
          }
        }
        // Do the same operation on the deprecated aliases.
        if (isset($command['deprecated-aliases']) && count($command['deprecated-aliases'])) {
          foreach ($command['deprecated-aliases'] as $alias) {
            $commands[$alias] = $command;
            $commands[$alias]['is_alias'] = TRUE;
            $commands[$alias]['deprecated'] = TRUE;
            $commands[$alias]['deprecated-name'] = $alias;
          }
        }

      }
    }
  }

  return drop_set_context('DROP_COMMANDS', $commands);
}

function drop_command_defaults($key, $commandfile, $path) {
  return array(
    'command' => $key,
    'command-hook' => $key,
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_LOGIN,
    'callback arguments' => array(),
    'commandfile' => $commandfile,
    'path' => dirname($path),
    'engines' => array(), // Helpful for drop_show_help().
    'callback' => 'drop_command',
    'description' => NULL,
    'sections' => array(
      'examples' => 'Examples',
      'arguments' => 'Arguments',
      'options' => 'Options',
    ),
    'arguments' => array(),
    'options' => array(),
    'examples' => array(),
    'aliases' => array(),
    'deprecated-aliases' => array(),
    'core' => array(),
    'scope' => 'site',
    'backdrop dependencies' => array(),
    'drop dependencies' => array(),
    'bootstrap_errors' => array(),
    'topics' => array(),
    'hidden' => FALSE,
  );
}

/**
 * Translates description and other keys of a command definition.
 *
 * @param $command
 *   A command definition.
 */
function drop_command_translate(&$command) {
  $command['description'] = _drop_command_translate($command['description']);
  $keys = array('arguments', 'options', 'examples', 'engines', 'sections');
  foreach ($keys as $key) {
    foreach ($command[$key] as $k => $v) {
      $command[$key][$k] = _drop_command_translate($v);
    }
  }
}

/**
 * Helper function for drop_command_translate().
 *
 * @param $source
 *   String or array.
 */
function _drop_command_translate($source) {
  return is_array($source) ? call_user_func_array('dt', $source) : dt($source);
}

/**
 * Matches a commands array, as returned by drop_get_arguments, with the
 * current command table.
 *
 * Note that not all commands may be discoverable at the point-of-call,
 * since Backdrop modules can ship commands as well, and they are
 * not available until after bootstrapping.
 *
 * drop_parse_command returns a normalized command descriptor, which
 * is an associative array. Some of its entries are:
 * - callback arguments: an array of arguments to pass to the calback.
 * - description: description of the command.
 * - arguments: an array of arguments that are understood by the command. for help texts.
 * - options: an array of options that are understood by the command. for help texts.
 * - examples: an array of examples that are understood by the command. for help texts.
 * - scope: one of 'system', 'project', 'site'.
 * - bootstrap: backdrop bootstrap level (depends on Backdrop major version). -1=no_bootstrap.
 * - core: Backdrop major version required.
 * - backdrop dependencies: backdrop modules required for this command.
 * - drop dependencies: other drop command files required for this command.
 *
 */
function drop_parse_command() {
  $args = drop_get_arguments();
  $command = FALSE;

  // Get a list of all implemented commands.
  $implemented = drop_get_commands();
  if (isset($implemented[$args[0]])) {
    $command = $implemented[$args[0]];
    $arguments = array_slice($args, 1);
  }

  // We have found a command that matches. Set the appropriate values.
  if ($command) {
    // Special case. Force help command if --help option was specified.
    if (drop_get_option(array('h', 'help'))) {
      $arguments = array($command['command']);
      $command = $implemented['help'];
      $command['arguments'] = $arguments;
    }
    else {
      _drop_prepare_command($command, $arguments);
    }
    drop_set_command($command);
  }
  return $command;
}

/*
 * Called by drop_parse_command.  If a command is dispatched
 * directly by drop_dispatch, then drop_dispatch will call
 * this function.
 */
function _drop_prepare_command(&$command, $arguments = array()) {
  // Merge specified callback arguments, which precede the arguments passed on the command line.
  if (isset($command['callback arguments']) && is_array($command['callback arguments'])) {
    $arguments = array_merge($command['callback arguments'], $arguments);
  }
  $command['arguments'] = $arguments;
}

/**
 * Entry point for commands into the drop_invoke API
 *
 * If a command does not have a callback specified, this function will be called.
 *
 * This function will trigger $hook_drop_init, then if no errors occur,
 * it will call drop_invoke() with the command that was dispatch.
 *
 * If no errors have occured, it will run $hook_drop_exit.
 */
function drop_command() {
  $args = func_get_args();
  $command = drop_get_command();

  foreach (drop_command_implements("drop_init") as $name) {
    $func = $name . '_drop_init';
    drop_log(dt("Initializing drop commandfile: !name", array('!name' => $name)), 'bootstrap');
    call_user_func_array($func, $args);
    _drop_log_backdrop_messages();
  }

  if (!drop_get_error()) {
    _drop_invoke_args($command['command-hook'], $args, $command['commandfile']);
  }

  if (!drop_get_error()) {
    foreach (drop_command_implements('drop_exit') as $name) {
      $func = $name . '_drop_exit';
      call_user_func_array($func, $args);
      _drop_log_backdrop_messages();
    }
  }
}



/**
 * Invoke a hook in all available command files that implement it.
 *
 * @see drop_command_invoke_all_ref()
 *
 * @param $hook
 *   The name of the hook to invoke.
 * @param ...
 *   Arguments to pass to the hook.
 * @return
 *   An array of return values of the hook implementations. If commands return
 *   arrays from their implementations, those are merged into one array.
 */
function drop_command_invoke_all() {
  $args = func_get_args();
  if (count($args) == 1) {
    $args[] = NULL;
  }
  $reference_value = $args[1];
  $args[1] = &$reference_value;

  return call_user_func_array('drop_command_invoke_all_ref', $args);
}

/**
 * A drop_command_invoke_all() that wants the first parameter to be passed by reference.
 *
 * @see drop_command_invoke_all()
 */
function drop_command_invoke_all_ref($hook, &$reference_parameter) {
  $args = func_get_args();
  array_shift($args);
  // Insure that call_user_func_array can alter first parameter
  $args[0] = &$reference_parameter;
  $return = array();
  foreach (drop_command_implements($hook) as $module) {
    $function = $module .'_'. $hook;
    $result = call_user_func_array($function, $args);
    if (isset($result) && is_array($result)) {
      $return = array_merge_recursive($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
  }
  return $return;
}

/**
 * Determine which command files are implementing a hook.
 *
 * @param $hook
 *   The name of the hook (e.g. "drop_help" or "drop_command").
 *
 * @return
 *   An array with the names of the command files which are implementing this hook.
 */
function drop_command_implements($hook) {
  $implementations[$hook] = array();
  $list = drop_commandfile_list();
  foreach ($list as $commandfile => $file) {
    if (drop_command_hook($commandfile, $hook)) {
      $implementations[$hook][] = $commandfile;
    }
  }
  return (array)$implementations[$hook];
}

/**
 * @param string
 *   name of command to check.
 *
 * @return boolean
 *   TRUE if the given command has an implementation.
 */
function drop_is_command($command) {
  $commands = drop_get_commands();
  return isset($commands[$command]);
}

/**
 * Collect a list of all available drop command files.
 *
 * Scans the following paths for drop command files:
 *
 * - The "/path/to/drop/commands" folder.
 * - Folders listed in the 'include' option (see example.droprc.php).
 * - The system-wide drop commands folder, e.g. /usr/share/drop/commands
 * - The ".drop" folder in the user's HOME folder.
 * - All modules in the current Backdrop installation whether they are enabled or
 *   not. Commands implementing hook_drop_load() in MODULE.drop.load.inc with
 *   a return value FALSE will not be loaded.
 *
 * A drop command file is a file that matches "*.drop.inc".
 *
 * @see drop_scan_directory()
 *
 * @return
 *   An associative array whose keys and values are the names of all available
 *   command files.
 */
function drop_commandfile_list() {
  return drop_get_context('DROP_COMMAND_FILES', array());
}

function _drop_find_commandfiles($phase, $phase_max = FALSE) {
  if (!$phase_max) {
    $phase_max = $phase;
  }

  $searchpath = array();
  switch ($phase) {
    case DROP_BOOTSTRAP_DROP:
      // Core commands shipping with drop
      $searchpath[] = realpath(dirname(__FILE__) . '/../commands/');

      // User commands, specified by 'include' option
      if ($include = drop_get_option(array('i', 'include'), FALSE)) {
        foreach (explode(PATH_SEPARATOR, $include) as $path) {
          $searchpath[] = $path;
        }
      }

      // System commands, residing in $SHARE_PREFIX/share/drop/commands
      $share_path = drop_get_context('SHARE_PREFIX', '/usr') . '/share/drop/commands';

      if (is_dir($share_path)) {
        $searchpath[] = $share_path;
      }

      // User commands, residing in ~/.drop
      if (!is_null(drop_server_home())) {
        $searchpath[] = drop_server_home() . '/.drop';
      }
      break;
    case DROP_BOOTSTRAP_BACKDROP_SITE:
      // If we are going to stop bootstrapping at the site, then
      // we will quickly add all commandfiles that we can find for
      // any module associated with the site, whether it is enabled
      // or not.  If we are, however, going to continue on to bootstrap
      // all the way to DROP_BOOTSTRAP_BACKDROP_FULL, then we will
      // instead wait for that phase, which will more carefully add
      // only those drop command files that are associated with
      // enabled modules.
      if ($phase_max < DROP_BOOTSTRAP_BACKDROP_FULL) {
        $searchpath[] = conf_path() . '/modules';
        // Too early for variable_get('install_profile', 'default'); Just use default.
        $searchpath[] = "profiles/default/modules";
        // Add all module paths, even disabled modules. Prefer speed over accuracy.
        $searchpath[] = 'sites/all/modules';
      }
      $searchpath[] = 'sites/all/themes';
      $searchpath[] = conf_path() . '/themes';
      break;
    case DROP_BOOTSTRAP_BACKDROP_CONFIGURATION:
      // See comment above  regarding this if() condition.
      if ($phase_max < DROP_BOOTSTRAP_BACKDROP_FULL) {
        // You must define your install_profile in settings.php. The DB is not sufficient.
        // Backdrop core does not yet do that. See http://backdrop.org/node/545452.
        if ($profile = variable_get('install_profile', NULL)) {
          $searchpath[] = "profiles/$profile/modules";
        }
      }
      break;
    case DROP_BOOTSTRAP_BACKDROP_FULL:
      // Add enabled module paths. Since we are bootstrapped,
      // we can use the Backdrop API.
      $ignored_modules = drop_get_option_list('ignored-modules', array());
      foreach (array_diff(module_list(), $ignored_modules) as $module) {
        $filename = backdrop_get_filename('module', $module);
        $searchpath[] = dirname($filename);
      }
      break;
  }

  _drop_add_commandfiles($searchpath, $phase);
}

function _drop_add_commandfiles($searchpath, $phase = NULL) {
  $cache =& drop_get_context('DROP_COMMAND_FILES', array());

  static $evaluated = array();
  static $deferred = array();

  if (sizeof($searchpath)) {
    // Build a list of all of the modules to attempt to load.
    // Start with any modules deferred from a previous phase.
    $list = $deferred;

    // Scan for drop command files; add to list for consideration if found.
    foreach (array_unique($searchpath) as $path) {
      if (is_dir($path)) {
        $nomask = array_merge(drop_filename_blacklist(), drop_get_option_list('ignored-modules'));
        $dmv = DROP_MAJOR_VERSION;
        $files = drop_scan_directory($path, "/\.drop($dmv|)\.inc$/", $nomask);
        foreach ($files as $filename => $info) {
          $module = basename($filename);
          $module = str_replace(array('.drop.inc', ".drop$dmv.inc"), '', $module);
          // Only try to bootstrap modules that we have never seen before, or that we
          // have tried to load but did not due to an unmet _drop_load() requirement.
          if (!array_key_exists($module, $evaluated) && file_exists($filename)) {
            $evaluated[$module] = TRUE;
            $list[$module] = $filename;
          }
        }
      }
    }

    // Check each file in the consideration list; if there is
    // a modulename_drop_load() function in modulename.drop.load.inc,
    // then call it to determine if this file should be loaded.
    foreach ($list as $module => $filename) {
      $load_command = TRUE;
      $load_test_inc = dirname($filename) . "/" . $module . ".drop.load.inc";
      if (file_exists($load_test_inc)) {
        include_once($load_test_inc);
        $load_test_func = $module . "_drop_load";
        if (function_exists($load_test_func)) {
          $load_command = $load_test_func($phase);
        }
      }
      if ($load_command) {
        require_once realpath($filename);
        unset($deferred[$module]);
      }
      else {
        unset($list[$module]);
        // Signal that we should try again on
        // the next bootstrap phase.  We set
        // the flag to the filename of the first
        // module we find so that only that one
        // will be retried.
        $deferred[$module] = $filename;
      }
    }

    if (sizeof($list)) {
      $cache = array_merge($cache, $list);
      ksort($cache);
    }
  }
}

/**
 * Substrings to ignore during commandfile searching.
 */
function drop_filename_blacklist() {
  return array_diff(array('.', '..', 'examples', 'tests', 'disabled', 'gitcache', 'drop4', 'drop5', 'drop6', 'drop7'), array('drop' . DROP_MAJOR_VERSION));
}

/**
 * Conditionally include files based on the command used.
 *
 * Steps through each of the currently loaded commandfiles and
 * loads an optional commandfile based on the key.
 *
 * When a command such as 'pm-enable' is called, this
 * function will find all 'enable.pm.inc' files that
 * are present in each of the commandfile directories.
 */
function drop_command_include($command) {
  $include_files = drop_command_get_includes($command);
  foreach($include_files as $filename => $commandfile) {
    drop_log(dt('Including !filename', array('!filename' => $filename)), 'bootstrap');
    include_once($filename);
  }
}

function drop_command_get_includes($command) {
  $include_files = array();
  $parts = explode('-', $command);
  $command = implode(".", array_reverse($parts));

  $commandfiles = drop_commandfile_list();
  $options = array();
  foreach ($commandfiles as $commandfile => $file) {
    $filename = sprintf("%s/%s.inc", dirname($file), $command);
    if (file_exists($filename)) {
      $include_files[$filename] = $commandfile;
    }
  }
  return $include_files;
}

/**
 * Conditionally include default options based on the command used.
 */
function drop_command_default_options($command = NULL) {
  if (!$command) {
    $command = drop_get_command();
  }
  if ($command) {
    // Look for command-specific options for this command
    // keyed both on the command's primary name, and on each
    // of its aliases.
    $options_were_set = _drop_command_set_default_options($command['command']);
    if (isset($command['aliases']) && count($command['aliases'])) {
      foreach ($command['aliases'] as $alias) {
        if (_drop_command_set_default_options($alias) === TRUE) {
          $options_were_set = TRUE;
        }
      }
    }
    // If we set or cleared any options, go back and re-bootstrap any global
    // options such as -y and -v.
    if ($options_were_set) {
      _drop_bootstrap_global_options();
    }
  }
}

function _drop_command_set_default_options($command) {
  $options_were_set = FALSE;
  $command_default_options = drop_get_context('command-specific');
  if (array_key_exists($command, $command_default_options)) {
    foreach ($command_default_options[$command] as $key => $value) {
      // We set command-specific options in their own context
      // that is higher precedence than the various config file
      // context, but lower than command-line options.
      if (!drop_get_option('no-' . $key, FALSE)) {
        drop_set_option($key, $value, 'specific');
        $options_were_set = TRUE;
      }
    }
  }
  return $options_were_set;
}

/**
 * Determine whether a command file implements a hook.
 *
 * @param $module
 *   The name of the module (without the .module extension).
 * @param $hook
 *   The name of the hook (e.g. "help" or "menu").
 * @return
 *   TRUE if the the hook is implemented.
 */
function drop_command_hook($commandfile, $hook) {
  return function_exists($commandfile .'_'. $hook);
}


/**
 * Finds all files that match a given mask in a given directory.
 * Directories and files beginning with a period are excluded; this
 * prevents hidden files and directories (such as SVN working directories
 * and GIT repositories) from being scanned.
 *
 * @param $dir
 *   The base directory for the scan, without trailing slash.
 * @param $mask
 *   The regular expression of the files to find.
 * @param $nomask
 *   An array of files/directories to ignore.
 * @param $callback
 *   The callback function to call for each match.
 * @param $recurse_max_depth
 *   When TRUE, the directory scan will recurse the entire tree
 *   starting at the provided directory.  When FALSE, only files
 *   in the provided directory are returned.  Integer values
 *   limit the depth of the traversal, with zero being treated
 *   identically to FALSE, and 1 limiting the traversal to the
 *   provided directory and its immediate children only, and so on.
 * @param $key
 *   The key to be used for the returned array of files. Possible
 *   values are "filename", for the path starting with $dir,
 *   "basename", for the basename of the file, and "name" for the name
 *   of the file without an extension.
 * @param $min_depth
 *   Minimum depth of directories to return files from.
 * @param $include_dot_files
 *   If TRUE, files that begin with a '.' will be returned if they
 *   match the provided mask.  If FALSE, files that begin with a '.'
 *   will not be returned, even if they match the provided mask.
 * @param $depth
 *   Current depth of recursion. This parameter is only used internally and should not be passed.
 *
 * @return
 *   An associative array (keyed on the provided key) of objects with
 *   "path", "basename", and "name" members corresponding to the
 *   matching files.
 */
function drop_scan_directory($dir, $mask, $nomask = array('.', '..', 'CVS'), $callback = 0, $recurse_max_depth = TRUE, $key = 'filename', $min_depth = 0, $include_dot_files = FALSE, $depth = 0) {
  $key = (in_array($key, array('filename', 'basename', 'name')) ? $key : 'filename');
  $files = array();

  if (is_string($dir) && is_dir($dir) && $handle = opendir($dir)) {
    while (FALSE !== ($file = readdir($handle))) {
      if (!in_array($file, $nomask) && (($include_dot_files && (!preg_match("/\.\+/",$file))) || ($file[0] != '.'))) {
        if (is_dir("$dir/$file") && (($recurse_max_depth === TRUE) || ($depth < $recurse_max_depth))) {
          // Give priority to files in this folder by merging them in after any subdirectory files.
          $files = array_merge(drop_scan_directory("$dir/$file", $mask, $nomask, $callback, $recurse_max_depth, $key, $min_depth, $include_dot_files, $depth + 1), $files);
        }
        elseif ($depth >= $min_depth && preg_match($mask, $file)) {
          // Always use this match over anything already set in $files with the same $$key.
          $filename = "$dir/$file";
          $basename = basename($file);
          $name = substr($basename, 0, strrpos($basename, '.'));
          $files[$$key] = new stdClass();
          $files[$$key]->filename = $filename;
          $files[$$key]->basename = $basename;
          $files[$$key]->name = $name;
          if ($callback) {
            drop_op($callback, $filename);
          }
        }
      }
    }

    closedir($handle);
  }

  return $files;
}

/**
 * Check that a command is valid for the current bootstrap phase.
 *
 * @param $command
 *   Command to check. Any errors will be added to the 'bootstrap_errors' element.
 *
 * @return
 *   TRUE if command is valid.
 */
function drop_enforce_requirement_bootstrap_phase(&$command) {
  $valid = array();
  $current_phase = drop_get_context('DROP_BOOTSTRAP_PHASE');
  if ($command['bootstrap'] <= $current_phase) {
    return TRUE;
  }
  // TODO: provide description text for each bootstrap level so we can give
  // the user something more helpful and specific here.
  $command['bootstrap_errors']['DROP_COMMAND_INSUFFICIENT_BOOTSTRAP'] = dt('Command !command needs a higher bootstrap level to run - you will need invoke drop from a more functional Backdrop environment to run this command.', array('!command' => $command['command']));
}

/**
 * Check that a command has its declared dependencies available or have no
 * dependencies.
 *
 * @param $command
 *   Command to check. Any errors  will be added to the 'bootstrap_errors' element.
 *
 * @return
 *   TRUE if command is valid.
 */
function drop_enforce_requirement_backdrop_dependencies(&$command) {
  // If the command bootstrap is DROP_BOOTSTRAP_MAX, then we will
  // allow the requirements to pass if we have not successfully
  // bootstrapped Backdrop.  The combination of DROP_BOOTSTRAP_MAX
  // and 'backdrop dependencies' indicates that the drop command
  // will use the dependent modules only if they are available.
  if ($command['bootstrap'] == DROP_BOOTSTRAP_MAX) {
    // If we have not bootstrapped, then let the dependencies pass;
    // if we have bootstrapped, then enforce them.
    if (drop_get_context('DROP_BOOTSTRAP_PHASE') < DROP_BOOTSTRAP_BACKDROP_FULL) {
      return TRUE;
    }
  }
  // If there are no backdrop dependencies, then do nothing
  if (!empty($command['backdrop dependencies'])) {
    foreach ($command['backdrop dependencies'] as $dependency) {
      if(!function_exists('module_exists') || !module_exists($dependency)) {
        $command['bootstrap_errors']['DROP_COMMAND_DEPENDENCY_ERROR'] = dt('Command !command needs the following modules installed/enabled to run: !dependencies.', array('!command' => $command['command'], '!dependencies' => implode(', ', $command['backdrop dependencies'])));
        return FALSE;
      }
    }
  }
  return TRUE;
}

/**
 * Check that a command has its declared drop dependencies available or have no
 * dependencies. Drop dependencies are helpful when a command is invoking
 * another command, or implementing its API.
 *
 * @param $command
 *   Command to check. Any errors  will be added to the 'bootstrap_errors' element.
 * @return
 *   TRUE if dependencies are met.
 */
function drop_enforce_requirement_drop_dependencies(&$command) {
  // If there are no drop dependencies, then do nothing.
  if (!empty($command['drop dependencies'])) {
    $commandfiles = drop_commandfile_list();
    foreach ($command['drop dependencies'] as $dependency) {
      if (!isset($commandfiles[$dependency])) {
        $dt_args = array(
          '!command' => $command['command'],
          '!dependency' => "$dependency.drop.inc",
        );
        $command['bootstrap_errors']['DROP_COMMAND_DEPENDENCY_ERROR'] = dt('Command !command needs the following drop command file to run: !dependency.', $dt_args);
        return FALSE;
      }
    }
  }
  return TRUE;
}

/**
 * Check that a command is valid for the current major version of core. Handles
 * explicit version numbers and 'plus' numbers like 1+ (compatible with 1, 2 ...).
 *
 * @param $command
 *   Command to check. Any errors  will be added to the 'bootstrap_errors' element.
 *
 * @return
 *   TRUE if command is valid.
 */
function drop_enforce_requirement_core(&$command) {
  $major = drop_backdrop_major_version();
  if (!$core = $command['core']) {
    return TRUE;
  }
  foreach ($core as $compat) {
    if ($compat == $major) {
      return TRUE;
    }
    elseif (substr($compat, -1) == '+' && $major >= substr($compat, 0, strlen($compat)-1)) {
      return TRUE;
    }
  }
  $versions = array_pop($core);
  if (!empty($core)) {
    $versions = implode(', ', $core) . dt(' or ') . $versions;
  }
  $command['bootstrap_errors']['DROP_COMMAND_CORE_VERSION_ERROR'] = dt('Command !command requires Backdrop core version !versions to run.', array('!command' => $command['command'], '!versions' => $versions));
}
