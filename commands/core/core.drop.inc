<?php

/**
 * @file
 *   Core drop commands.
 */

/**
 * Implementation of hook_drop_command().
 *
 * In this hook, you specify which commands your
 * drop module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @return
 *   An associative array describing your command(s).
 */
function core_drop_command() {
  $items = array();

  $items['help'] = array(
    'description' => 'Print this help message. See `drop help help` for more options.',
    'bootstrap' => DROP_BOOTSTRAP_DROP, // No bootstrap.
    'options' => array(
      'sort' => 'Sort commands in alphabetical order. drop waits for full bootstrap before printing any commands when this option is used.',
      'filter' => 'Restrict command list to those commands defined in the specified file.',
      'html' => 'Print help for all commands in HTML format.',
      'pipe' => 'A list of available commands, one per line.',
    ),
    'examples' => array(
      'drop' => 'List all commands.',
      'drop --filter=devel_generate' => 'Show only commands defined in devel_generate.drop.inc',
      'drop help pm-download' => 'Show help for one command.',
      'drop help dl' => 'Show help for one command using an alias.',
    ),
    'topics' => array('docs-readme'),
  );
  $items['version'] = array(
    'description' => 'Show drop version.',
    'bootstrap' => DROP_BOOTSTRAP_DROP, // No bootstrap.
    'options' => array(
      'pipe' => 'Print just the version number, and nothing else.',
      'self-update' => 'Check for pending updates to drop itself. Set to 0 to avoid check.',
    ),
  );
  $items['self-update'] = array(
    'description' => 'Update drop to the latest version, if available.',
    'bootstrap' => DROP_BOOTSTRAP_DROP, // No bootstrap.
    'options' => array(
      '--dev' => 'Allow updates to latest dev release.',
    ),
    'aliases' => array('selfupdate'),
  );
  $items['core-cron'] = array(
    'description' => 'Run all cron hooks in all active modules for specified site.',
    'aliases' => array('cron'),
  );
  $items['updatedb'] = array(
    'description' => 'Apply any database updates required (as with running update.php).',
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_SITE,
    'aliases' => array('updb'),
  );
  $items['core-status'] = array(
    'description' => 'Provides a birds-eye view of the current Backdrop installation, if any.',
    'bootstrap' => DROP_BOOTSTRAP_MAX,
    'aliases' => array('status', 'st'),
    'examples' => array(
      'drop status version' => 'Show all status lines that contain version information.',
      'drop status --pipe' => 'A list key=value items separated by line breaks.',
      'drop status drop-version --pipe' => 'Emit just the drop version with no label.',
    ),
    'arguments' => array(
      'item' => 'Optional.  The status item line(s) to display.',
    ),
    'options' => array(
      'show-passwords' => 'Show database password.',
    ),
    'topics' => array('docs-readme'),
  );
  $items['php-eval'] = array(
    'description' => 'Evaluate arbitrary php code after bootstrapping Backdrop (if available).',
    'examples' => array(
      'drop php-eval "variable_set(\'hello\', \'world\');"' => 'Sets the hello variable using Backdrop API.',
    ),
    'arguments' => array(
      'code' => 'PHP code',
    ),
    'bootstrap' => DROP_BOOTSTRAP_MAX,
    'aliases' => array('eval', 'ev'),
  );
  $items['php-script'] = array(
    'description' => "Run php script(s).",
    'examples' => array(
      'drop php-script scratch' => 'Run scratch.php script. See commands/core directory.',
      'drop php-script example --script-path=/path/to/scripts:/another/path' => 'Run script from specified paths',
      'drop php-script' => 'List all available scripts.',
      '' => '',
      "#!/usr/bin/env drop\n<?php\nvariable_set('key', drop_shift());" => "Execute php code with a full Backdrop bootstrap directly from a shell script.",
    ),
    'arguments' => array(
      'filename' => 'Optional. The file you wish to execute (without extension). If omitted, list files ending in .php in the current working directory and specified script-path. Some might not be real drop scripts. Beware.',
    ),
    'options' => array(
      'script-path' => "Additional paths to search for scripts, separated by : (Unix-based systems) or ; (Windows).",
    ),
    'bootstrap' => DROP_BOOTSTRAP_MAX,
    'aliases' => array('scr'),
    'deprecated-aliases' => array('script'),
    'topics' => array('docs-examplescript', 'docs-scripts'),
  );
  $items['search-status'] = array(
    'description' => 'Show how many items remain to be indexed out of the total.',
    'backdrop dependencies' => array('search'),
    'options' => array(
      'pipe' => 'Display in the format remaining/total for processing by scripts.',
    ),
  );
  $items['search-index'] = array(
    'description' => 'Index the remaining search items without wiping the index.',
    'backdrop dependencies' => array('search'),
  );
  $items['search-reindex'] = array(
    'description' => 'Force the search index to be rebuilt.',
    'backdrop dependencies' => array('search'),
    'options' => array(
      'immediate' => 'Rebuild the index immediately, instead of waiting for cron.',
    ),
  );
  $items['core-rsync'] = array(
    'description' => 'Rsync the Backdrop tree to/from another server using ssh.',
    'bootstrap' => DROP_BOOTSTRAP_DROP, // No bootstrap.
    'arguments' => array(
      'source' => 'May be rsync path or site alias. See rsync documentation and example.aliases.droprc.php.',
      'destination' => 'May be rsync path or site alias. See rsync documentation and example.aliases.droprc.php.',
    ),
    'options' => array(
      'mode' => 'The unary flags to pass to rsync; --mode=rultz implies rsync -rultz.  Default is -az.',
      'RSYNC-FLAG' => 'Most rsync flags passed to drop sync will be passed on to rsync.  See rsync documentation.',
      'exclude-conf' => 'Excludes settings.php from being rsynced.  Default.',
      'include-conf' => 'Allow settings.php to be rsynced. Default is to exclude settings.php.',
      'include-vcs' => 'Include special version control directories (e.g. .svn).  Default is to exclude vcs files.',
      'exclude-files' => 'Exclude the files directory.',
      'exclude-sites' => 'Exclude all directories in "sites/" except for "sites/all".',
      'exclude-other-sites' => 'Exclude all directories in "sites/" except for "sites/all" and the site directory for the site being synced.  Note: if the site directory is different between the source and destination, use --exclude-sites followed by "drop rsync @from:%site @to:%site"',
      'exclude-paths' => 'List of paths to exclude, seperated by : (Unix-based systems) or ; (Windows).',
      'include-paths' => 'List of paths to include, seperated by : (Unix-based systems) or ; (Windows).',
    ),
    'examples' => array(
      'drop rsync @dev @stage' => 'Rsync Backdrop root from dev to stage (one of which must be local).',
      'drop rsync ./ @stage:%files/img' => 'Rsync all files in the current directory to the \'img\' directory in the file storage folder on stage.',
    ),
    'aliases' => array('rsync'),
    'deprecated-aliases' => array('sync'),
    'topics' => array('docs-aliases'),
  );
  $items['site-install'] = array(
    'description' => 'Install Backdrop along with modules/themes/configuration using the specified install profile.',
    'arguments' => array(
      'profile' => 'the install profile you wish to run. defaults to \'default\' in D6, \'standard\' in D7',
      'key=value...' => 'any additional settings you wish to pass to the profile. Only support on D7. The key is in the form [form name].[parameter name].',
    ),
    'options' => array(
      'account-name' => 'uid1 name. defaults to admin',
      'account-pass' => 'uid1 pass. defaults to admin',
      'account-mail' => 'uid1 email. defaults to admin@example.com',
      'db-url' => 'A Backdrop 5/6 style database URL. Only required for initial install - not re-install.',
      'db-prefix' => 'An optional table prefix to use for initial install.  Can be a key-value array of tables/prefixes in a droprc file (not the command line).',
      'db-su' => 'Account to use when creating a new database. Optional.',
      'db-su-pw' => 'Password for the "db-su" account. Optional.',
      'locale' => 'A short language code. Sets the default site language. Language files must already be present. You may use download command to get them.',
      'clean-url'=> 'Defaults to 1',
      'site-name' => 'Defaults to Site-Install',
      'site-mail' => 'From: for system mailings. Defaults to admin@example.com',
      'sites-subdir' => "Name of directory under 'sites' which should be created. Only needed when the subdirectory does not already exist. Defaults to 'default'",
    ),
    'examples' => array(
      'drop site-install expert --locale=uk' => '(Re)install using the expert install profile. Set default language to Ukranian.',
      'drop site-install --db-url=mysql://root:pass@localhost:port/dbname' => 'Install using the specified DB params.',
      'drop site-install --db-url=sqlite:/full/path/to/database.sqlite' => 'Install using SQLite (D7 only).',
      'drop site-install --account-name=joe --account-pass=mom' => 'Re-install with specified uid1 credentials.',
      'drop site-install standard install_configure_form.site_default_country=FR my_profile_form.my_settings.key=value' => 'Pass additional arguments to the profile (D7 only).',
    ),
    'backdrop' => array('1+'),
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_ROOT,
    'aliases' => array('si'),
    'deprecated-aliases' => array('installsite', 'is'),
  );
  $items['backdrop-directory'] = array(
    'description' => 'Return path to a given module/theme directory.',
    'arguments' => array(
      'target' => 'A module/theme name, or special names like root, files, private, or an alias : path alias string such as @alias:%files. Defaults to root.',
    ),
    'options' => array(
      'component' => "The portion of the evaluated path to return.  Defaults to 'path'; 'name' returns the site alias of the target.",
      'local' => "Reject any target that specifies a remote site.",
    ),
    'examples' => array(
      'cd `drop bd devel`' => 'Navigate into the devel module directory',
      'cd `drop bd` ' => 'Navigate to the root of your Backdrop site',
      'cd `drop bd files`' => 'Navigate to the files directory.',
      'drop bd @alias:%files' => 'Print the path to the files directory on the site @alias.',
      'edit `drop dd devel`/devel.module' => "Open devel module in your editor (customize 'edit' for your editor)",
    ),
    'aliases' => array('bd'),
    'bootstrap' => DROP_BOOTSTRAP_DROP,
  );

  $items['core-cli'] = array(
    'description' => 'Enter a new shell optimized for drop use.',
    'options' => array(
      'override' => 'List of drop commands or aliases that should override built-in shell functions and commands; otherwise, built-ins override drop commands. Defaults to dd,help,sa.',
      'contextual' => 'Additional drop overrides that function ONLY when the prompt is "@alias>".  Defaults to cc,cron,rsync,status,sync,updatedb.',
      'ignore' => 'Drop commands or aliases that should not be usable from core-cli. Takes precedence over override and contextual options. Defaults to core-cli,cli.',
      'pipe' => 'Print the generated .bashrc file and exit.',
    ),
    'examples' => array(
      'help' => 'Print available drop commands',
      'cd @alias' => 'Navigate to the root of the site indicated by @alias; subsequent commands will target that site.',
      'cd %files' => 'Navigate to the files directory.',
      'cd ~' => 'Navigate back to your $HOME directory.',
      'lsd files' => 'List all files in the Backdrop files directory.',
      'on @alias core-status' => 'Run the command "core-status" on the site indicated by @alias',
      '@alias core-status' => 'An even shorter form that also runs "core-status" on the site @alias',
      'use @alias' => 'Run subsequent commands on the site indicated by @alias',
      'use -' => 'Switch back to the last alias "used".',
      'use ~' => 'Use the default alias.',
      'use' => 'Revert to an ordinary prompt; do not use an alias.',
      'drop core-cli --pipe > ~/.bash_aliases' => 'Convert your default shell into drop core-cli.  Make sure that your .bashrc file includes .bash_aliases (e.g. "source ~/.bash_aliases" or ". ~/.bash_aliases").',
    ),
    'aliases' => array('cli'),
    'bootstrap' => DROP_BOOTSTRAP_MAX,
  );

  $items['batch-process'] = array(
    'description' => 'Process operations in the specified batch set',
    'hidden' => TRUE,
      'arguments' => array(
        'batch-id' => 'The batch id that will be processed',
      ),
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_LOGIN,
  );

  $items['updatedb-batch-process'] = array(
    'description' => 'Perform update functions',
    'hidden' => TRUE,
    'arguments' => array(
      'batch-id' => 'The batch id that will be processed',
    ),
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_SITE,
  );
  $items['core-global-options'] = array(
    'description' => 'All global options',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => DROP_BOOTSTRAP_DROP,
  );
  return $items;
}

