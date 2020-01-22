<?php

/**
 * @file
 *   An early implementation of Site Archive dump/restore. See
 *   http://groups.drupal.org/site-archive-format.
 */

function archive_brush_command() {
  $items['archive-dump'] = array(
    'description' => 'Backup your code, files, and database into a single file.',
    'arguments' => array(
      'targets' => 'Optional. Site specifications, delimited by commas. Typically, list subdirectory name(s) under /sites.',
    ),
    'options' => array(
      'description' => 'Describe the archive contents.',
      'tags' => 'Add tags to the archive manifest. Delimit multiple by commas.',
      'destination' => 'The full path and filename in which the archive should be stored. If omitted, it will be saved to the brush-backups directory and a filename will be generated.',
      'overwrite' => 'Do not fail if the destination file exists; overwrite it instead.',
      'generator' => 'The generator name to store in the MANIFEST file. The default is "Brush archive-dump".',
      'generatorversion' => 'The generator version number to store in the MANIFEST file. The default is ' . BRUSH_VERSION . '.',
      'pipe' => 'Only print the destination of the archive. Useful for scripts that don\'t pass --destination.',
      'preserve-symlinks' => 'Preserve symbolic links.',
    ),
    'examples' => array(
      'brush archive-dump default,example.com,foo.com' => 'Write an archive containing 3 sites in it.',
      'brush archive-dump @sites' => 'Save archive containing all sites in a multi-site.',
      'brush archive-dump default --destination=/backups/mysite.tar' => 'Save archive to custom location.',
    ),
    'bootstrap' => BRUSH_BOOTSTRAP_BACKDROP_SITE,
    'aliases' => array('ard', 'archive-backup', 'arb'),
  );
  $items['archive-restore'] = array(
    'description' => 'Expand a site archive into a Backdrop web site.',
    'arguments' => array(
      'file' => 'The site archive file that should be expanded.',
      'site name' => 'Optional. Which site within the archive you want to restore. Defaults to all.',
    ),
    'options' => array(
      'destination' => 'Specify where the Backdrop site should be expanded. Defaults to the current working directory.',
      'db-prefix' => 'An optional table prefix to use during restore.',
      'db-url' => 'A database URL indicating the target for database restore. If not provided, the archived settings.php is used.',
      'db-su' => 'Account to use when creating the new database. Optional.',
      'db-su-pw' => 'Password for the "db-su" account. Optional.',
      'overwrite' => 'Allow brush to overwrite any files in the destinion.',

    ),
    'examples' => array(
      'brush archive-restore ./example.tar.gz' => 'Restore the files and databases for all sites in the archive.',
      'brush archive-restore ./example.tar.gz example.com' => 'Restore the files and database for example.com site.',
      'brush archive-restore ./example.tar.gz --destination=/var/www/example.com/docroot' => 'Restore archive to a custom location.',
      'brush archive-restore ./example.tar.gz --db-url=mysql://root:pass@127.0.0.1/dbname' => 'Restore archive to a new database (and customize settings.php to point there.).',
    ),
    'bootstrap' => BRUSH_BOOTSTRAP_DROP,
    'aliases' => array('arr'),
  );
  return $items;
}

/**
 * Command callback. Generate site archive file.
 */
