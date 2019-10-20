<?php

/**
 * @file Drop pm SVN extension
 */

class drop_pm_version_control_svn implements drop_pm_version_control {

  /**
   * Implementation of pre_update().
   */
  public function pre_update(&$project, $items_to_test = array()) {
    // If items to test is empty, test everything; otherwise, pass just
    // the list of files to test to svn status.
    $args = array_keys($items_to_test);
    array_unshift($args, 'svn status '. drop_get_option('svnstatusparams') . str_repeat('%s ', count($args)));
    array_unshift($args, $project['full_project_path']);
    if (call_user_func_array('drop_shell_cd_and_exec', $args)) {
      $output = preg_grep('/^[ ACDMRX?!~][ CM][ L][ +][ SX][ K]/', drop_shell_exec_output());
      if (!empty($output)) {
        return drop_set_error('DROP_PM_SVN_LOCAL_CHANGES', dt("The SVN working copy at !path appears to have uncommitted changes (see below). Please commit or revert these changes before continuing:\n!output", array('!path' => $project['full_project_path'], '!output' => implode("\n", $output))));
      }
    }
    else {
      return drop_set_error('DROP_PM_SVN_NOT_FOUND', dt("Drop was unable to get the svn status on !path.\nThe specific errors are below:\n!errors", array('!path' => $project['full_project_path'], '!errors' => implode("\n", drop_shell_exec_output()))));
    }

    // Check for incoming updates
    $args = array_keys($items_to_test);
    array_unshift($args, 'svn status -u '. drop_get_option('svnstatusparams') . str_repeat('%s ', count($args)));
    array_unshift($args, $project['full_project_path']);
    if (call_user_func_array('drop_shell_cd_and_exec', $args)) {
      $output = preg_grep('/\*/', drop_shell_exec_output());
      if (!empty($output)) {
        return drop_set_error('DROP_PM_SVN_REMOTE_CHANGES', dt("The SVN working copy at !path appears to be out of date with the repository (see below). Please run 'svn update' to pull down changes before continuing:\n!output", array('!path' => $project['full_project_path'], '!output' => implode("\n", $output))));
      }
    }
    else {
      return drop_set_error('DROP_PM_SVN_NOT_FOUND', dt("Drop was unable to get the svn remote status on !path. Check that you have connectivity to the repository.\nThe specific errors are below:\n!errors", array('!path' => $project['full_project_path'], '!errors' => implode("\n", drop_shell_exec_output()))));
    }
    return TRUE;
  }

  /**
   * Implementation of rollback().
   */
  public function rollback($project) {
    if (drop_shell_exec('svn revert '. drop_get_option('svnrevertparams') .' '. $project['full_project_path'])) {
      $output = drop_shell_exec_output();
      if (!empty($output)) {
        return drop_set_error('DROP_PM_SVN_LOCAL_CHANGES', dt("The SVN working copy at !path appears to have uncommitted changes (see below). Please commit or revert these changes before continuing:\n!output", array('!path' => $project['full_project_path'], '!output' => implode("\n", $output))));
      }
    }
    else {
      return drop_set_error('DROP_PM_SVN_NOT_FOUND', dt("Drop was unable to get the svn status on !path. Check that you have Subversion \ninstalled and that this directory is a subversion working copy.\nThe specific errors are below:\n!errors", array('!path' => $project['full_project_path'], '!errors' => implode("\n", drop_shell_exec_output()))));
    }
  }

  /**
   * Implementation of post_update().
   */
  public function post_update($project) {
    if ($this->sync($project)) {
      // Only attempt commit on a sucessful sync
      $this->commit($project);
    }
  }

  /**
   * Implementation of post_download().
   */
  public function post_download($project) {
    if ($this->sync($project)) {
      // Only attempt commit on a sucessful sync
      $this->commit($project);
    }
  }

  /**
   * Automatically add any unversioned files to Subversion and remove any files
   * that have been deleted on the file system
   */
  private function sync($project) {
    if (drop_get_option('svnsync')) {
      $errors = '';
      if (drop_shell_exec('svn status '. drop_get_option('svnstatusparams') .' '. $project['full_project_path'])) {
        $output = drop_shell_exec_output();
        foreach ($output as $line) {
          if (preg_match('/^\? *(.*)/', $line, $matches)) {
            if (!drop_shell_exec('svn add '. drop_get_option('svnaddparams') .' '. $matches[1])) {
              $errors .= implode("\n", drop_shell_exec_output());
            }
          }
          if (preg_match('/^\! *(.*)/', $line, $matches)) {
            if (!drop_shell_exec('svn remove '. drop_get_option('svnremoveparams') .' '. $matches[1])) {
              $errors .= implode("\n", drop_shell_exec_output());
            }
          }
        }
        if (!empty($errors)) {
          return drop_set_error('DROP_PM_SVN_SYNC_PROBLEMS', dt("Problems were encountered adding or removing files to/from this SVN working copy.\nThe specific errors are below:\n!errors", array('!errors' => $errors)));
        }
      }
      else {
        return drop_set_error('DROP_PM_SVN_NOT_FOUND', dt("Drop was unable to get the svn status on !path. Check that you have Subversion \ninstalled and that this directory is a subversion working copy.\nThe specific errors are below:\n!errors", array('!path' => $project['full_project_path'], '!errors' => implode("\n", drop_shell_exec_output()))));
      }
      return TRUE;
    }
  }

  /**
   * Automatically commit changes to the repository
   */
  private function commit($project) {
    if (drop_get_option('svncommit')) {
      $message = drop_get_option('svnmessage');
      if (empty($message)) {
        $message = dt("Drop automatic commit: \n") . implode(' ', $_SERVER['argv']);
      }
      if (drop_shell_exec('svn commit '. drop_get_option('svncommitparams') .' -m "'. $message .'" '. $project['full_project_path'])) {
        drop_log(dt('Project committed to Subversion successfully'), 'ok');
      }
      else {
        drop_set_error('DROP_PM_SVN_COMMIT_PROBLEMS', dt("Problems were encountered committing your changes to Subversion.\nThe specific errors are below:\n!errors", array('!errors' => implode("\n", drop_shell_exec_output()))));
      }
    }
    else {
      drop_print(dt("You should consider committing the new code to your Subversion repository.\nIf this version becomes undesireable, use Subversion to roll back."));
    }
  }

  public static function reserved_files() {
    return array('.svn');
  }
}