function core_drop_engine_backdrop() {
  $engines = array();
  $engines['batch'] = array();
  $engines['update'] = array();
  $engines['environment'] = array();
  $engines['site_install'] = array();
  return $engines;
}

/**
 * Command handler. Execute update.php code from drop.
 */
function drop_core_updatedb() {
  if (drop_get_context('DROP_SIMULATE')) {
    drop_log(dt('updatedb command does not support --simulate option.'), 'ok');
    return TRUE;
  }

  drop_include_engine('backdrop', 'update', drop_backdrop_major_version());
  if (update_main() === FALSE) {
    return FALSE;
  }

  if (drop_backdrop_major_version() <= 6) {
    // Clear all caches. We just performed major surgery.
    drop_backdrop_cache_clear_all();
  }
  else {
    // Should be unnecessary on D7.
    // On D7 site-upgrade, this cache_clear was leading to:
    // Call to undefined function field_read_fields() in field_sql_storage.install line 17
  }

  drop_log(dt('Finished performing updates.'), 'ok');
}

/**
 * Implementation of hook_drop_help().
 *
 * This function is called whenever a drop user calls
 * 'drop help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drop:')
 *
 * @return
 *   A string with the help text for your command.
 */
function core_drop_help($section) {
  switch ($section) {
    case 'meta:core:title':
      return dt("Core drop commands");
    case 'drop:help':
      return dt("Drop provides an extensive help system that describes both drop commands and topics of general interest.  Use `drop help --filter` to present a list of command categories to view, and `drop topic` for a list of topics that go more in-depth on how to use and extend drop.");
    case 'drop:php-script':
      return dt("Runs the given php script(s) after a full Backdrop bootstrap. A useful alternative to eval command when your php is lengthy or you can't be bothered to figure out bash quoting. If you plan to share a script with others, consider making a full drop command instead, since that's more self-documenting.  Drop provides commandline options to the script via drop_get_option('option-name'), and commandline arguments can be accessed either via drop_get_arguments(), which returns all arguments in an array, or drop_shift(), which removes the next argument from the list and returns it.");
    case 'drop:rsync':
      return dt("Sync the entire backdrop directory or a subdirectory to a <destination> using ssh. Excludes reserved files and directories for supported VCSs. Useful for pushing copies of your tree to a staging server, or retrieving a files directory from a remote site. Relative paths start from the Backdrop root directory if a site alias is used; otherwise they start from the current working directory.");
    case 'drop:backdrop-directory':
      return dt("Return the filesystem path for modules/themes and other key folders. Used by `cli` command for handy commands `cdd` and `lsd`. If you want to use this command directly, you usually want to prefix the command with cd and enclose the command invocation in backticks. See Examples below.");
    case 'drop:core-cli':
      return dt("Enter a new shell optimized for drop use. All .bashrc customizations are still available.");
    case 'error:DROP_BACKDROP_DB_ERROR':
      $message = dt("Drop was not able to start (bootstrap) the Backdrop database.\n");
      $message .= dt("Hint: This error often occurs when Drop is trying to bootstrap a site that has not been installed or does not have a configured database.\n");
      $message .= dt("\nDrop was attempting to connect to : \n!credentials\n", array('!credentials' => _core_site_credentials()));
      $message .= dt("You can select another site with a working database setup by specifying the URI to use with the --uri parameter on the command line or \$options['uri'] in your droprc.php file.\n");
      return $message;
    case 'error:DROP_BACKDROP_BOOTSTRAP_ERROR':
      $message = dt("Drop was not able to start (bootstrap) Backdrop.\n");
      $message .= dt("Hint: This error can only occur once the database connection has already been successfully initiated, therefore this error generally points to a site configuration issue, and not a problem connecting to the database.\n");
      $message .= dt("\nDrop was attempting to connect to : \n!credentials\n", array('!credentials' => _core_site_credentials()));
      $message .= dt("You can select another site with a working database setup by specifying the URI to use with the --uri parameter on the command line or \$options['uri'] in your droprc.php file.\n");
      return $message;
      break;
  }
}

