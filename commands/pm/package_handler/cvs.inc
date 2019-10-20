<?php

/**
 * @file Drop PM CVS extension
 */

/**
 * Validate this package handler can run.
 */
function package_handler_validate() {
  // Check cvs command exists. Disable possible output.
  $debug = drop_get_context('DROP_DEBUG');
  drop_set_context('DROP_DEBUG', FALSE);
  $success = drop_shell_exec('cvs --version');
  drop_set_context('DROP_DEBUG', $debug);
  if (!$success) {
    return drop_set_error('DROP_SHELL_COMMAND_NOT_FOUND', dt('cvs executable not found.'));
  }
  // Check cvs_deploy is enabled. Only for bootstrapped sites.
  if (drop_get_context('DROP_BOOTSTRAP_PHASE') >= DROP_BOOTSTRAP_BACKDROP_FULL) {
    if (!module_exists('cvs_deploy')) {
      drop_log(dt('cvs package handler needs cvs_deploy module enabled to work properly.'), 'warning');
    }
  }
}

/**
 * Install a project.
 *
 * @param $project The project array with name, base and full (final) paths.
 * @param $release The release details array from backdrop.org
 */
function package_handler_download_project(&$project, $release) {
  // Check it out.
  drop_pm_cvs($project, $release);

  if (!drop_get_context('DROP_SIMULATE')) {
    if (is_dir($project['full_project_path'])) {
      drop_log("Checking out " . $project['name'] . " was successful.");
      return TRUE;
    }
    else {
      return drop_set_error('DROP_PM_CVS_CHECKOUT_PROBLEMS', dt("Unable to check out !project to !destination from cvs.backdrop.org", array('!project' => $project['name'], '!destination' => $project['full_project_path'])));
    }
  }
}

/**
 * Update a project (so far, only modules are supported).
 *
 * @param $project The project array with name, base and full (final) paths.
 * @param $release The release details array from backdrop.org
 */
function package_handler_update_project(&$project, $release) {
  drop_log('Updating project ' . $project['name'] . ' ...');

  // Check out a fresh copy, or update an existing one.
  drop_pm_cvs($project, $release);

  if (is_dir($project['full_project_path']) && !drop_get_context('DROP_SIMULATE')) {
    drop_log("Updating of " . $project['name'] . " was successful.");
    return TRUE;
  }
  else {
    return drop_set_error('DROP_PM_CVS_UPDATE_PROBLEMS', dt("Unable to update !project from cvs.backdrop.org", array('!project' => $project['name'])));
  }
}

/**
 * General CVS helper function.
 *
 * @param $project The project array with name, base and full (final) paths.
 * @param $release The release details array from backdrop.org
 */
function drop_pm_cvs(&$project, $release) {
  // Build the cvs command to execute.
  $command = array('cvs');

  // Global options.
  $command[] = '-z6';
  // cvs root.
  $cvsroot = '-d:pserver:' . drop_get_option('cvscredentials', 'anonymous:anonymous') . '@cvs.backdrop.org:/cvs/';
  $cvsroot .= ($project['project_type'] == 'core')?'backdrop':'backdrop-contrib';
  $command[] = $cvsroot;

  // CVS command ("cvs method").
  $cvsmethod = drop_get_option('cvsmethod', FALSE);
  if (empty($cvsmethod)) {
    $cvsmethod = 'checkout';
     // If we have an existing working copy we update.
    if (is_dir($project['full_project_path'] . '/CVS')) {
      $cvsmethod = 'update';
    }
  }
  $command[] = $cvsmethod;

  // CVS command options.
  $cvsparams = drop_get_option('cvsparams', FALSE);
  // common options for any command.
  $command[] = '-r '. $release['tag'];
  // command specific options.
  if ($cvsmethod == 'checkout') {
    // checkout dir.
    $command[] = '-d ' . $project['project_dir'];
    // path to cvs 'module' to check out.
    if ($project['project_type'] == 'core') {
      $command[] = $project['name']; // backdrop
    }
    else {
      // strtr for 'theme engine' type.
      $command[] = 'contributions/' . strtr($project['project_type'], ' ' ,'-') . 's/' . $project['name'];
    }
  }
  else {
    if ($cvsparams === FALSE) {
      // By default we update overwriting changes, however if we have an
      // existing CVS checkout that is version controlled then the default is
      // to update in place, which will attempt to merge changes but we assume
      // anyone using a VCS is competent enough to deal with this!
      $reserved_files = drop_version_control_reserved_files();
      $overwrite = TRUE;
      foreach ($reserved_files as $file) {
        if (file_exists($project['full_project_path'] . '/' . $file)) {
          $overwrite = FALSE;
          break;
        }
      }
      $command[] = $overwrite?'-dPC':'-dP';
    }
    // Directory to work on.
    $command[] = $project['project_dir'];
  }

  // CVS only accepts relative paths. We will cd in the checkout path right
  // before running the cvs command.
  if (!drop_shell_cd_and_exec($project['base_project_path'], implode(' ', $command))) {
    return drop_set_error('DROP_PM_UNABLE_CHECKOUT', dt('Unable to !op !project from cvs.backdrop.org.', array('!op' => $cvsmethod, '!project' => $project['name'])));
  }
}

/**
 * Post download action.
 *
 * This action take place once the project is placed in its final location.
 */
function package_handler_post_download($project) {
}

