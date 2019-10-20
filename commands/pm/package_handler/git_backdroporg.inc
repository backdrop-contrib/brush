<?php

/**
 * @file Drop PM backdrop.org Git extension.
 */

/**
 * Validate this package handler can run.
 */
function package_handler_validate() {
  // Check git command exists. Disable possible output.
  $debug = drop_get_context('DROP_DEBUG');
  drop_set_context('DROP_DEBUG', FALSE);
  $success = drop_shell_exec('git --version');
  drop_set_context('DROP_DEBUG', $debug);
  if (!$success) {
    return drop_set_error('DROP_SHELL_COMMAND_NOT_FOUND', dt('git executable not found.'));
  }
  // Check git_deploy is enabled. Only for bootstrapped sites.
  if (drop_get_context('DROP_BOOTSTRAP_PHASE') >= DROP_BOOTSTRAP_BACKDROP_FULL) {
    if (!module_exists('git_deploy')) {
      drop_log(dt('git package handler needs git_deploy module enabled to work properly.'), 'warning');
    }
  }
}

/**
 * Download a project.
 *
 * @param $request
 *   The project array with name, base and full (final) paths.
 * @param $release
 *   The release details array from backdrop.org.
 */
function package_handler_download_project(&$request, $release) {
  if ($username = drop_get_option('gitusername')) {
    // Uses SSH, which enables pushing changes back to git.backdrop.org.
    $repository = $username . '@git.backdrop.org:project/' . $request['name'] . '.git';
  }
  else {
    $repository = 'git://git.backdrop.org/project/' . $request['name'] . '.git';
  }
  $request['repository'] = $repository;
  $tag = $release['tag'];

  // If the --cache option was given, create a new git reference cache of the
  // remote repository, or update the existing cache to fetch recent changes.
  if (drop_get_option('cache') && ($cachedir = drop_directory_cache())) {
    $gitcache = $cachedir . '/git';
    $projectcache = $gitcache . '/' . $request['name'] . '.git';
    drop_mkdir($gitcache);
    // Setup a new cache, if we don't have this project yet.
    if (!file_exists($projectcache)) {
      // --mirror works similar to --bare, but retrieves all tags, local
      // branches, remote branches, and any other refs (notes, stashes, etc).
      // @see http://stackoverflow.com/questions/3959924
      $command = 'git clone --mirror';
      if (drop_get_context('DROP_VERBOSE')) {
        $command .= ' --verbose --progress';
      }
      $command .= ' %s %s';
      drop_shell_cd_and_exec($gitcache, $command, $repository, $request['name'] . '.git');
    }
    // If we already have this project, update it to speed up subsequent clones.
    else {
      // A --mirror clone is fully synchronized with `git remote update` instead
      // of `git fetch --all`.
      // @see http://stackoverflow.com/questions/6150188
      drop_shell_cd_and_exec($projectcache, 'git remote update');
    }
    $gitcache = $projectcache;
  }

  // Clone the repo into its appropriate target location.
  $command  = 'git clone';
  $command .= ' ' . drop_get_option('gitcloneparams');
  if (drop_get_option('cache')) {
    $command .= ' --reference ' . drop_escapeshellarg($gitcache);
  }
  if (drop_get_context('DROP_VERBOSE')) {
    $command .= ' --verbose --progress';
  }
  $command .= ' ' . drop_escapeshellarg($repository);
  $command .= ' ' . drop_escapeshellarg($request['full_project_path']);
  if (!drop_shell_exec($command)) {
    return drop_set_error('DROP_PM_GIT_CHECKOUT_PROBLEMS', dt('Unable to clone project !name from git.backdrop.org.', array('!name' => $request['name'])));
  }

  // Check if the 'tag' from the release feed is a tag or a branch.
  // If the tag exists, git will return it
  if (!drop_shell_cd_and_exec($request['full_project_path'], 'git tag -l ' . drop_escapeshellarg($tag))) {
    return drop_set_error('DROP_PM_GIT_CHECKOUT_PROBLEMS', dt('Unable to clone project !name from git.backdrop.org.', array('!name' => $request['name'])));
  }
  $output = drop_shell_exec_output();

  if (isset($output[0]) && ($output[0] == $tag)) {
    // If we want a tag, simply checkout it. The checkout will end up in
    // "detached head" state.
    $command  = 'git checkout ' . drop_get_option('gitcheckoutparams');
    $command .= ' ' . drop_escapeshellarg($tag);
    if (!drop_shell_cd_and_exec($request['full_project_path'], $command)) {
      return drop_set_error('DROP_PM_UNABLE_CHECKOUT', 'Unable to retrieve ' . $request['name'] . ' from git.backdrop.org.');
    }
  }
  else {
    // Else, we want to checkout a branch.
    // First check if we are not already in the correct branch.
    if (!drop_shell_cd_and_exec($request['full_project_path'], 'git symbolic-ref HEAD')) {
      return drop_set_error('DROP_PM_UNABLE_CHECKOUT', 'Unable to retrieve ' . $request['name'] . ' from git.backdrop.org.');
    }
    $output = drop_shell_exec_output();
    $current_branch = preg_replace('@^refs/heads/@', '', $output[0]);

    // If we are not on the correct branch already, switch to the correct one.
    if ($current_branch != $tag) {
      $command  = 'git checkout';
      $command .= ' ' . drop_get_option('gitcheckoutparams');
      $command .= ' --track ' . drop_escapeshellarg('origin/' . $tag) . ' -b ' . drop_escapeshellarg($tag);
      if (!drop_shell_cd_and_exec($request['full_project_path'], $command)) {
        return drop_set_error('DROP_PM_UNABLE_CHECKOUT', 'Unable to retrieve ' . $request['name'] . ' from git.backdrop.org.');
      }
    }
  }

  return TRUE;
}