// TODO: consolidate with SQL commands?
function _core_site_credentials() {
  $status_table = _core_site_status_table();
  return _core_site_credential_table($status_table);
}

function _core_site_credential_table($status_table) {
  $credentials = '';
  foreach ($status_table as $key => $value) {
    $credentials .= sprintf("  %-18s: %s\n", $key, $value);
  }
  return $credentials;
}

function _core_site_credential_list($status_table) {
  $credentials = '';
  foreach ($status_table as $key => $value) {
    if (isset($value)) {
      $credentials .= sprintf("%s=%s\n", strtolower(str_replace(' ', '_', $key)), $value);
    }
  }
  return $credentials;
}

function _core_path_aliases($project = '') {
  $paths = array();
  $phase = drop_get_context('DROP_BOOTSTRAP_PHASE');
  if ($backdrop_root = drop_get_context('DROP_BACKDROP_ROOT')) {
    $paths['%root'] = $backdrop_root;
    if ($site_root = drop_get_context('DROP_BACKDROP_SITE_ROOT')) {
      $paths['%site'] = $site_root;
      if (is_dir($modules_path = conf_path() . '/modules')) {
        $paths['%modules'] = $modules_path;
      }
      else {
        $paths['%modules'] = 'sites/all/modules';
      }
      if (is_dir($themes_path = conf_path() . '/themes')) {
        $paths['%themes'] = $themes_path;
      }
      else {
        $paths['%themes'] = 'sites/all/themes';
      }
      $paths['%files'] = config_get('system.core', 'file_public_path');
      $paths['%temp'] = config_get('system.core', 'file_temporary_path');
      $paths['%private'] = config_get('system.core', 'file_private_path');
      $paths['%install_profile'] = config_get('system.core', 'install_profile');

      // If the 'project' parameter was specified, then search
      // for a project (or a few) and add its path to the path list
      if (!empty($project)) {
        foreach(explode(',', $project) as $target) {
          $path = drop_core_find_project_path($target);
          if(isset($path)) {
            $paths['%' . $target] = $path;
          }
        }
      }
    }
  }

  // Add in all of the global paths from $options['path-aliases']
  $paths = array_merge($paths, drop_get_option('path-aliases', array()));

  return $paths;
}

