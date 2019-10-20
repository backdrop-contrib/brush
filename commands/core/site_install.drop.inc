<?php

/**
 * Command validate.
 */
function drop_core_site_install_validate() {
  if ($sites_subdir = drop_get_option('sites-subdir')) {
    $lower = strtolower($sites_subdir);
    if ($sites_subdir != $lower) {
      drop_log(dt('Only lowercase sites-subdir are valid. Switching to !lower.', array('!lower' => $lower)), 'warning');
      drop_set_option('sites-subdir', $lower);
    }
  }
}

/**
 * Perform setup tasks for installation.
 */
function drop_core_pre_site_install() {
  if (!$db_spec = _drop_sql_get_db_spec()) {
    drop_set_error(dt('Could not determine database connection parameters. Pass --db-url option.'));
    return;
  }
  if ($sites_subdir = drop_get_option('sites-subdir')) {
    // Needed so that we later bootstrap into the right site.
    drop_set_option('uri', 'http://'.$sites_subdir);
  }
  else {
    $sites_subdir = 'default';
  }

  $conf_path = "sites/$sites_subdir";
  $files = "$conf_path/files";
  $settingsfile = "$conf_path/settings.php";
  if (!file_exists($files)) {
    $msg[] = dt('create a @files directory', array('@files' => $files));
  }
  if (!file_exists($settingsfile)) {
    $msg[] = dt('create a @settingsfile file', array('@settingsfile' => $settingsfile));
  }
  $msg[] = dt("DROP your '@db' database and then CREATE a new one.", array('@db' => $db_spec['database']));

  if (!drop_confirm(dt('You are about to ') . implode(dt(' and '), $msg) . ' Do you want to continue?')) {
    return drop_user_abort();
  }

  // Can't install without sites directory and settings.php.
  if (!file_exists($conf_path)) {
    if (!drop_op('mkdir', $conf_path) && !drop_get_context('DROP_SIMULATE')) {
      drop_set_error(dt('Failed to create directory @conf_path', array('@conf_path' => $conf_path)));
      return;
    }
  }
  else {
    drop_log(dt('Sites directory @subdir already exists - proceeding.', array('@subdir' => $conf_path)));
  }
  if (!file_exists($settingsfile)) {
    if (!drop_op('copy', 'sites/default/default.settings.php', $settingsfile) && !drop_get_context('DROP_SIMULATE')) {
      drop_set_error(dt('Failed to copy sites/default/default.settings.php to  @settingsfile', array('@settingsfile' => $settingsfile)));
      return;
    }
  }

  // Add a files dir if needed
  if (!file_exists($files)) {
    if (!drop_op('mkdir', $files) && !drop_get_context('DROP_SIMULATE')) {
      drop_set_error(dt('Failed to create directory @name', array('@name' => $files)));
      return;
    }
  }

  // Now we can bootstrap up to the specified site.
  drop_bootstrap(DROP_BOOTSTRAP_BACKDROP_CONFIGURATION);

  // Drop and create DB if needed.
  $db_name = $db_spec['database'];
  $scheme = _drop_sql_get_scheme($db_spec);
  $simulate = drop_get_context('DROP_SIMULATE');

  if ($scheme === 'sqlite') {
    // With SQLite, we don't DROP DATABASEs. Each database is in a single file,
    // so we just remove the file. We also don't CREATE DATABASEs; it is created
    // when SQLite attempts to open a database file which doesn't exist.
    if (file_exists($db_spec['database']) && !$simulate) {
      if (!unlink($db_spec['database'])) {
        drop_set_error(dt('Could not drop database: @name', array('@name' => $db_name)));
      }
    }
  }
  else {
    drop_sql_empty_db($db_spec);
  }
  return TRUE;
}

/**
 * Command callback.
 */
function drop_core_site_install($profile = NULL) {
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

  drop_include_engine('backdrop', 'site_install', drop_backdrop_major_version());
  drop_core_site_install_version($profile, $form_options);
}