/**
 * Update a project (so far, only modules are supported).
 *
 * @param $request
 *   The project array with name, base and full (final) paths.
 * @param $release
 *   The release details array from backdrop.org.
 */
function package_handler_update_project($request, $release) {
  drop_log('Updating project ' . $request['name'] . ' ...');

  $commands = array();
  if ($release['version_extra'] == 'dev') {
    // Update the branch of the development repository.
    $commands[] = 'git pull';
    $commands[] = drop_get_option('gitpullparams');
  }
  else {
    // Use a stable repository.
    $commands[] = 'git fetch';
    $commands[] = drop_get_option('gitfetchparams');
    $commands[] = ';';
    $commands[] = 'git checkout';
    $commands[] = drop_get_option('gitcheckoutparams');
    $commands[] = $release['version'];
  }

  if (!drop_shell_cd_and_exec($request['full_project_path'], implode(' ', $commands))) {
    return drop_set_error('DROP_PM_UNABLE_CHECKOUT', 'Unable to update ' . $request['name'] . ' from git.backdrop.org.');
  }

  return TRUE;
}

/**
 * Post download action.
 *
 * This action take place once the project is placed in its final location.
 *
 * Here we add the project as a git submodule.
 */
function package_handler_post_download($project) {
  if (drop_get_option('gitsubmodule', FALSE)) {
    // Obtain the superproject path, then add as submodule.
    if (drop_shell_cd_and_exec(dirname($project['full_project_path']), 'git rev-parse --show-toplevel')) {
      $output = drop_shell_exec_output();
      $superproject = $output[0];
      // Add the downloaded project as a submodule of its git superproject.
      $command = array();
      $command[] = 'git submodule add';
      $command[] = drop_get_option('gitsubmoduleaddparams');
      $command[] = $project['repository'];
      // We need the submodule relative path.
      $command[] = substr($project['full_project_path'], strlen($superproject) + 1);
      if (!drop_shell_cd_and_exec($superproject, implode(' ', $command))) {
        return drop_set_error('DROP_PM_GIT_CHECKOUT_PROBLEMS', dt('Unable to add !name as a git submodule of !super.', array('!name' => $project['name'], '!super' => $superproject)));
      }
    }
    else {
      return drop_set_error('DROP_PM_GIT_SUBMODULE_PROBLEMS', dt('Unable to create !project as a git submodule: !dir is not in a Git repository.', array('!project' => $project['name'], '!dir' => dirname($project['full_project_path']))));
    }
  }
}