function _core_site_status_table($project = '') {
  $phase = drop_get_context('DROP_BOOTSTRAP_PHASE');
  if ($backdrop_root = drop_get_context('DROP_BACKDROP_ROOT')) {
    $status_table['Backdrop version'] = BACKDROP_VERSION;
    if ($site_root = drop_get_context('DROP_BACKDROP_SITE_ROOT')) {
      $status_table['Site URI'] = drop_get_context('DROP_URI');
      if ($creds = drop_get_context('DROP_DB_CREDENTIALS')) {
        $status_table['Database driver'] = $creds['driver'];
        $status_table['Database hostname'] = $creds['host'];
        $status_table['Database username'] = $creds['user'];
        $status_table['Database name'] = $creds['name'];
        if (drop_get_option('show-passwords', FALSE)) {
          $status_table['Database password'] = $creds['pass'];
        }
        if ($phase > DROP_BOOTSTRAP_BACKDROP_DATABASE) {
          $status_table['Database'] = dt('Connected');
          if ($phase > DROP_BOOTSTRAP_BACKDROP_FULL) {
            $status_table['Backdrop bootstrap'] = dt('Successful');
            if ($phase == DROP_BOOTSTRAP_BACKDROP_LOGIN) {
              global $user;
              $username =  ($user->uid) ? $user->name : dt('Anonymous');
              $status_table['Backdrop user'] = $username;
            }
          }
        }
      }
    }
    $status_table['Default theme'] = drop_theme_get_default();
    $status_table['Administration theme'] = drop_theme_get_admin();
  }
  if ($php_ini_files = _drop_core_config_php_ini_files()) {
    $status_table['PHP configuration'] = implode(' ', $php_ini_files);
  }
  drop_sitealias_load_all();
  $status_table['Drop script'] = DROP_COMMAND;
  $status_table['Drop version'] = DROP_VERSION;
  $drop_configs = implode(' ', drop_get_context_options('context-path', TRUE));
  if (!empty($drop_configs)) {
    $status_table['Drop configuration'] = $drop_configs;
  }
  $status_table['Drop alias files'] = implode(' ', drop_get_context('drop-alias-files'));

  // None of the Status keys are in dt(); this helps with machine-parsing of status?
  $path_names['root'] = 'Backdrop root';
  $path_names['site'] = 'Site path';
  $path_names['install_profile'] = 'Install profile';
  $path_names['modules'] = 'Modules path';
  $path_names['themes'] = 'Themes path';
  $path_names['files'] = 'Files directory path';
  $path_names['temp'] = 'Temporary files directory path';
  $path_names['private'] = 'Private files directory path';

  $paths = _core_path_aliases($project);

  if (!empty($paths)) {
    foreach ($paths as $target => $one_path) {
      $name = $target;
      if (substr($name,0,1) == '%') {
        $name = substr($name,1);
      }
      if (array_key_exists($name, $path_names)) {
        $name = $path_names[$name];
      }
      $status_table[$name] = $one_path;
    }
  }

  // Store the paths into the '%paths' index; this will be
  // used by other code, but will not be included in the output
  // of the drop status command.
  $status_table['%paths'] = $paths;

  return $status_table;
}

function _drop_core_config_php_ini_files() {
  $ini_files = array();
  // Function available on PHP >= 5.2.4, but we use it if available to help
  // users figure out their php.ini issues.
  if (function_exists('php_ini_loaded_file') && !empty(php_ini_loaded_file())) {
    $ini_files[] = php_ini_loaded_file();
  } else {
    // Backdroped
    // Sometimes (eg. Docksal) php_ini_loaded_file() returns empty:
    if (file_exists($_SERVER['PHP_INI_DIR'] . "/php.ini")) {
      $ini_files[] = $_SERVER['PHP_INI_DIR'] . "/php.ini";
    }
  }

  foreach (array(DROP_BASE_PATH, '/etc/drop', drop_server_home() . '/.drop') as $ini_dir) {
    if (file_exists($ini_dir . "/php.ini")) {
      $ini_files[] = realpath($ini_dir . "/php.ini");
    }
    if (file_exists($ini_dir . "/drop.ini")) {
      $ini_files[] = realpath($ini_dir . "/drop.ini");
    }
  }

  return $ini_files;
}