function brush_archive_dump($sites_subdirs = '@self') {
  $aliases = brush_sitealias_resolve_sitespecs(explode(',', $sites_subdirs));
  foreach ($aliases as $key => $alias) {
    if (($db_record = sitealias_get_databases_from_record($alias))) {
      $full[$key] = $alias += $db_record;
    }
    else {
      brush_log(dt('DB connections not found for !alias', array('!alias' => $alias)), 'error');
      return;
    }
  }

  // The user can specify a destination filepath or not. That filepath might
  // end with .gz, .tgz, or something else. At the end of this command we will
  // gzip a file, and we want it to end up with the user-specified name (if
  // any), but gzip renames files and refuses to compress files ending with
  // .gz and .tgz, making our lives difficult. Solution:
  //
  // 1. Create a unique temporary base name to which gzip WILL append .gz.
  // 2. If no destination is provided, set $dest_dir to a backup directory and
  // $final_destination to be the unique name in that dir.
  // 3. If a destination is provided, set $dest_dir to that directory and
  // $final_destination to the exact name given.
  // 4. Set $destination, the actual working file we will build up, to the
  // unqiue name in $dest_dir.
  // 5. After gzip'ing $destination, rename $destination.gz to
  // $final_destination.
  //
  // Sheesh.

  // Create the unique temporary name.
  $date = gmdate('Ymd_his');
  $first = current($full);
  $prefix = count($sites_subdirs) > 1 ? 'multiple_sites' : $first['default']['default']['database'];
  $temp_dest_name = "$prefix.$date.tar";

  $final_destination = brush_get_option('destination');
  if (!$final_destination) {
    // No destination provided.
    brush_include_engine('version_control', 'backup');
    $backup = new brush_pm_version_control_backup();
    // TODO: this standard brush pattern leads to a slightly obtuse directory structure.
    $dest_dir = $backup->prepare_backup_dir('archive-dump');
    if (empty($dest_dir)) {
      $dest_dir = brush_tempdir();
    }
    $final_destination = "$dest_dir/$temp_dest_name.gz";
  }
  else {
    // Use the supplied --destination. If it is relative, resolve it
    // relative to the directory in which brush was invoked.
    $command_cwd = getcwd();
    brush_op('chdir', brush_get_context('BRUSH_OLDCWD', getcwd()));
    // This doesn't perform realpath on the basename, but that's okay. This is
    // not path-based security. We just use it for checking for perms later.
    $dest_dir = realpath(dirname($final_destination));
    $final_destination = $dest_dir . '/' . basename($final_destination);
    brush_op('chdir', $command_cwd);
  }

  // $dest_dir is either the backup directory or specified directory. Set our
  // working file.
  $destination = "$dest_dir/$temp_dest_name";

  // Validate the FINAL destination. It should be a file that does not exist
  // (unless --overwrite) in a writable directory (and a writable file if
  // it exists). We check all this up front to avoid failing after a long
  // dump process.
  $overwrite = brush_get_option('overwrite');
  $dest_dir = dirname($final_destination);
  $dt_args = array('!file' => $final_destination, '!dir' => $dest_dir);
  if (is_dir($final_destination)) {
    brush_set_error('BRUSH_ARCHIVE_DEST_IS_DIR', dt('destination !file must be a file, not a directory.', $dt_args));
    return;
  }
  else if (file_exists($final_destination)) {
    if (!$overwrite) {
      brush_set_error('BRUSH_ARCHIVE_DEST_EXISTS', dt('destination !file exists; specify --overwrite to overwrite.', $dt_args));
      return;
    }
    else if (!is_writable($final_destination)) {
      brush_set_error('BRUSH_ARCHIVE_DEST_FILE_NOT_WRITEABLE', dt('destination !file is not writable.', $dt_args));
      return;
    }
  }
  else if (!is_writable(dirname($final_destination))) {
    brush_set_error('BRUSH_ARCHIVE_DEST_DIR_NOT_WRITEABLE', dt('destination directory !dir is not writable.', $dt_args));
    return;
  }

  $docroot_path = realpath(brush_get_context('BRUSH_BACKDROP_ROOT'));
  $docroot = basename($docroot_path);
  $workdir = dirname($docroot_path);
  $dereference = (brush_get_option('preserve-symlinks', FALSE)) ? '' : '--dereference ';
  // Archive Backdrop core, excluding sites dir.
  brush_shell_cd_and_exec($workdir, "tar --exclude '{$docroot}/sites' {$dereference}-cf %s %s", $destination, $docroot);
  // Add sites/all to the same archive.
  brush_shell_cd_and_exec($workdir, "tar {$dereference}-rf %s %s", $destination, "{$docroot}/sites/all");

  // Dump the default database for each site and add to the archive.
  foreach ($full as $key => $alias) {
    if ($db = $alias['databases']['default']['default']) {
      $tmp = brush_tempdir();
      // Use a subdirectory name matching the docroot name.
      brush_mkdir("{$tmp}/{$docroot}");
      // Ensure uniqueness by prefixing key if needed.
      $dbname = $db['database'];
      $result_file = count($full) == 1 ? "$tmp/$dbname.sql" : str_replace('@', '', "$tmp/$key-$dbname.sql");
      $all_dbs[$key] = array(
        'file' => $result_file,
        'driver' => $db['driver'],
      );
      brush_set_option('result-file', $result_file);
      $table_selection = brush_sql_get_table_selection();
      list($dump_exec, $dump_file) = brush_sql_build_dump_command($table_selection, $db);
      brush_shell_exec($dump_exec);
      brush_shell_cd_and_exec($tmp, 'tar --dereference -rf %s %s', $destination, basename($result_file));
    }
  }

  // Build a manifest file AND add sites/$subdir to archive as we go.
  $platform = array(
    'datestamp' => time(),
    'formatversion' => '1.0',
    'generator' => brush_get_option('generator', 'Brush archive-dump'),
    'generatorversion' => brush_get_option('generatorversion', BRUSH_VERSION),
    'description' => brush_get_option('description', ''),
    'tags' => brush_get_option('tags', ''),
  );
  $contents = brush_export_ini(array('Global' => $platform));

  $i=0;
  foreach ($full as $key => $alias) {
    $status = brush_invoke_sitealias_args($alias, 'core-status', array(), array());

    // Add the site specific directory to archive.
    if (!empty($status['object']['%paths']['%site'])) {
      brush_shell_cd_and_exec($workdir, "tar --dereference -rf %s %s", $destination,  "{$docroot}/sites/" . basename($status['object']['%paths']['%site']));
    }

    $site = array(
      'docroot' => BACKDROP_ROOT,
      'sitedir' => @$status['object']['%paths']['%site'],
      'files-public' => @$status['object']['%paths']['%files'],
      'files-private' => @$status['object']['%paths']['%private'],
    );
    // Add info for each DB connection (usually only 1);
    foreach ($all_dbs as $dbkey => $db) {
      $site["database-default-file"] = basename($db['file']);
      $site["database-default-driver"] = $db['driver'];
    }
    // The section title is the sites subdirectory name.
    $info[basename($site['sitedir'])] = $site;
    $contents .= "\n" . brush_export_ini($info);
    unset($info);
    $i++;
  }
  file_put_contents("{$tmp}/MANIFEST.ini", $contents);

  // Add manifest to archive.
  brush_shell_cd_and_exec($tmp, 'tar --dereference -rf %s %s', $destination, "MANIFEST.ini");

  // Compress the archive
  brush_shell_exec("gzip --no-name -f %s", $destination);

  // gzip appends .gz unless the name already ends in .gz, .tgz, or .taz.
  if ("{$destination}.gz" != $final_destination) {
    brush_move_dir("{$destination}.gz", $final_destination, $overwrite);
  }

  brush_log(dt('Archive saved to !dest', array('!dest' => $final_destination)), 'ok');
  brush_print_pipe($final_destination);
  return $final_destination;
}

