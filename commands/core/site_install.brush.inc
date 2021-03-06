<?php

/**
 * Command validate.
 */
function brush_core_site_install_validate() {
  if ($sites_subdir = brush_get_option('sites-subdir')) {
    $lower = strtolower($sites_subdir);
    if ($sites_subdir != $lower) {
      brush_log(bt('Only lowercase sites-subdir are valid. Switching to !lower.', array('!lower' => $lower)), 'warning');
      brush_set_option('sites-subdir', $lower);
    }
  }
}

/**
 * Perform setup tasks for installation.
 */
function brush_core_pre_site_install() {
  if (!$db_spec = _brush_sql_get_db_spec()) {
    brush_set_error(bt('Could not determine database connection parameters. Pass --db-url option.'));
    return;
  }
  if ($sites_subdir = brush_get_option('sites-subdir')) {
    // Needed so that we later bootstrap into the right site.
    brush_set_option('uri', 'http://'.$sites_subdir);
  }
  else {
    $sites_subdir = 'default';
  }

  $conf_path = "sites/$sites_subdir";
  $files = "$conf_path/files";
  $settingsfile = "$conf_path/settings.php";
  if (!file_exists($files)) {
    $msg[] = bt('create a @files directory', array('@files' => $files));
  }
  if (!file_exists($settingsfile)) {
    $msg[] = bt('create a @settingsfile file', array('@settingsfile' => $settingsfile));
  }
  $msg[] = bt("drop your '@db' database and then create a new one.", array('@db' => $db_spec['database']));

  if (!brush_confirm(bt('You are about to ') . implode(bt(' and '), $msg) . ' Do you want to continue?')) {
    return brush_user_abort();
  }

  // Can't install without sites directory and settings.php.
  if (!file_exists($conf_path)) {
    if (!brush_op('mkdir', $conf_path) && !brush_get_context('BRUSH_SIMULATE')) {
      brush_set_error(bt('Failed to create directory @conf_path', array('@conf_path' => $conf_path)));
      return;
    }
  }
  else {
    brush_log(bt('Sites directory @subdir already exists - proceeding.', array('@subdir' => $conf_path)));
  }
  if (!file_exists($settingsfile)) {
    if (!brush_op('copy', 'sites/default/default.settings.php', $settingsfile) && !brush_get_context('BRUSH_SIMULATE')) {
      brush_set_error(bt('Failed to copy sites/default/default.settings.php to  @settingsfile', array('@settingsfile' => $settingsfile)));
      return;
    }
  }

  // Add a files dir if needed
  if (!file_exists($files)) {
    if (!brush_op('mkdir', $files) && !brush_get_context('BRUSH_SIMULATE')) {
      brush_set_error(bt('Failed to create directory @name', array('@name' => $files)));
      return;
    }
  }

  // Now we can bootstrap up to the specified site.
  brush_bootstrap(BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION);

  // Brush and create DB if needed.
  $db_name = $db_spec['database'];
  $scheme = _brush_sql_get_scheme($db_spec);
  $simulate = brush_get_context('BRUSH_SIMULATE');

  if ($scheme === 'sqlite') {
    // With SQLite, we don't BRUSH DATABASEs. Each database is in a single file,
    // so we just remove the file. We also don't CREATE DATABASEs; it is created
    // when SQLite attempts to open a database file which doesn't exist.
    if (file_exists($db_spec['database']) && !$simulate) {
      if (!unlink($db_spec['database'])) {
        brush_set_error(bt('Could not brush database: @name', array('@name' => $db_name)));
      }
    }
  }
  else {
    brush_sql_empty_db($db_spec);
  }
  return TRUE;
}

/**
 * Command callback.
 */
function brush_core_site_install($profile = NULL) {
  $args = func_get_args();
  $form_options = array();

  if ($args) {
    // The first argument is the profile.
    $profile = array_shift($args);
    // Subsequent arguments are additional form values.
    foreach ($args as $arg) {
      list($key, $value) = explode('=', $arg);
      $form_options[$key] = $value;
    }
  }

  brush_include_engine('backdrop', 'site_install', brush_backdrop_major_version());
  brush_core_site_install_version($profile, $form_options);
}