/**
 * Command callback. Runs cron hooks.
 *
 * This is where the action takes place.
 *
 * In this function, all of Backdrops API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * To print something to the terminal window, use drop_print().
 *
 */
function drop_core_cron() {
  if (backdrop_cron_run()) {
    drop_log(dt('Cron run successfully.'), 'success');
  }
  else {
    return drop_set_error('DROP_CRON_FAILED', dt('Cron run failed.'));
  }
}

/**
 * Command callback. Provides a birds-eye view of the current Backdrop
 * installation.
 */
function drop_core_status() {
  $status_table = _core_site_status_table(drop_get_option('project',''));
  // If args are specified, filter out any entry that is not named
  // (in other words, only show lines named by one of the arg values)
  $args = func_get_args();
  if (!empty($args)) {
    foreach ($status_table as $key => $value) {
      if (!_drop_core_is_named_in_array($key, $args)) {
        unset($status_table[$key]);
      }
    }
  }
  drop_backend_set_result($status_table);
  unset($status_table['%paths']);
  // Print either an ini-format list or a formatted ASCII table
  if (drop_get_option('pipe')) {
    if (count($status_table) == 1) {
      $first_value = array_shift($status_table);
      drop_print_pipe($first_value);
    }
    else {
      drop_print_pipe(_core_site_credential_list($status_table));
    }
  }
  else {
    drop_print_table(drop_key_value_to_array_table($status_table));
  }
  return;
}

// Command callback. Show all global options. Exposed via topic command.
function drop_core_global_options() {
  drop_print(dt('These options are applicable to most drop commands.'));
  drop_print();
  $fake = drop_global_options_command(FALSE);
  $global_option_rows = drop_format_help_section($fake, 'options');
  drop_print_table($global_option_rows);
}

function _drop_core_is_named_in_array($key, $the_array) {
  $is_named = FALSE;

  $simplified_key = str_replace(array(' ', '_', '-'), array('', '', ''), $key);

  foreach ($the_array as $name) {
    if (stristr($simplified_key, str_replace(array(' ', '_', '-'), array('', '', ''), $name))) {
      $is_named = TRUE;
    }
  }

  return $is_named;
}

/**
 * Command callback. Runs "naked" php scripts
 * and drop "shebang" scripts ("#!/usr/bin/env drop").
 */
function drop_core_php_script() {
  $found = FALSE;
  $script = NULL;
  if ($args = func_get_args()) {
    $script = $args[0];
  }

  if ($script == '-') {
    eval(stream_get_contents(STDIN));
  }
  elseif (file_exists($script)) {
    $found = $script;
  }
  else {
    // Array of paths to search for scripts
    $searchpath['DIR'] = dirname(__FILE__);
    $searchpath['cwd'] = drop_cwd();

    // Additional script paths, specified by 'script-path' option
    if ($script_path = drop_get_option('script-path', FALSE)) {
      foreach (explode(PATH_SEPARATOR, $script_path) as $path) {
        $searchpath[] = $path;
      }
    }
    drop_log(dt('Searching for scripts in ') . implode(',', $searchpath), 'debug');

    if (!isset($script)) {
      // List all available scripts.
      $all = array();
      foreach($searchpath as $key => $path) {
        $recurse = !(($key == 'cwd') || ($path == '/'));
        $all = array_merge( $all , array_keys(drop_scan_directory($path, '/\.php$/', array('.', '..', 'CVS'), NULL, $recurse)) );
      }
      drop_print(implode("\n", $all));
    }
    else {
      // Execute the specified script.
      foreach($searchpath as $path) {
        $script_filename = $path . '/' . $script;
        if (file_exists($script_filename . '.php')) {
          $script_filename .= '.php';
        }
        if (file_exists($script_filename)) {
          $found = $script_filename;
          break;
        }
        $all[] = $script_filename;
      }
      if (!$found) {
        return drop_set_error('DROP_TARGET_NOT_FOUND', dt('Unable to find any of the following: @files', array('@files' => implode(', ', $all))));
      }
    }
  }

  if ($found) {
    // Set the DROP_SHIFT_SKIP to two; this will cause
    // drop_shift to skip the next two arguments the next
    // time it is called.  This allows scripts to get all
    // arguments, including the 'php-script' and script
    // pathname, via drop_get_arguments(), or it can process
    // just the arguments that are relevant using drop_shift().
    drop_set_context('DROP_SHIFT_SKIP', 2);
    if (_drop_core_eval_shebang_script($found) === FALSE) {
      include($found);
    }
  }
}

function drop_core_php_eval($command) {
  eval($command . ';');
}

/*
 * Evaluate a script that begins with #!drop php-script
 */
function _drop_core_eval_shebang_script($script_filename) {
  $found = FALSE;
  $fp = fopen($script_filename, "r");
  if ($fp !== FALSE) {
    $line = fgets($fp);
    if (_drop_is_drop_shebang_line($line)) {
      $first_script_line = '';
      while ($line = fgets($fp)) {
        $line = trim($line);
        if ($line == '<?php') {
          $found = TRUE;
          break;
        }
        elseif (!empty($line)) {
          $first_script_line = $line . "\n";
          break;
        }
      }
      $script = stream_get_contents($fp);
      // Pop off the first two arguments, the
      // command (php-script) and the path to
      // the script to execute, as a service
      // to the script.
      eval($first_script_line . $script);
      $found = TRUE;
    }
    fclose($fp);
  }
  return $found;
}


/**
 * Process sets from the specified batch.
 *
 * This is the default batch processor that will be used if the $command parameter
 * to drop_backend_batch_process() has not been specified.
 */