/**
 * Command callback. Restore web site(s) from a site archive file.
 */
function brush_archive_restore($file, $site_id = NULL) {
  $tmp = brush_tempdir();

  if (!$files = brush_tarball_extract($file, $tmp)) {
    return brush_set_error('BRUSH_ARCHIVE_UNABLE_TO_EXTRACT', dt('Unable to extract site archive tarball to !tmp.', array('!tmp' => $tmp)));
  }

  $manifest = $tmp . '/MANIFEST.ini';
  if (file_exists($manifest)) {
    if (!$ini = parse_ini_file($manifest, TRUE)) {
      return brush_set_error('BRUSH_ARCHIVE_UNABLE_TO_PARSE_MANIFEST', dt('Unable to parse MANIFEST.ini in the archive.'));
    }
  }
  else {
    // No manifest. Try to find docroot and DB dump file.
    $db_file = brush_scan_directory($tmp, '/\.sql$/',  array('.', '..', 'CVS'), 0, 0);
    $directories = glob($tmp . '/*' , GLOB_ONLYDIR);
    $ini = array(
      'Global' => array(),
      'default' => array(
        'docroot' => reset($directories),
        'sitedir' => 'sites/default',
        'database-default-file' => key($db_file),
      ),
    );
  }

  // Grab the first site in the Manifest and move docroot to destination.
  $ini_tmp = $ini;
  unset($ini_tmp['Global']);
  $first = array_shift($ini_tmp);
  $docroot = basename($first['docroot']);
  $destination = brush_get_option('destination', realpath('.') . "/$docroot");
  if (!brush_move_dir("$tmp/$docroot", $destination, brush_get_option('overwrite'))) {
    return brush_set_error('BRUSH_ARCHIVE_UNABLE _RESTORE_FILES', dt('Unable to restore files to !dest', array('!dest' => $destination)));
  }

  // Loop over sites and restore databases and append to settings.php.
  foreach ($ini as $section => $site) {
    if ($section != 'Global' && (is_null($site_id) || $section == $site_id) && !empty($site['database-default-file'])) {

      // Restore database
      $sql_file = $tmp . '/' . $site['database-default-file'];
      if ($db_url = brush_get_option('db-url')) {
        if (empty($site_id) && count($ini) >= 3) {
          // TODO: Use droprc to provide multiple db-urls for multi-restore?
          return brush_set_error('BRUSH_ARCHIVE_UNABLE_MULTI_RESTORE', dt('You must specify a site to restore when the archive contains multiple sites and a db-url is provided.'));
        }
        $db_spec = brush_convert_db_from_db_url($db_url);
      }
      else {
        $site_specification = $destination . '#' . $section;
        if ($return = brush_invoke_sitealias_args($site_specification, 'sql-conf', array(), array('all' => TRUE), array('integrate' => FALSE, 'override-simulated' => TRUE))) {
          $databases = $return['object'];
          $db_spec = $databases['default']['default'];
        }
        else {
          return brush_set_error('BRUSH_ARCHIVE_UNABLE_DISCOVER_DB', dt('Unable to get database details from db-url option or settings.php', array('!key' => $key)));
        }
      }
      brush_sql_empty_db($db_spec);
      _brush_sql_query(NULL, $db_spec, $sql_file);

      // Append new DB info to settings.php.
      if ($db_url) {
        $settingsfile = $destination . '/' . $site['sitedir'] . '/settings.php';
        chmod($settingsfile, 0755); // Need to do something here or else we can't write.
        file_put_contents($settingsfile, "\n// Appended by brush archive-restore command.\n", FILE_APPEND);
        file_put_contents($settingsfile, "\n" . '$databases = ' . var_export(brush_sitealias_convert_db_from_db_url($db_url), TRUE) . ";\n", FILE_APPEND);
        brush_log(dt('Brush appended the new database configuration at settings.php. Optionally remove the old configuration manually.'), 'ok');
      }
    }
  }
  brush_log(dt('Archive restored to !dest', array('!dest' => $destination)), 'ok');
  return $destination;
}
