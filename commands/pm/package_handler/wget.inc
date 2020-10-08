<?php

/**
 * @file Brush PM Wget extension
 */

/**
 * Validate this package handler can run.
 */
function package_handler_validate() {
  // Check wget or curl command exists. Disable possible output.
  $debug = brush_get_context('BRUSH_DEBUG');
  brush_set_context('BRUSH_DEBUG', FALSE);
  $success = brush_shell_exec('wget --version');
  if (!$success) {
    $success = brush_shell_exec('curl --version');
    // Old version of curl shipped in darwin returns error status for --version
    // and --help. Give the chance to use it.
    if (!$success) {
      $success = brush_shell_exec('which curl');
    }
  }
  brush_set_context('BRUSH_DEBUG', $debug);
  if (!$success) {
    return brush_set_error('BRUSH_SHELL_COMMAND_NOT_FOUND', dt('wget nor curl executables found.'));
  }
}

/**
 * Download a project.
 *
 * @param $request Array with information on the request to download.
 * @param $release The release details array from drupal.org.
 */
function package_handler_download_project(&$request, $release) {

  // Install profiles come in several variants. User may specify which one she wants.
  if ($request['project_type'] == 'profile') {
    // @todo Use xpath to get the right file url.
    $files = $release['files'];
    foreach ($files as $key => $file) {
      if ((string)$file->variant == brush_get_option('variant', 'full') && (string)$file->archive_type == 'tar.gz') {
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
  brush_op('chdir', $request['base_project_path']);

  // If exist, remove previously downloaded files.
  brush_shell_exec("rm $filename*");

  // Download the project.
  if (!brush_shell_exec("wget -P . %s", $release['download_link'])) {
    brush_shell_exec("curl -O %s", $release['download_link']);
  }
  if (file_exists($filename) || brush_get_context('BRUSH_SIMULATE')) {
    brush_log("Downloading " . $filename . " was successful.");
  }
  else {
    brush_op('chdir', $olddir);
    return brush_set_error('BRUSH_PM_DOWNLOAD_FAILED', 'Unable to download ' . $filename . ' to ' . $request['base_project_path'] . ' from '. $release['download_link']);
  }

  // Extract the tarball.
  $file_list = brush_tarball_extract($filename, $request['base_project_path'], TRUE);
  brush_op('unlink', $filename);

  // Move untarred directory to project_dir, if distinct.
  if (($request['project_type'] == 'core') || (($request['project_type'] == 'profile') && (brush_get_option('variant', 'core') == 'core'))) {
    // Obtain the dodgy project_dir for drupal core.
    $project_dir = rtrim($file_list[0], DIRECTORY_SEPARATOR);
    if ($request['project_dir'] != $project_dir) {
      $path = $request['base_project_path'];
      brush_move_dir($path . '/'. $project_dir, $path . '/' . $request['project_dir']);
    }
  }
  // Set previous working directory.
  brush_op('chdir', $olddir);

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
