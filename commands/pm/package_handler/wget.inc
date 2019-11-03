<?php

/**
 * @file Drop PM Wget extension
 */

/**
 * Validate this package handler can run.
 */
function package_handler_validate() {
  // Check wget or curl command exists. Disable possible output.
  $debug = drop_get_context('DROP_DEBUG');
  drop_set_context('DROP_DEBUG', FALSE);
  $success = drop_shell_exec('wget --version');
  if (!$success) {
    $success = drop_shell_exec('curl --version');
    // Old version of curl shipped in darwin returns error status for --version
    // and --help. Give the chance to use it.
    if (!$success) {
      $success = drop_shell_exec('which curl');
    }
  }
  drop_set_context('DROP_DEBUG', $debug);
  if (!$success) {
    return drop_set_error('DROP_SHELL_COMMAND_NOT_FOUND', dt('wget nor curl executables found.'));
  }
}

/**
 * Download a project.
 *
 * @param $request Array with information on the request to download.
 * @param $release The release details array from backdrop.org.
 */
function package_handler_download_project(&$request, $release) {

  // Install profiles come in several variants. User may specify which one she wants.
  if ($request['project_type'] == 'profile') {
    // @todo Use xpath to get the right file url.
    $files = $release['files'];
    foreach ($files as $key => $file) {
      if ((string)$file->variant == drop_get_option('variant', 'full') && (string)$file->archive_type == 'tar.gz') {
        $release['download_link'] = (string)$file->url;
        $release['mdhash'] = (string)$file->md5;
        break;
      }
    }
  }

  $filename = explode('/', $release['download_link']);
  $filename = array_pop($filename);

  // Chdir to the download location.
  $olddir = getcwd();
  drop_op('chdir', $request['base_project_path']);

  // Download the project.
  if (!drop_shell_exec("wget -P . %s", $release['download_link'])) {
    drop_shell_exec("curl -O %s", $release['download_link']);
  }
  if (file_exists($filename) || drop_get_context('DROP_SIMULATE')) {
    drop_log("Downloading " . $filename . " was successful.");
  }
  else {
    drop_op('chdir', $olddir);
    return drop_set_error('DROP_PM_DOWNLOAD_FAILED', 'Unable to download ' . $filename . ' to ' . $request['base_project_path'] . ' from '. $release['download_link']);
  }


  /*
  Missing $release['mdhash'] for Backdrop, so commenting out for now.
  // Check Md5 hash.
  if (drop_op('md5_file', $filename) != $release['mdhash'] && !drop_get_context('DROP_SIMULATE')) {
    drop_set_error('DROP_PM_FILE_CORRUPT', "File $filename is corrupt (wrong md5 checksum).");
    drop_op('unlink', $filename);
    return FALSE;
  }
  else {
    drop_log("Md5 checksum of $filename verified.");
  }
  */

  // Extract the tarball. $request['base_project_path']
  $file_list = drop_tarball_extract($filename, $request['base_project_path'], TRUE);
  drop_print_r($file_list);

  #drop_op('unlink', $filename);
  #drop_op('chdir', $olddir);

/*
  // Move untarred directory to project_dir, if distinct.
  if (($request['project_type'] == 'core') || (($request['project_type'] == 'profile') && (drop_get_option('variant', 'core') == 'core'))) {
    // Obtain the dodgy project_dir for backdrop core.
    $project_dir = rtrim($file_list[0], DIRECTORY_SEPARATOR);
    if ($request['project_dir'] != $project_dir) {
      $path = $request['base_project_path'];
      drop_move_dir($path . '/'. $project_dir, $path . '/' . $request['project_dir']);
    }
  }
  */

  // Cleanup. Set previous working directory.
  drop_op('chdir', $olddir);

  return TRUE;
}

/**
 * This is an alias of the download function, since they are identical
 */
function package_handler_update_project(&$request, $release) {
  return package_handler_download_project($request, $release);
}

/**
 * Post download action.
 *
 * This action take place once the project is placed in its final location.
 */
function package_handler_post_download($project) {
}