function drop_core_batch_process($id) {
  drop_batch_command($id);
}

/**
 * Process outstanding updates during updatedb.
 *
 * This is a batch processing command that makes use of the drop_backend_invoke
 * api.
 *
 * This command includes the version specific update engine, which correctly
 * initialises the environment to be able to successfully handle minor and major
 * upgrades.
 */
function drop_core_updatedb_batch_process($id) {
  drop_include_engine('backdrop', 'update', drop_backdrop_major_version());
  _update_batch_command($id);
}

/**
 * Given a target (e.g. @site:%modules), return the evaluated
 * directory path
 *
 * @param $target
 *   The target to evaluate.  Can be @site or /path or @site:path
 *   or @site:%pathalais, etc. (just like rsync parameters)
 * @param $component
 *   The portion of the evaluated path to return.  Possible values:
 *   'path' - the full path to the target (default)
 *   'name' - the name of the site from the path (e.g. @site1)
 *   'user-path' - the part after the ':' (e.g. %modules)
 *   'root' & 'uri' - the Backdrop root and URI of the site from the path
 *   'path-component' - The ':' and the path
 */
function _drop_core_directory($target = 'root', $component = 'path', $local_only = FALSE) {
  // Normalize to a sitealias in the target.
  $normalized_target = $target;
  if (strpos($target, ':') === FALSE) {
    if (substr($target,0,1) == '@') {
      $normalized_target = $target; // . ':%site';
    }
    else {
      // @self makes no sense before 'site' level.
      if(!drop_bootstrap(DROP_BOOTSTRAP_BACKDROP_SITE)) {
        return FALSE;
      }
      $normalized_target = '@self:';
      if (substr($target,0,1) != '%') {
        $normalized_target .= '%';
      }
      $normalized_target .= $target;
    }
  }

  $additional_options = array();
  $values = drop_sitealias_evaluate_path($normalized_target, $additional_options, $local_only);
  if (isset($values[$component])) {
    // Hurray, we found the destination
    return $values[$component];
  }
  return NULL;
}

function drop_core_backdrop_directory($target = 'root') {
  $path = _drop_core_directory($target, drop_get_option('component', 'path'), drop_get_option('local', FALSE));

  // If _drop_core_directory is working right, it will turn
  // %blah into the path to the item referred to by the key 'blah'.
  // If there is no such key, then no replacement is done.  In the
  // case of the dd command, we will consider it an error if
  // any keys are -not- replaced in _drop_core_directory.
  if ((isset($path)) && (strpos($path, '%') === FALSE)) {
    drop_print($path);
  }
  else {
    return drop_set_error('DROP_TARGET_NOT_FOUND', dt("Target '!target' not found.", array('!target' => $target)));
  }

  return TRUE;
}

/**
 * Called for `drop version` or `drop --version`
 */
function drop_core_version() {
  drop_print(dt("drop version !version", array('!version' => DROP_VERSION)));
  drop_print_pipe(DROP_VERSION);
  // Next check to see if there is a newer drop.
  if (!drop_get_context('DROP_PIPE') && !drop_get_option('self-update', TRUE)) {
    drop_check_self_update();
  }
}

function drop_core_self_update() {
  drop_set_option('select', TRUE);
  return drop_invoke('pm-download', 'drop');
}

function drop_core_find_project_path($target) {
  $theme_suffix = drop_backdrop_major_version() >= 6 ? '.info' : '/style.css';
  $masks = array(
    conf_path() . '/modules' => "/^$target\.module$/",
    'profiles/default/modules' => "/^$target\.module$/", // Too early for variable_get('install_profile', 'default'); Just use default.
    'sites/all/modules' => "/^$target\.module$/", // Add all module paths, even disabled modules.
    conf_path() . '/themes' => "/^$target" . "$theme_suffix/",
    'sites/all/themes' => "/^$target" . "$theme_suffix/",
  );

  $files = array();
  foreach ($masks as $key => $mask) {
    $skip_list = array('.', '..', 'CVS');
    $skip_list = array_merge($skip_list, drop_version_control_reserved_files());
    if ($files = drop_scan_directory("$key", $mask, $skip_list, 0, TRUE, 'name')) {
      // Just use the first match.
      $file = reset($files);
      return drop_get_context('DROP_BACKDROP_ROOT') . '/' . dirname($file->filename);
    }
  }

  return NULL;
}

function drop_core_cli() {
  // Do not allow cli to start recursively, or from backend invoke.
  if (drop_get_option('in-cli', FALSE)) {
    return drop_set_error('DROP_CLI_NOT_REENTRANT', dt('Already in drop core-cli; press control-d to exit.'));
  }
  if (drop_get_context('DROP_BACKEND') || drop_get_context('DROP_AFFIRMATIVE') || drop_get_context('DROP_NEGATIVE')) {
    return drop_set_error('DROP_CLI_INTERACTIVE_ERROR', dt('Cannot run drop core-cli from non-interactive mode; aborting.'));
  }

  // We set interactive mode if we are about to run a bash subshell.
  // The purpose of doing this is that some options are set differently
  // in --pipe mode.  To see everything with --pipe, use --pipe --verbose.
  $interactive_mode = !drop_get_context('DROP_PIPE') || drop_get_context('DROP_VERBOSE');

  // Make sure that we call drop the same way that we were called.
  // In --pipe mode, just use 'drop'.
  $drop_command = $interactive_mode ? DROP_COMMAND.' --in-cli' : 'drop';

  $bashrc_data = implode("\n\n", drop_command_invoke_all('cli_bashrc', $drop_command, $interactive_mode));

  // Print our bashrc file and exit in --pipe mode
  if (drop_get_context('DROP_PIPE')) {
    drop_print_pipe($bashrc_data);
    return TRUE;
  }

  // If there were any errors, then exit.
  if (drop_get_error()) {
    return FALSE;
  }

  drop_print("Entering the drop cli.  Use CONTROL-D to exit.");
  drop_print("Type 'help' for help.");

  // Save out bashrc in a temporary file and launch bash.
  // control-d to exit.  The temp file will be deleted after
  // we exit.
  $bashrc = drop_save_data_to_temp_file($bashrc_data);
  return drop_op_system('bash --rcfile ' . $bashrc . ' > `tty`');
}

// Implement our own hook_cli_bashrc()
function core_cli_bashrc($drop_command, $interactive_mode) {
  $bashrc_data = '';

  $initial_site = '';
  $site_list = drop_sitealias_resolve_sitespecs(array('@self'));
  if (!empty($site_list)) {
    $site_list = array_keys($site_list);
    if ($site_list[0] != "@self") {
      $initial_site = $site_list[0];
    }
  }
  if (empty($initial_site)) {
    drop_sitealias_load_all(TRUE);
    $initial_site = _drop_sitealias_find_local_alias_name(drop_get_context('DROP_BACKDROP_ROOT'), str_replace('http://','',drop_get_context('DROP_URI')));
  }

  $searchpath[] = drop_get_context('SHARE_PREFIX', '/usr') . '/share/drop/commands';
  $searchpath[] = drop_get_context('ETC_PREFIX', '') . '/etc/drop';
  $searchpath[] = realpath(dirname(__FILE__) . '/../../');
  $bashrc_searchpath = implode(' ', $searchpath);

  // Set the prompt to 'drop> ' in interactive mode.
  if ($interactive_mode) {
    $bashrc_data .= "# Set our prompt to 'drop> '\nPS1='drop> '\n\n";
  }

  // Set up our default bashrc file for the drop cli
  $bashrc_data .= <<<EOD
# Drop .bashrc file.  Copy selected portions of this
# into your own bash resource file to add features of the
# drop core-cli command to your default shell.

# Where we will search for .bashrc files
BASHRC_PATH="~/.drop $bashrc_searchpath"

# Set the DROP_CLI variable; it may also be of use
# in the user's .bashrc, or elsewhere.  If we have not
# already set DROP_CLI, add ~ to our BASHRC_PATH.
# This allows us to source ~/.bashrc when we are running
# `drop core-cli` without causing problems with reentrancy
# if someone sources this file from ~/.bashrc.
[ -z \$DROP_CLI ] && BASHRC_PATH="~ \$BASHRC_PATH"
DROP_CLI=true

# Cache our initial site
INITIAL_SITE=$initial_site

# Look for drop-specific bashrc files and
# source any that are found.  Will also source
# \$HOME/.bashrc if run via `drop core-cli`
for d in \$BASHRC_PATH ; do
  [ -r \$d/.bashrc ] && {
    . \$d/.bashrc
  }
done

# Display our initial site in the bash prompt.
INITIAL_PS1="\$PS1"

# When using the drop shell, all commands will
# by default target the site specified by the
# shell variable \$BACKDROP_SITE.  To run a command
# on some other site, use either:
#   drop @site command
# or
#   on @site command
alias on="$drop_command"

# To specify a new BACKDROP_SITE without changing
# your current directory:
#   use @site
# To use the previous site (like `cd -`):
#   use -
# To use the initial site set when the shell was
# first entered:
#   use ~
function use() {
  if [ "x\$1" == "x-" ]
  then
    use \$PREVIOUS_SITE
  # `use ~1` will go back to our initial site.  Note, however,
  # that the shell will convert ~ to \$HOME before we get here,
  # so we will simply test for either ~ or \$HOME for this feature.
  elif [ "x\$1" == "x~" ] || [ "x\$1" == "x\$HOME" ]
  then
    use \$INITIAL_SITE
  elif [ "x\$1" != "x" ] && [ "x\$1" != "x@self" ]
  then
    SITE=`drop site-alias \$1 --short 2>/dev/null`
    if [ \$? != 0 ] || [ -z "\$SITE" ]
    then
      echo "Alias \$1 not found."
    else
      REMOTE_BACKDROP=`drop site-alias \$1 --component='remote-host' 2>/dev/null`
      PREVIOUS_SITE=\$BACKDROP_SITE
      BACKDROP_SITE=$1
      if [ -n "\$REMOTE_BACKDROP" ]
      then
        PS1="\$REMOTE_BACKDROP:\${1:-drop}> "
      else
        PS1="\${1:-drop}> "
      fi
    fi
  else
    PREVIOUS_SITE=\$BACKDROP_SITE
    BACKDROP_SITE=
    PS1="\$INITIAL_PS1"
  fi
}

# We override the cd command to allow convenient
# shorthand notations, such as:
#   cd @site1
#   cd %modules
#   cd %devel
#   cd @site2:%files
# When changing the cwd to a new site, the BACKDROP_SITE
# variable is also changed, so the new site becomes
# the default drop target.
function cd() {
  d=\$1;
  # If we do not have a \$BACKDROP_SITE from `use @alias`,
  # then 'cd @alias' will still cd to @alias, but all other
  # patterns will be passed to builtin cd.  The same behavior
  # will be used if \$BACKDROP_SITE points to a remote alias.
  if [ -z \$BACKDROP_SITE ] || [ -n "\$REMOTE_BACKDROP" ]
  then
    if [ -n "\$d" ] && [ \${d:0:1} == "@" ]
    then
      cdd \$@;
    else
      builtin cd "\$d";
    fi
  else
    # First special-case the no-parameter cd
    if [ -z "\$d" ]
    then
      cdd %root;
    # Next test and see if this should be an 'ordinary' cd
    elif [ "\$1" == "-" ] || [ -d "\$d" ]
    then
      builtin cd "\$d";
    else
      cdd \$@
    fi
  fi
}

# Do a special drop core-cli cd, handling
# shorthand notation directory names -- anything
# understood by backdrop-directory
function cdd() {
  DEST=`backdrop-directory \$1 --local 2>/dev/null`
  if [ \$? == 0 ]
  then
    SITE=`backdrop-directory \$1 --component=name 2>/dev/null`
    if [ -n "\$SITE" ]
    then
      use \$SITE;
    fi
    echo "cd \$DEST";
    builtin cd "\$DEST";
  else
    builtin cd "\$1"
  fi
}

# The command lsd will quickly do an 'ls' on the
# specified special drop directory, e.g.:
#   lsd %modules
function lsd() {
  # First special-case the no-parameter cd
  if [ "x\$1" == "x" ]
  then
    lsd %root
  else
    # Do a special drop core-cli cd, handling
    # shorthand notation directory names -- anything
    # understood by backdrop-directory
    DEST=`backdrop-directory \$1 --local`
    if [ \$? == 0 ]
    then
      echo "ls \$DEST";
      ls \$DEST;
    fi
  fi
}

EOD;

  // Add aliases for all drop commands
  $bashrc_data .= "# Add aliases for all drop commands\n";
  $commands = drop_get_commands();
  $cli_overrides = _convert_csv_to_array(drop_get_option('override', 'bd,help,sa'));
  $cli_contextual_commands = _convert_csv_to_array(drop_get_option('contextual', 'cc,cron,rsync,status,sync,updatedb'));
  $cli_ignore = _convert_csv_to_array(drop_get_option('ignore','core-cli,cli'));
  foreach ($commands as $key => $command) {
    // Filter out old commands that still have spaces;
    // do not mask any existing bash command (e.g. 'grep')
    // or builtin (e.g. 'eval') with drop aliases.
    if ((strpos($key, ' ') === FALSE) && (!in_array($key, $cli_ignore))) {
      $conflict = FALSE;
      $is_shell_builtin = (drop_shell_exec("bash -c \"builtin $key\"") == "1");
      if (!in_array($key, $cli_overrides)) {
        $status = drop_shell_exec("which $key 2>&1");
        if ($status !== FALSE) {
          $output_lines = drop_shell_exec_output();
          $conflict = $output_lines[0];
        }
        elseif ($is_shell_builtin) {
          $conflict = "builtin $key";
        }
      }
      elseif ($is_shell_builtin && ($key != "help")) {
        drop_set_error('DROP_OVERRIDE_BUILTIN', dt("Cannot override shell builtin function !builtin", array('!builtin' => $key)));
      }
      if (!$conflict) {
        $bashrc_data .= "function $key() {\n  $drop_command \$BACKDROP_SITE $key \"\$@\"\n}\n";
      }
      elseif (in_array($key, $cli_contextual_commands)) {
        if ($is_shell_builtin) {
          $warning = dt("Warning: overriding shell built-in command !builtin.  This could cause unpredictable results with shell scripts.", array('!builtin' => $key));
          drop_log($warning, 'warning');
          $bashrc_data .= "# $warning\n";
        }
	if ($key == 'rsync') {
          $bashrc_data .= "function $key() { \n  HAS_SITE_PARAMS=\n  for p in \"\$@\" ; do\n    if [ \"x\${p:0:1}\" == \"x@\" ] || [ \"x\${p:0:1}\" == \"x%\" ]\n    then\n      HAS_SITE_PARAMS=true\n    fi\n  done\n  if [ -z \$HAS_SITE_PARAMS ]\n  then \n      $conflict \"\$@\"\n  else\n      $drop_command \$BACKDROP_SITE $key \"\$@\"\n  fi\n}\n";
	}
	else {
          $bashrc_data .= "function $key() {\n  if [ -z \$BACKDROP_SITE ]\n  then \n    $conflict \"\$@\"\n  else\n    $drop_command \$BACKDROP_SITE $key \"\$@\"\n  fi\n}\n";
        }
      }
      else {
        $bashrc_data .= "# drop $key skipped; conflicts with $conflict\n";
      }
    }
  }

  // Make bash aliases for all drop site aliases.
  // @sitealias becomes an alias for "drop @sitealias".
  // This allows:
  //   $ @sitealias status
  drop_sitealias_load_all();
  foreach (drop_get_context('site-aliases') as $site_name => $site_record) {
    $bashrc_data .= "alias $site_name='drop $site_name'\n";
  }

  // Add some additional statements to the bashrc data that
  // should not go into --pipe mode.
  if ($interactive_mode) {
    // If there is an initial site, then cd to it.  This will set the prompt.
    if (!empty($initial_site)) {
      $bashrc_data .= "cd $initial_site\n";
    }
    // Before entering the new bash script, cd to the current site root,
    // if any.  This will make our default site for drop match the
    // currently bootstrapped site (at least until the user cd's away).
    // We do this -after- the drop_print_pipe($bashrc_data) because
    // we DO NOT want to change the working directory if this bashrc
    // is being used by a login shell.
    elseif ($site_root = drop_get_context('DROP_BACKDROP_SITE_ROOT')) {
      $site_root = drop_get_context('DROP_BACKDROP_ROOT') . '/' . $site_root;
      $bashrc_data .= "builtin cd $site_root\n";
    }
    if ($drop_command != 'drop') {
      $bashrc_data .= "alias drop=\"$drop_command\"\n";
    }
  }
  // Add some additional statements that should only appear in non-interactive mode.
  else {
    // If there is an initial site, then use.  This will set the prompt.
    if (!empty($initial_site)) {
      $bashrc_data .= "use $initial_site\n";
    }
  }

  return $bashrc_data;
}
