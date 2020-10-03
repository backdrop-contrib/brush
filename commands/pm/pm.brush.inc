<?php

/**
 * @file
 *  The brush Project Manager
 *
 * Terminology:
 * - Request: a requested project (string or keyed array), with a name and (optionally) version.
 * - Project: a Backdrop project such as cck or zen.
 * - Extension: a Backdrop module, theme or profile.
 * - Version: a requested version, such as 1.0 or 1.x-dev.
 * - Release: a specific release of a project, with associated metadata.
 */

/**
 * Project is a user requested version update.
 */
define('BRUSH_PM_REQUESTED_UPDATE', 101);

/**
 * User requested version already installed.
 */
define('BRUSH_PM_REQUESTED_CURRENT', 102);

/**
 * User requested project was not packaged by drupal.org.
 */
define('BRUSH_PM_REQUESTED_PROJECT_NOT_PACKAGED', 103);

/**
 * User requested version not found.
 */
define('BRUSH_PM_REQUESTED_VERSION_NOT_FOUND', 104);

/**
 * User requested project not found.
 */
define('BRUSH_PM_REQUESTED_PROJECT_NOT_FOUND', 105);

/**
 * User requested project not updateable.
 */
define('BRUSH_PM_REQUESTED_PROJECT_NOT_UPDATEABLE', 106);


/**
 * Implementation of hook_brush_help().
 */
function pm_brush_help($section) {
  switch ($section) {
    case 'meta:pm:title':
      return dt('Project manager commands');
    case 'meta:pm:summary':
      return dt('Download, enable, examine and update your modules and themes.');
    case 'brush:pm-enable':
      return dt('Enable one or more extensions (modules or themes). Enable dependant extensions as well.');
    case 'brush:pm-disable':
      return dt('Disable one or more extensions (modules or themes). Disable dependant extensions as well.');
    case 'brush:pm-updatecode':
    case 'brush:pm-update':
      $message = dt("Display available update information for Backdrop core and all enabled projects and allow updating to latest recommended releases.");
      if ($section == 'brush:pm-update') {
        $message .= ' '.dt("Also apply any database updates required (same as pm-updatecode + updatedb).");
      }
      $message .= ' '.dt("Note: The user is asked to confirm before the actual update. Backups are performed unless directory is already under version control. Updated projects can potentially break your site. It is NOT recommended to update production sites without prior testing.");
      return $message;
    case 'brush:pm-updatecode-postupdate':
      return dt("This is a helper command needed by updatecode. It is used to check for db updates in a backend process after code updated have been performed. We need to run this task in a separate process to not conflict with old code already in memory.");
      case 'brush:pm-releases':
      return dt("View all releases for a given Backdrop project. Useful for deciding which version to install/update.");
    case 'brush:pm-download':
      return dt("Download Backdrop core or projects from Github (Backdrop core, modules, themes or profiles) and other sources. It will automatically figure out which project version you want based on its recommended release, or you may specify a particular version.

If no --destination is provided, then destination depends on the project type:
  - Profiles will be downloaded to profiles/ in your Backdrop root.
  - Modules and themes will be downloaded to the site specific directory (sites/example.com/modules|themes) if available, or to sites/all/modules|themes.
  - If you're downloading backdrop core or you are not running the command within a bootstrapped backdrop site, the default location is the current directory.
  - Brush commands will be relocated to /usr/share/brush/commands (if available) or ~/.brush. Relocation is determined once the project is downloaded by examining its content. Note you can provide your own function in a commandfile to determine the relocation of any project.");
  }
}

/**
 * Implementation of hook_brush_command().
 */
function pm_brush_command() {
  $update = 'update';
  if (brush_backdrop_major_version() == 5) {
    $update = 'update_status';
  }
  $engines = array(
    'engines' => array(
      'version_control' => 'Integration with VCS in order to easily commit your changes to projects.',
      'package_handler' => 'Determine how to download/checkout new projects and acquire updates to projects.',
    ),
  );
  $update_options = array(
    'security-only' => 'Only update modules that have security updates available. However, if there were other releases of a module between the installed version the security update, other changes to features or functionality may occur.',
    'lock' => 'Add a persistent lock to remove the specified projects from consideration during updates.  Locks may be removed with the --unlock parameter, or overridden by specifically naming the project as a parameter to pm-update or pm-updatecode.  The lock does not affect pm-download.  See also the update-advanced project for similar and improved functionality.',
  );
  $update_suboptions = array(
    'lock' => array(
      'lock-message' => 'A brief message explaining why a project is being locked; displayed during pm-updatecode.  Optional.',
      'unlock' => 'Remove the persistent lock from the specified projects so that they may be updated again.',
    ),
  );

  $items['pm-enable'] = array(
    'description' => 'Enable one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => 'A list of modules or themes. You can use the * wildcard at the end of extension names to enable all matches.',
    ),
    'aliases' => array('en'),
    'deprecated-aliases' => array('enable'),
  );
  $items['pm-disable'] = array(
    'description' => 'Disable one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => 'A list of modules or themes. You can use the * wildcard at the end of extension names to disable multiple matches.',
    ),
    'aliases' => array('dis'),
    'deprecated-aliases' => array('disable'),
  );
  $items['pm-info'] = array(
    'description' => 'Show detailed info for one or more extensions (modules or themes).',
    'arguments' => array(
      'extensions' => 'A list of modules or themes. You can use the * wildcard at the end of extension names to show info for multiple matches. If no argument is provided it will show info for all available extensions.',
    ),
    'aliases' => array('pmi'),
  );
  $items['pm-uninstall'] = array(
    'description' => 'Uninstall one or more modules.',
    'arguments' => array(
      'modules' => 'A list of modules.',
    ),
    'aliases' => array('pmu'),
    'deprecated-aliases' => array('uninstall'),
  );
  $items['pm-list'] = array(
    'description' => 'Show a list of available extensions (modules and themes).',
    'callback arguments' => array(array(), FALSE),
    'options' => array(
      'type' => 'Filter by extension type. Choices: module, theme.',
      'status' => 'Filter by extension status. Choices: enabled, disable and/or \'not installed\'. You can use multiple comma separated values. (i.e. --status="disabled,not installed").',
      'package' => 'Filter by project packages. You can use multiple comma separated values. (i.e. --package="Core - required,Other").',
      'core' => 'Filter out extensions that are not in backdrop core.',
      'no-core' => 'Filter out extensions that are provided by backdrop core.',
      'pipe' => 'Returns a whitespace delimited list of the names of the resulting extensions.',
    ),
    'aliases' => array('pml'),
    'deprecated-aliases' => array('sm'),
  );
  $items['pm-refresh'] = array(
    'description' => 'Refresh update status information.',
    'backdrop dependencies' => array($update),
    'aliases' => array('rf'),
    'deprecated-aliases' => array('refresh'),
  );
  $items['pm-updatecode'] = array(
    'description' => 'Update Backdrop core and contrib projects to latest recommended releases.',
    'backdrop dependencies' => array($update),
    'arguments' => array(
      'projects' => 'Optional. A list of installed projects to update.',
    ),
    'options' => array(
      'pipe' => 'Returns a whitespace delimited list of projects with any of its extensions enabled and their respective version and update information, one project per line. Order: project name, current version, recommended version, update status.',
      'no-core' => 'Only update modules and skip the core update.',
      'self-update' => 'Check for pending updates to brush itself. Set to 0 to avoid check.',
    ) + $update_options,
    'sub-options' => $update_suboptions,
    'aliases' => array('upc'),
    'deprecated-aliases' => array('updatecode'),
    'topics' => array('docs-policy'),
  ) + $engines;
  // Merge all items from above.
  $items['pm-update'] = array_merge($items['pm-updatecode'], array(
    'description' => 'Update Backdrop core and contrib projects and apply any pending database updates (Same as pm-updatecode + updatedb).',
    'aliases' => array('up'),
    'deprecated-aliases' => array('update'),
  ));
  $items['pm-updatecode-postupdate'] = array(
    'description' => 'Notify of pending db updates.',
    'hidden' => TRUE
  );
  $items['pm-releases'] = array(
    'description' => 'Print release information for given projects.',
    'arguments' => array(
      'projects' => 'A list of Backdrop project names. Defaults to \'backdrop\'',
    ),
    'options' => array(
      'dev' => "Show only development releases.",
      'all' => "Shows all available releases instead of the default short list of recent releases.",
    ),
    'examples' => array(
      'brush pm-releases cck zen' => 'View releases for cck and Zen projects for your Backdrop version.',
    ),
    'aliases' => array('rl'),
    'bootstrap' => BRUSH_BOOTSTRAP_MAX,
  );
  $items['pm-download'] = array(
    'description' => 'Download projects from Github or other sources.',
    'examples' => array(
      'brush dl' => 'Download latest recommended release of Backdrop core.',
      'brush dl backdrop' => 'Same as `brush dl`.',
      'brush dl devel zen' => 'Download latest versions of Devel and Zen projects.',
      'brush dl og-1.3' => 'Download a specfic version of Organic groups module for my version of Backdrop.',
      'brush dl diff-6.x-2.x' => 'Download a specific development branch of diff module for a specific Backdrop version.',
      'brush dl views --select' => 'Show a list of recent releases of the views project, prompt for which one to download.',
      'brush dl webform --dev' => 'Download the latest dev release of webform.',
    ),
    'arguments' => array(
      'projects' => 'A comma delimited list of Backdrop project names, with optional version. Defaults to \'backdrop\'',
    ),
    'options' => array(
      'destination' => 'Path to which the project will be copied. If you\'re providing a relative path, note it is relative to the backdrop root (if bootstrapped).',
      'use-site-dir' => 'Force to use the site specific directory. It will create the directory if it doesn\'t exist. If --destination is also present this option will be ignored.',
      'source' => 'The base URL which provides project release history in XML. Defaults to https://updates.backdropcms.org/release-history.',
      'variant' => "Only useful for install profiles. Possible values: 'full', 'projects', 'profile-only'.",
      'dev' => "Download a development release.",
      'select' => "Select the version to download interactively from a list of available releases.",
      'all' => "Useful only with --select; shows all available releases instead of a short list of recent releases.",
      'backdrop-project-rename' => 'Alternate name for "backdrop-x.y" directory when downloading Backdrop project. Defaults to "backdrop".',
      'default-major' => 'Specify the default major version of modules to download when there is no bootstrapped Backdrop site.  Defaults to "7".',
      'pipe' => 'Returns a list of the names of the extensions (modules and themes) contained in the downloaded projects.',
    ),
    'bootstrap' => BRUSH_BOOTSTRAP_MAX,
    'aliases' => array('dl'),
    'deprecated-aliases' => array('download'),
  ) + $engines;
  return $items;
}

/**
 * @defgroup extensions Extensions management.
 * @{
 * Functions to manage extensions.
 */

/**
 * Sort callback function for sorting extensions.
 *
 * It will sort first by type, second by package and third by name.
 */
function _brush_pm_sort_extensions($a, $b) {
  if ($a->type == 'module' && $b->type == 'theme') {
    return -1;
  }
  if ($a->type == 'theme' && $b->type == 'module') {
    return 1;
  }
  $cmp = strcasecmp($a->info['package'], $b->info['package']);
  if ($cmp == 0) {
    $cmp = strcasecmp($a->info['name'], $b->info['name']);
  }
  return $cmp;
}

/**
 * Calculate a extension status based on current status and schema version.
 *
 * @param $extension
 *   Object of a single extension info.
 *
 * @return
 *   String describing extension status. Values: enabled|disabled|not installed
 */
function brush_get_extension_status($extension) {
  if (($extension->type == 'module')&&($extension->schema_version == -1)) {
    $status = "not installed";
  }
  else {
    $status = ($extension->status == 1)?'enabled':'disabled';
  }

  return $status;
}

/**
 * Wrapper of backdrop_get_extensions() with additional information used by
 * pm- commands.
 *
 * @return
 *   An array containing info for all available extensions w/additional info.
 */
function brush_pm_get_extensions() {
  $extensions = brush_get_extensions();
  foreach ($extensions as $key => $extension) {
    if (empty($extension->info['package'])) {
      $extensions[$key]->info['package'] = dt('Other');
    }
  }
  return $extensions;
}

/**
 * Classify extensions as modules, themes or unknown.
 *
 * @param $extensions
 *   Array of extension names, by reference.
 * @param $modules
 *   Empty array to be filled with modules in the provided extension list.
 * @param $themes
 *   Empty array to be filled with themes in the provided extension list.
 */
function brush_pm_classify_extensions(&$extensions, &$modules, &$themes, $extension_info) {
  _brush_pm_expand_extensions($extensions, $extension_info);
  foreach ($extensions as $extension) {
    if (!isset($extension_info[$extension])) {
      continue;
    }
    if ($extension_info[$extension]->type == 'module') {
      $modules[$extension] = $extension;
    }
    else if ($extension_info[$extension]->type == 'theme') {
      $themes[$extension] = $extension;
    }
  }
}

/**
 * Obtain an array of installed projects off the extensions available.
 *
 * A project is considered to be 'enabled' when any of its extensions is
 * enabled.
 *
 * @param array $extensions.
 *   Array of extensions as returned by brush_get_extensions().
 * @return
 *   Array of installed projects with info of version, status and provided
* extensions.
 */
function brush_get_projects(&$extensions = NULL) {
  if (is_null($extensions)) {
    $extensions = brush_get_extensions();
  }
  $projects = array('backdrop' => array('version' => VERSION));
  foreach ($extensions as $extension) {
    // The project name is not available in this cases:
    // 1. the extension is part of backdrop core.
    // 2. the project was checked out from CVS/git and cvs_deploy/git_deploy
    // is not installed.
    // 3. it is not a project hosted in Github.
    if (empty($extension->info['project'])) {
      if (isset($extension->info['version']) && ($extension->info['version'] == VERSION)) {
        $project = 'backdrop';
      }
      else {
        if (is_dir(dirname($extension->filename) . '/CVS') && (!module_exists('cvs_deploy'))) {
          $extension->vcs = 'cvs';
          brush_log(dt('Extension !extension is fetched from cvs. Ignoring.', array('!extension' => $extension->name)), 'debug');
        }
        elseif (is_dir(dirname($extension->filename) . '/.git') && (!module_exists('git_deploy'))) {
          $extension->vcs = 'git';
          brush_log(dt('Extension !extension is fetched from git. Ignoring.', array('!extension' => $extension->name)), 'debug');
        }
        continue;
      }
    }
    else {
        $project = $extension->info['project'];
    }
    // Create/update the project in $projects with the project data.
    if (!isset($projects[$project])) {
      $projects[$project] = array(
        'type' => $extension->type,
        'version' => $extension->info['version'],
        'status' => $extension->status,
        'extensions' => array(),
      );
      if (isset($extension->info['project status url'])) {
        $projects[$project]['status url'] = $extension->info['project status url'];
      }
    }
    elseif ($extension->status != 0) {
      $projects[$project]['status'] = $extension->status;
    }
    $projects[$project]['extensions'][] = $extension->name;
  }

  return $projects;
}

/**
 * Returns a list of enabled modules.
 *
 * This is a simplified version of module_list().
 */
function pm_module_list() {
  $enabled = array();
  $rsc = brush_db_select('system', 'name', 'type=:type AND status=:status', array(':type' => 'module', ':status' => 1));
  while ($row = brush_db_result($rsc)) {
    $enabled[$row] = $row;
  }

  return $enabled;
}

/**
 * @} End of "defgroup extensions".
 */

/**
 * Command callback. Show a list of extensions with type and status.
 */
function brush_pm_list() {
  //--package
  $package_filter = array();
  $package = strtolower(brush_get_option('package'));
  if (!empty($package)) {
    $package_filter = explode(',', $package);
  }
  if (empty($package_filter) || count($package_filter) > 1) {
    $header[] = dt('Package');
  }

  $header[] = dt('Name');

  //--type
  $all_types = array('module', 'theme');
  $type_filter = strtolower(brush_get_option('type'));
  if (!empty($type_filter)) {
    $type_filter = explode(',', $type_filter);
  }
  else {
    $type_filter = $all_types;
  }

  if (count($type_filter) > 1) {
    $header[] = dt('Type');
  }
  foreach ($type_filter as $type) {
    if (!in_array($type, $all_types)) { //TODO: this kind of chck can be implemented brush-wide
      return brush_set_error('BRUSH_PM_INVALID_PROJECT_TYPE', dt('!type is not a valid project type.', array('!type' => $type)));
    }
  }

  //--status
  $all_status = array('enabled', 'disabled', 'not installed');
  $status_filter = strtolower(brush_get_option('status'));
  if (!empty($status_filter)) {
    $status_filter = explode(',', $status_filter);
  }
  else {
    $status_filter = $all_status;
  }
  if (count($status_filter) > 1) {
    $header[] = dt('Status');
  }

  foreach ($status_filter as $status) {
    if (!in_array($status, $status_filter)) { //TODO: this kind of chck can be implemented brush-wide
      return brush_set_error('BRUSH_PM_INVALID_PROJECT_TYPE', dt('!status is not a valid project status.', array('!status' => $status)));
  }
  }

  $header[] = dt('Version');
  $rows[] = $header;

  $extension_info = brush_pm_get_extensions();
  uasort($extension_info, '_brush_pm_sort_extensions');

  $major_version = brush_backdrop_major_version();
  foreach ($extension_info as $key => $extension) {
    if (!in_array($extension->type, $type_filter)) {
      unset($extension_info[$key]);
      continue;
    }
    $status = brush_get_extension_status($extension);
    if (!in_array($status, $status_filter)) {
      unset($extension_info[$key]);
      continue;
    }
    if (($major_version >= 6) and (isset($extension->info['hidden']))) {
      unset($extension_info[$key]);
      continue;
    }

    // filter out core if --no-core specified
    if (brush_get_option('no-core', FALSE)) {
      if ($extension->info['version'] == VERSION) {
        unset($extension_info[$key]);
        continue;
      }
    }

    // filter out non-core if --core specified
    if (brush_get_option('core', FALSE)) {
      if ($extension->info['version'] != VERSION) {
        unset($extension_info[$key]);
        continue;
      }
    }

    // filter by package
    if (!empty($package_filter)) {
      if (!in_array(strtolower($extension->info['package']), $package_filter)) {
        unset($extension_info[$key]);
        continue;
      }
    }

    if (empty($package_filter) || count($package_filter) > 1) {
      $row[] = $extension->info['package'];
    }

    if (($major_version >= 6)||($extension->type == 'module')) {
      $row[] = $extension->info['name'].' ('.$extension->name.')';
    }
    else {
      $row[] = $extension->name;
    }
    if (count($type_filter) > 1) {
      $row[] = ucfirst($extension->type);
    }
    if (count($status_filter) > 1) {
      $row[] = ucfirst($status);
    }
    if (($major_version >= 6)||($extension->type == 'module')) {
      // Suppress notice when version is not present.
      $row[] = @$extension->info['version'];
    }

    $rows[] = $row;
    $pipe[] = $extension->name;
    unset($row);
  }
  brush_print_table($rows, TRUE);

  if (isset($pipe)) {
    // Newline-delimited list for use by other scripts. Set the --pipe option.
    brush_print_pipe($pipe);
  }
  // Set the result for backend invoke
  brush_backend_set_result($extension_info);
}

function brush_pm_find_project_from_extension($extension) {
  $result =  brush_pm_lookup_extension_in_cache($extension);

  if (!isset($result)) {
    // If we can find info on a project that has the same name
    // as the requested extension, then we'll call that a match.
    $info = _brush_pm_get_releases(array($extension));
    if (!empty($info)) {
      $result = $extension;
    }
  }

  return $result;
}

/**
 * Validate callback. Determine the modules and themes that the user would like enabled.
 */
function brush_pm_enable_validate() {
  $args = _convert_csv_to_array(func_get_args());

  $extension_info = brush_get_extensions();

  $recheck = TRUE;
  while ($recheck) {
    $recheck = FALSE;

    // Classify $args in themes, modules or unknown.
    $modules = array();
    $themes = array();
    brush_pm_classify_extensions($args, $modules, $themes, $extension_info);
    $extensions = array_merge($modules, $themes);
    $unknown = array_diff($args, $extensions);

    // Discard and set an error for each unknown extension.
    foreach ($unknown as $name) {
      brush_log(dt('!extension was not found and will not be enabled.', array('!extension' => $name)), 'warning');
    }

    // Discard already enabled and incompatible extensions.
    foreach ($extensions as $name) {
      if ($extension_info[$name]->status) {
        brush_log(dt('!extension is already enabled.', array('!extension' => $name)), 'ok');
      }
      // Check if the extension is compatible with Backdrop core and php version.
      if (brush_extension_check_incompatibility($extension_info[$name])) {
        brush_set_error('BRUSH_PM_ENABLE_MODULE_INCOMPATIBLE', dt('!name is incompatible with the Backdrop version.', array('!name' => $name)));
        if ($extension_info[$name]->type == 'module') {
          unset($modules[$name]);
        }
        else {
          unset($themes[$name]);
        }
      }
    }

    if (!empty($modules)) {
      // Check module dependencies.
      $dependencies = brush_check_module_dependencies($modules, $extension_info);
      $unmet_dependencies = array();
      foreach ($dependencies as $module => $info) {
        if (!empty($info['unmet-dependencies'])) {
          foreach ($info['unmet-dependencies'] as $unmet_module) {
            $unmet_project = brush_pm_find_project_from_extension($unmet_module);
            if (!empty($unmet_project)) {
              $unmet_dependencies[$module][$unmet_project] = $unmet_project;
            }
          }
        }
      }
      if (!empty($unmet_dependencies)) {
        $msgs = array();
        $unmet_project_list = array();
        foreach ($unmet_dependencies as $module => $unmet_projects) {
          $unmet_project_list = array_merge($unmet_project_list, $unmet_projects);
          $msgs[] = dt("!module requires !unmet-projects", array('!unmet-projects' => implode(', ', $unmet_projects), '!module' => $module));
        }
        if (brush_get_option('resolve-dependencies') || brush_confirm(dt("The following projects have unmet dependencies:\n!list\nWould you like to download them?", array('!list' => implode("\n", $msgs))))) {
          // If we did not already print a log message via brush_confirm, then print one now.
          if (brush_get_option('resolve-dependencies')) {
            brush_log(dt("The following projects have unmet dependencies:\n@list\nThey are being downloaded.", array('@list' => implode("\n", $msgs))));
          }
          // Disable BRUSH_AFFIRMATIVE context temporarily.
          $brush_affirmative = brush_get_context('BRUSH_AFFIRMATIVE');
          brush_set_context('BRUSH_AFFIRMATIVE', FALSE);
          // Invoke a new process to download dependencies.
          $result = brush_invoke_process('@self', 'pm-download', $unmet_project_list, array('cache' => TRUE), array('interactive' => TRUE));
          // Restore BRUSH_AFFIRMATIVE context.
          brush_set_context('BRUSH_AFFIRMATIVE', $brush_affirmative);
          // Refresh module cache after downloading the new modules.
          $extension_info = brush_get_extensions();
          $recheck = TRUE;
        }
      }
    }
  }

  if (!empty($modules)) {
    $all_dependencies = array();
    $dependencies_ok = TRUE;
    foreach ($dependencies as $key => $info) {
      if (isset($info['error'])) {
        unset($modules[$key]);
        $dependencies_ok = brush_set_error($info['error']['code'], $info['error']['message']);
      }
      elseif (!empty($info['dependencies'])) {
        // Make sure we have an assoc array.
        $assoc = backdrop_map_assoc($info['dependencies']);
        $all_dependencies = array_merge($all_dependencies, $assoc);
      }
    }
    if (!$dependencies_ok) {
      return FALSE;
    }
    $modules = array_diff(array_merge($modules, $all_dependencies), pm_module_list());
    // Discard modules which doesn't meet requirements.
    require_once brush_get_context('BRUSH_BACKDROP_ROOT') . '/core/includes/install.inc';
    foreach ($modules as $key => $module) {
      // Check to see if the module can be installed/enabled (hook_requirements).
      // See @system_modules_submit
      if (!backdrop_check_module($module)) {
        unset($modules[$key]);
        brush_set_error('BRUSH_PM_ENABLE_MODULE_UNMEET_REQUIREMENTS', dt('Module !module doesn\'t meet the requirements to be enabled.', array('!module' => $module)));
        _brush_log_backdrop_messages();
        return FALSE;
      }
    }
  }

  $searchpath = array();
  foreach (array_merge($modules, $themes) as $name) {
    $searchpath[] = dirname($extension_info[$name]->filename);
  }
  // Add all modules that passed validation to the brush
  // list of commandfiles (if they have any).  This
  // will allow these newly-enabled modules to participate
  // in the pre-pm_enable and post-pm_enable hooks.
  if (!empty($searchpath)) {
    _brush_add_commandfiles($searchpath);
  }

  brush_set_context('PM_ENABLE_EXTENSION_INFO', $extension_info);
  brush_set_context('PM_ENABLE_MODULES', $modules);
  brush_set_context('PM_ENABLE_THEMES', $themes);

  return TRUE;
}

/**
 * Command callback. Enable one or more extensions from downloaded projects.
 * Note that the modules and themes to be enabled were evaluated during the
 * pm-enable validate hook, above.
 */
function brush_pm_enable() {
  // Get the data built during the validate phase
  $extension_info = brush_get_context('PM_ENABLE_EXTENSION_INFO');
  $modules = brush_get_context('PM_ENABLE_MODULES');
  $themes = brush_get_context('PM_ENABLE_THEMES');

  // Inform the user which extensions will finally be enabled.
  $extensions = array_merge($modules, $themes);
  if (empty($extensions)) {
    return brush_log(dt('There were no extensions that could be enabled.'), 'ok');
  }
  else {
    brush_print(dt('The following extensions will be enabled: !extensions', array('!extensions' => implode(', ', $extensions))));
    if(!brush_confirm(dt('Do you really want to continue?'))) {
      return brush_user_abort();
    }
  }

  // Enable themes.
  if (!empty($themes)) {
    brush_theme_enable($themes);
  }

  // Enable modules and pass dependency validation in form submit.
  if (!empty($modules)) {
    brush_module_enable($modules);
    brush_system_modules_form_submit(pm_module_list());
  }

  // Inform the user of final status.
  $rsc = brush_db_select('system', array('name', 'status'), 'name IN (:extensions)', array(':extensions' => $extensions));
  $problem_extensions = array();
  while ($extension = brush_db_fetch_object($rsc)) {
    if ($extension->status) {
      brush_log(dt('!extension was enabled successfully.', array('!extension' => $extension->name)), 'ok');
    }
    else {
      $problem_extensions[] = $extension->name;
    }
  }
  if (!empty($problem_extensions)) {
    return brush_set_error('BRUSH_PM_ENABLE_EXTENSION_ISSUE', dt('There was a problem enabling !extension.', array('!extension' => implode(',', $problem_extensions))));
  }
  // Return the list of extensions enabled
  return $extensions;
}

/**
 * Command callback. Disable one or more extensions.
 */
function brush_pm_disable() {
  $args = _convert_csv_to_array(func_get_args());

  $extension_info = brush_get_extensions();

  // classify $args in themes, modules or unknown.
  $modules = array();
  $themes = array();
  brush_pm_classify_extensions($args, $modules, $themes, $extension_info);
  $extensions = array_merge($modules, $themes);
  $unknown = array_diff($args, $extensions);

  // Discard and set an error for each unknown extension.
  foreach ($unknown as $name) {
    brush_log('BRUSH_PM_ENABLE_EXTENSION_NOT_FOUND', dt('!extension was not found and will not be disabled.', array('!extension' => $name)), 'warning');
  }

  // Discard already disabled extensions.
  foreach ($extensions as $name) {
    if (!$extension_info[$name]->status) {
      if ($extension_info[$name]->type == 'module') {
        unset($modules[$name]);
      }
      else {
        unset($themes[$name]);
      }
      brush_log(dt('!extension is already disabled.', array('!extension' => $name)), 'ok');
    }
  }

  // Discard default theme.
  if (!empty($themes)) {
    $default_theme = brush_theme_get_default();
    if (in_array($default_theme, $themes)) {
      unset($themes[$default_theme]);
      brush_log(dt('!theme is the default theme and can\'t be disabled.', array('!theme' => $default_theme)), 'ok');
    }
  }

  if (!empty($modules)) {
    // Add enabled dependents to the list of modules to disable.
    $dependents = brush_module_dependents($modules, $extension_info);
    $dependents = array_intersect($dependents, pm_module_list());
    $modules = array_merge($modules, $dependents);

    // Discard required modules.
    $required = brush_backdrop_required_modules($extension_info);
    foreach ($required as $module) {
      if (isset($modules[$module])) {
        unset($modules[$module]);
        // No message for hidden modules.
        if (!isset($extension_info[$module]->info['hidden'])) {
          brush_log(dt('!module is a required module and can\'t be disabled.', array('!module' => $module)), 'ok');
        }
      }
    }
  }

  // Inform the user which extensions will finally be disabled.
  $extensions = array_merge($modules, $themes);
  if (empty($extensions)) {
    return brush_log(dt('There were no extensions that could be disabled.'), 'ok');
  }
  else {
    brush_print(dt('The following extensions will be disabled: !extensions', array('!extensions' => implode(', ', $extensions))));
    if(!brush_confirm(dt('Do you really want to continue?'))) {
      return brush_user_abort();
    }
  }

  // Disable themes.
  if (!empty($themes)) {
    brush_theme_disable($themes);
  }

  // Disable modules and pass dependency validation in form submit.
  if (!empty($modules)) {
    brush_module_disable($modules);
    brush_system_modules_form_submit(pm_module_list());
  }

  // Inform the user of final status.
  $rsc = brush_db_select('system', array('name', 'status'), 'name IN (:extensions)', array(':extensions' => $extensions));
  $problem_extensions = array();
  while ($extension = brush_db_fetch_object($rsc)) {
    if (!$extension->status) {
      brush_log(dt('!extension was disabled successfully.', array('!extension' => $extension->name)), 'ok');
    }
    else {
      $problem_extensions[] = $extension->name;
    }
  }
  if (!empty($problem_extensions)) {
    return brush_set_error('BRUSH_PM_DISABLE_EXTENSION_ISSUE', dt('There was a problem disabling !extension.', array('!extension' => implode(',', $problem_extensions))));
  }
}

/**
 * Command callback. Show detailed info for one or more extension.
 */
function brush_pm_info() {
  $args = _convert_csv_to_array(func_get_args());

  $extension_info = brush_pm_get_extensions();
  _brush_pm_expand_extensions($args, $extension_info);
  // If no extensions are provided, select all but the hidden ones.
  if (count($args) == 0) {
    foreach ($extension_info as $key => $extension) {
      if (isset($extension->info['hidden'])) {
        unset($extension_info[$key]);
      }
    }
    $args = array_keys($extension_info);
  }

  foreach ($args as $project) {
    if (isset($extension_info[$project])) {
      $info = $extension_info[$project];
    }
    else {
      brush_log(dt('!project was not found.', array('!project' => $project)), 'warning');
      continue;
    }
    if ($info->type == 'module') {
      $data = _brush_pm_info_module($info);
    }
    else {
      $data = _brush_pm_info_theme($info);
    }
    brush_print_table(brush_key_value_to_array_table($data));
    print "\n";
  }
}

/**
 * Return a string with general info of a extension.
 */
function _brush_pm_info_extension($info) {
  $data['Project'] = $info->name;
  $data['Type'] = $info->type;
  $data['Title'] = $info->info['name'];
  $data['Description'] = $info->info['description'];
  if (!empty($info->info['version'])) {
    $data['Version'] = $info->info['version'];
  } else {
    $data['Version'] = $info->info['backdrop'].'-dev';
  }
  $data['Package'] = $info->info['package'];
  $data['Backdrop'] = $info->info['backdrop'];
  $data['PHP'] = $info->info['php'];
  $data['Status'] = brush_get_extension_status($info);
  $path = $info->filename;
  $path = substr($path, 0, strrpos($path, '/'));
  $data['Path'] = $path;

  return $data;
}

/**
 * Return a string with info of a module.
 */
function _brush_pm_info_module($info) {

  $major_version = brush_backdrop_major_version();

  $data = _brush_pm_info_extension($info);
  if ($info->schema_version > 0) {
    $schema_version = $info->schema_version;
  }
  elseif ($info->schema_version == -1) {
    $schema_version = "no schema installed";
  }
  else {
    $schema_version = "module has no schema";
  }
  $data['Schema version'] = $schema_version;
  if ($major_version == 7) {
    $data['Files'] = implode(', ', $info->info['files']);
  }
  if (count($info->info['dependencies']) > 0) {
    $requires = implode(', ', $info->info['dependencies']);
  }
  else {
    $requires = "none";
  }
  $data['Requires'] = $requires;

  if ($major_version == 6) {
    if (count($info->info['dependents']) > 0) {
      $requiredby = implode(', ', $info->info['dependents']);
    }
    else {
      $requiredby = "none";
    }
    $data['Required by'] = $requiredby;
  }

  return $data;
}

/**
 * Return a string with info of a theme.
 */
function _brush_pm_info_theme($info) {
  $data = _brush_pm_info_extension($info);

  $data['Engine'] = $info->info['engine'];
  if (count($info->info['stylesheets']) > 0) {
    foreach ($info->info['stylesheets'] as $media => $files) {
      $files = implode(', ', array_keys($files));
      $data['Media '.$media] = $files;
    }
  }
  if (count($info->info['scripts']) > 0) {
    $scripts = implode(', ', array_keys($info->info['scripts']));
    $data['Scripts'] = $scripts;
  }
  if (count($info->info['ckeditor_stylesheets']) > 0) {
    $ckeditor_stylesheets = implode(', ', array_values($info->info['ckeditor_stylesheets']));
    $data['CKEditor stylesheets'] = $ckeditor_stylesheets;
  }
  return $data;
}

/**
 * Add extensions that match extension_name*.
 *
 * A helper function for commands that take a space separated list of extension
 * names. It will identify extensions that have been passed in with a
 * trailing * and add all matching extensions to the array that is returned.
 *
 * @param $extensions
 *   An array of extensions, by reference.
 * @param $extension_info
 *   Optional. An array of extension info as returned by brush_get_extensions().
 */
function _brush_pm_expand_extensions(&$extensions, $extension_info = array()) {
  if (empty($extension_info)) {
    $extension_info = brush_get_extensions();
  }
  foreach ($extensions as $key => $extension) {
    if (($wildcard = rtrim($extension, '*')) !== $extension) {
      foreach (array_keys($extension_info) as $extension_name) {
        if (substr($extension_name, 0, strlen($wildcard)) == $wildcard) {
          $extensions[] = $extension_name;
        }
      }
      unset($extensions[$key]);
      continue;
    }
  }
}

/**
 * Command callback. Uninstall one or more modules.
 * // TODO: Use backdrop_execute on system_modules_uninstall_confirm_form so that input is validated.
 */
function brush_pm_uninstall() {
  $modules = _convert_csv_to_array(func_get_args());

  brush_include_engine('backdrop', 'environment');
  $module_info = brush_get_modules();
  $required = backdrop_required_modules();

  // Discards modules which are enabled, not found or already uninstalled.
  foreach ($modules as $key => $module) {
    if (!isset($module_info[$module])) {
      // The module does not exist in the system.
      unset($modules[$key]);
      brush_log(dt('Module !module was not found and will not be uninstalled.', array('!module' => $module)), 'warning');
    }
    else if ($module_info[$module]->status) {
      // The module is enabled.
      unset($modules[$key]);
      brush_log(dt('!module is not disabled. Use `pm-disable` command before `pm-uninstall`.', array('!module' => $module)), 'warning');
    }
    else if ($module_info[$module]->schema_version == -1) { // SCHEMA_UNINSTALLED
      // The module is uninstalled.
      unset($modules[$key]);
      brush_log(dt('!module is already uninstalled.', array('!module' => $module)), 'ok');
    }
    else {
      $dependents = array();
      foreach (brush_module_dependents(array($module), $module_info) as $dependent) {
        if (!in_array($dependent, $required) && ($module_info[$dependent]->schema_version != -1)) {
          $dependents[] = $dependent;
        }
      }
      if (count($dependents)) {
        brush_log(dt('To uninstall !module, the following modules must be uninstalled first: !required', array('!module' => $module, '!required' => implode(', ', $dependents))), 'error');
        unset($modules[$key]);
      }
    }
  }

  // Inform the user which modules will finally be uninstalled.
  if (empty($modules)) {
    return brush_log(dt('There were no modules that could be uninstalled.'), 'ok');
  }
  else {
    brush_print(dt('The following modules will be uninstalled: !modules', array('!modules' => implode(', ', $modules))));
    if(!brush_confirm(dt('Do you really want to continue?'))) {
      return brush_user_abort();
    }
  }

  // Disable the modules.
  brush_module_uninstall($modules);

  // Inform the user of final status.
  foreach ($modules as $module) {
    brush_log(dt('!module was successfully uninstalled.', array('!module' => $module)), 'ok');
  }
}

/**
 * Completes projects' update data with the path to install location on disk.
 *
 * Given an array of release info for available projects, find the path to the install location.
 */
function _pm_get_project_path($data, $lookup) {
  foreach ($data as $name => $release) {
    if ($name == 'backdrop') {
      continue;
    }
    // Array of extensions (modules/themes) within the project.
    $extensions = array_keys($release[$lookup]);
    $path = _pm_find_common_path($release['project_type'], $extensions);
    $reserved = array('modules', 'sites', 'themes');
    if ((in_array(basename($path), $reserved)) && (!in_array($name, $reserved))) {
      brush_log(dt('Error while trying to find the common path for enabled extensions of project !project. Extensions are: !extensions.', array('!project' => $name, '!extensions' => implode(', ', $extensions))), 'error');
      unset($data[$name]);
    }
    else {
      $data[$name]['path'] = $path;
    }
  }

  return $data;
}

/**
 * Helper function to find the common path for a list of extensions in the aim to obtain the project name.
 *
 * @param $project_type
 *  Type of project we're trying to find. Valid values: module, theme.
 * @param $extensions
 *  Array of extension names.
 */
function _pm_find_common_path($project_type, $extensions) {
  // Select the first path as the candidate to be the common prefix.
  $path = backdrop_get_path($project_type, array_pop($extensions));
  // If there's only one extension we are done. Otherwise, we need to find
  // the common prefix for all of them.
  if (count($extensions) > 0) {
    // Iterate over the other projects.
    while($project = array_pop($extensions)) {
      $path2 = backdrop_get_path($project_type, $project);
      // Option 1: same path.
      if ($path == $path2) {
        continue;
      }
      // Option 2: $path is a prefix of $path2.
      if (strpos($path2, $path) === 0) {
        continue;
      }
      // Option 3: $path2 is a prefix of $path.
      if (strpos($path, $path2) === 0) {
        $path = $path2;
        continue;
      }
      // Option 4: no one is a prefix of the other. Find the common
      // prefix by iteratively strip the rigthtmost piece of $path.
      // We will iterate until a prefix is found or path = '.', that on the
      // other hand is a condition theorically impossible to reach.
      do {
        $path = dirname($path);
        if (strpos($path2, $path) === 0) {
          break;
        }
      } while ($path != '.');
    }
  }

  return $path;
}

/**
 * Command callback. Show available releases for given project(s).
 */
function brush_pm_releases() {
  if (!$requests = _convert_csv_to_array(func_get_args())) {
    $requests = array('backdrop');
  }

  $info = _brush_pm_get_releases($requests);
  if (!$info) {
    return brush_log(dt('No valid projects given.'), 'ok');
  }

  foreach ($info as $name => $project) {
    $header = dt('------- RELEASES FOR \'!name\' PROJECT -------', array('!name' => strtoupper($name)));
    $rows = array();
    $rows[] = array(dt('Release'), dt('Date'), dt('Status'));
    $releases = _brush_pm_filter_releases($project['releases'], brush_get_option('all', FALSE), brush_get_option('dev', FALSE));
    foreach ($releases as $release) {
      $rows[] = array(
        $release['version'],
        gmdate('Y-M-d', $release['date']),
        implode(', ', $release['release_status'])
      );
    }
    brush_print($header);
    brush_print_table($rows, TRUE, array(0 => 14));
  }

  return $info;
}

/**
 * Obtain releases for a project's xml as returned by the update service.
 */
function _brush_pm_get_releases_from_xml($xml, $project) {
  // If bootstraped, we can obtain which is the installed release of a project.
  static $installed_projects = FALSE;
  if (!$installed_projects) {
    if (brush_get_context('BRUSH_BOOTSTRAP_PHASE') >= BRUSH_BOOTSTRAP_BACKDROP_FULL) {
      $installed_projects = brush_get_projects();
    }
    else {
      $installed_projects = array();
    }
  }

  foreach (array('title', 'short_name', 'dc:creator', 'api_version', 'recommended_major', 'supported_majors', 'default_major', 'project_status', 'link') as $item) {
    if (array_key_exists($item, $xml)) {
      $value = $xml->xpath($item);
      $project_info[$item] = (string)$value[0];
    }
  }

  $recommended_major = @$xml->xpath("/project/recommended_major");
  $recommended_major = empty($recommended_major)?"":(string)$recommended_major[0];
  $supported_majors = @$xml->xpath("/project/supported_majors");
  $supported_majors = empty($supported_majors)?array():array_flip(explode(',', (string)$supported_majors[0]));
  $releases_xml = @$xml->xpath("/project/releases/release[status='published']");
  $recommended_version = NULL;
  $latest_version = NULL;
  foreach ($releases_xml as $release) {
    $release_info = array();
    foreach (array('name', 'version', 'tag', 'version_major', 'version_extra', 'status', 'release_link', 'download_link', 'date', 'mdhash', 'filesize') as $item) {
      if (array_key_exists($item, $release)) {
        $value = $release->xpath($item);
        $release_info[$item] = (string)$value[0];
      }
    }
    $statuses = array();
    if (array_key_exists($release_info['version_major'], $supported_majors)) {
      $statuses[] = "Supported";
      unset($supported_majors[$release_info['version_major']]);
    }
    if ($release_info['version_major'] == $recommended_major) {
      if (!isset($latest_version)) {
        $latest_version = $release_info['version'];
      }
      // The first stable version (no 'version extra') in the recommended major
      // is the recommended release
      if (empty($release_info['version_extra']) && (!isset($recommended_version))) {
        $statuses[] = "Recommended";
        $recommended_version = $release_info['version'];
      }
    }
    if (!empty($release_info['version_extra']) && ($release_info['version_extra'] == "dev")) {
      $statuses[] = "Development";
    }
    foreach ($release->xpath('terms/term/value') as $release_type) {
      // There are three kinds of release types that we recognize:
      // "Bug fixes", "New features" and "Security update".
      // We will add "Security" for security updates, and nothing
      // for the other kinds.
      if (strpos($release_type, "Security") !== FALSE) {
        $statuses[] = "Security";
      }
    }
    // Add to status whether the project is installed.
    if (isset($installed_projects[$project])) {
      if ($installed_projects[$project]['version'] == $release_info['version']) {
         $statuses[] = dt('Installed');
         $project_info['installed'] = $release_info['version'];
       }
    }
    $release_info['release_status'] = $statuses;
    $releases[$release_info['version']] = $release_info;
  }
  // If there is no -stable- release in the recommended major,
  // then take the latest verion in the recommended major to be
  // the recommended release.
  if (!isset($recommended_version) && isset($latest_version)) {
    $recommended_version = $latest_version;
    $releases[$recommended_version]['release_status'][] = "Recommended";
  }

  $project_info['releases'] = $releases;
  $project_info['recommended'] = $recommended_version;

  return $project_info;
}

/**
 * Helper function for _brush_pm_filter_releases().
 */
function _brush_pm_compare_date($a, $b) {
    if ($a['date'] == $b['date']) {
        return 0;
    }
    if ($a['version_major'] == $b['version_major']) {
      return ($a['date'] > $b['date']) ? -1 : 1;
    }
    return ($a['version_major'] > $b['version_major']) ? -1 : 1;
}

/**
 * Filter a list of releases.
 *
 * @param $releases
 *   Array of release information
 * @param $all
 *   Show all releases. If FALSE, shows only the first release that is
 *   Recommended or Supported or Security or Installed.
 * @param $dev
 *   Show only development release.
 * @param $show_all_until_installed
 *   If TRUE, then all releases will be shown until the INSTALLED release is found,
 *   at which point the algorithm will stop.
 */
function _brush_pm_filter_releases($releases, $all = FALSE, $dev = FALSE, $show_all_until_installed = TRUE) {
  // Start off by sorting releases by release date.
  uasort($releases, '_brush_pm_compare_date');
  // Find version_major for the installed release
  $installed_version_major = FALSE;
  foreach ($releases as $version => $release_info) {
    if (in_array("Installed", $release_info['release_status'])) {
      $installed_version_major = $release_info['version_major'];
    }
  }
  // Now iterate through and filter out the releases we're
  // interested in.
  $options = array();
  $limits_list = array();
  foreach ($releases as $version => $release_info) {
    if (!$dev || ((array_key_exists('version_extra', $release_info)) && ($release_info['version_extra'] == 'dev'))) {
      $saw_unique_status = FALSE;
      foreach ($release_info['release_status'] as $one_status) {
        // We will show the first release of a given kind;
        // after we show the first security release, we show
        // no other.  We do this on a per-major-version basis,
        // though, so if a project has three major versions, then
        // we will show the first security release from each.
        // This rule is overridden by $all and $show_all_until_installed.
        $test_key = $release_info['version_major'] . $one_status;
        if (!array_key_exists($test_key, $limits_list)) {
          $limits_list[$test_key] = TRUE;
          $saw_unique_status = TRUE;
          // Once we see the "Installed" release we will stop
          // showing all releases
          if ($one_status == "Installed") {
            $show_all_until_installed = FALSE;
            $installed_release_date = $release_info['date'];
          }
        }
      }
      if ($all || ($show_all_until_installed && ($installed_version_major == $release_info['version_major'])) || $saw_unique_status) {
        $options[$release_info['version']] = $release_info;
      }
    }
  }
  // If "show all until installed" is still true, that means that
  // we never encountered the installed release anywhere in releases,
  // and therefore we did not filter out any releases at all.  If this
  // is the case, then call ourselves again, this time with
  // $show_all_until_installed set to FALSE from the beginning.
  // The other situation we might encounter is when we do not encounter
  // the installed release, and $options is still empty.  This means
  // that there were no supported or recommented or security or development
  // releases found.  If this is the case, then we will force ALL to TRUE
  // and show everything on the second iteration.
  if (($all === FALSE) && ($show_all_until_installed === TRUE)) {
    $options = _brush_pm_filter_releases($releases, empty($options), $dev, FALSE);
  }
  return $options;
}

/**
 * Return an array of available releases for given project(s).
 *
 * Helper function for pm-download.
 */
function _brush_pm_download_releases_choice($xml, $project, $all = FALSE, $dev = FALSE) {
  $project_info = _brush_pm_get_releases_from_xml($xml, $project);
  $releases = _brush_pm_filter_releases($project_info['releases'], $all, $dev);

  $options = array();
  foreach($releases as $version => $release) {
    $options[$version] = array($version, '-', gmdate('Y-M-d', $release['date']), '-', implode(', ', $release['release_status']));
  }
  return $options;
}

/**
 * Obtain releases info for given projects and fill in status information.
 *
 * It does connect directly to the update service and does not depend on
 * a bootstraped site.
 *
 * @param $requests
 *   An array of Backdrop project names optionally with a version.
 *
 * @see brush_pm_releases()
 */
function _brush_pm_get_releases($requests) {
  $info = array();

  // Parse out project name and version.
  $requests = pm_parse_project_version($requests);

  // Get release history for each request.
  foreach ($requests as $name => $request) {
    $xml = _brush_pm_get_release_history_xml($request);
    if (!$xml) {
      continue;
    }

    $project_info = _brush_pm_get_releases_from_xml($xml, $name);
    $info[$name] = $project_info;
  }
  return $info;
}

/**
 * Command callback. Refresh update status information.
 */
function brush_pm_refresh() {
  // We don't provide for other options here, so we supply an explicit path.
  brush_include_engine('update_info', 'backdrop', NULL, BRUSH_BASE_PATH . '/commands/pm/update_info');

  _pm_refresh();
}

/**
 * Command callback. Execute pm-update.
 */
function brush_pm_update() {
  // Call pm-updatecode.  updatedb will be called in the post-update process.
  $args = _convert_csv_to_array(func_get_args());
  array_unshift($args, 'pm-updatecode');
  return call_user_func_array('brush_invoke', $args);
}

/**
 * Post-command callback.
 * Execute updatedb command after an updatecode - user requested `update`.
 */
function brush_pm_post_pm_update() {
  // Use brush_backend_invoke to start a subprocess. Cleaner that way.
  if (brush_get_context('BRUSH_PM_UPDATED', FALSE) !== FALSE) {
    brush_backend_invoke('updatedb');
  }
}

/**
 * Validate callback for updatecode command. Abort if 'backup' directory exists.
 */
function brush_pm_updatecode_validate() {
  $path = brush_get_context('BRUSH_BACKDROP_ROOT') . '/backup';
  if (is_dir($path) && (realpath(brush_get_option('backup-dir', FALSE)) != $path)) {
    return brush_set_error('', dt('Backup directory !path found. It\'s a security risk to store backups inside the Backdrop tree. Brush now uses by default ~/brush-backups. You need to move !path out of the Backdrop tree to proceed. Note: if you know what you\'re doing you can explicitly set --backup-dir to !path and continue.', array('!path' => $path)));
  }

  // Validate package-handler.
  $package_handler = brush_get_option('package-handler', 'wget');
  brush_include_engine('package_handler', $package_handler);
  return package_handler_validate();
}

/**
 * Post-command callback for updatecode.
 *
 * Execute pm-updatecode-postupdate in a backend process to not conflict with
 * old code already in memory.
 */
function brush_pm_post_pm_updatecode() {
  // Skip if updatecode was invoked by pm-update.
  // This way we avoid being noisy, as updatedb is to be executed.
  $command = brush_get_command();
  if ($command['command'] != 'pm-update') {
    if (brush_get_context('BRUSH_PM_UPDATED', FALSE) !== FALSE) {
      brush_backend_invoke('pm-updatecode-postupdate');
    }
  }
}

/**
 * Command callback. Execute updatecode-postupdate.
 */
function brush_pm_updatecode_postupdate() {
  // Clear the cache, since some projects could have moved around.
  brush_backdrop_cache_clear_all();

  // Notify of pending database updates.
  // Make sure the installation API is available
  require_once brush_get_context('BRUSH_BACKDROP_ROOT') . '/core/includes/install.inc';

  // Load all .install files.
  backdrop_load_updates();

  // @see system_requirements().
  foreach (pm_module_list() as $module) {
    $updates = backdrop_get_schema_versions($module);
    if ($updates !== FALSE) {
      $default = backdrop_get_installed_schema_version($module);
      if (max($updates) > $default) {
        brush_log(dt("You have pending database updates. Run `brush updatedb` or visit update.php in your browser."), 'warning');
        break;
      }
    }
  }
}

/**
 * Determine a candidate destination directory for a particular site path and
 * return it if it exists, optionally attempting to create the directory.
 */
function pm_dl_destination_lookup($type, $backdrop_root, $sitepath, $create = FALSE) {
  switch ($type) {
    case 'module':
      // Prefer sites/all/modules/contrib if it exists.
      $destination = $sitepath . '/modules';
      $contrib = $destination . '/contrib';
      if (is_dir($contrib)) {
        $destination = $contrib;
      }
      break;
    case 'theme':
      $destination = $sitepath . '/themes';
      break;
    case 'theme engine':
      $destination = $sitepath . '/themes/engines';
      break;
    case 'profile':
      $destination = $backdrop_root . '/profiles';
      break;
  }
  if ($create) {
    brush_log(dt('Attempting to create destination directory at !dir', array('!dir' => $destination)));
    brush_mkdir($destination);
  }
  if (is_dir($destination)) {
    brush_log(dt('Using destination directory !dir', array('!dir' => $destination)));
    return $destination;
  }
  brush_log(dt('Could not find destination directory at !dir', array('!dir' => $destination)));
  return FALSE;
}

/**
 * Returns the best destination for a particular download type we can find.
 *
 * It is based on the project type and backdrop and site contexts.
 */
function pm_dl_destination($type) {
  $backdrop_root = brush_get_context('BRUSH_BACKDROP_ROOT');
  $site_root = brush_get_context('BRUSH_BACKDROP_SITE_ROOT', FALSE);
  $full_site_root = $backdrop_root .'/'. $site_root;
  $sites_all = $backdrop_root . '/sites/all';

  $in_site_directory = FALSE;
  // Check if we are running within the site directory.
  if ($full_site_root == substr(brush_cwd(), 0, strlen($full_site_root)) || (brush_get_option('use-site-dir', FALSE))) {
    $in_site_directory = TRUE;
  }

  $destination = '';
  if ($type != 'core') {
    // Attempt 1: If we are in a specific site directory, and the destination
    // directory already exists, then we use that.
    if (empty($destination) && $site_root && $in_site_directory) {
      $create_dir = brush_get_option('use-site-dir', FALSE);
      $destination = pm_dl_destination_lookup($type, $backdrop_root, $full_site_root, $create_dir);
    }
    // Attempt 2: If the destination directory already exists for sites/all,
    // then we use that.
    if (empty($destination) && $backdrop_root) {
      $destination = pm_dl_destination_lookup($type, $backdrop_root, $sites_all);
    }
    // Attempt 3: If a specific (non default) site directory exists and
    // sites/all does not exist, then we create destination in the site
    // specific directory.
    if (empty($destination) && $site_root && $site_root !== 'sites/default' && is_dir($full_site_root) && !is_dir($sites_all)) {
      $destination = pm_dl_destination_lookup($type, $backdrop_root, $full_site_root, TRUE);
    }
    // Attempt 4: If sites/all exists, then we create destination in the
    // sites/all directory.
    if (empty($destination) && is_dir($sites_all)) {
      $destination = pm_dl_destination_lookup($type, $backdrop_root, $sites_all, TRUE);
    }
    // Attempt 5: If site directory exists (even default), then we create
    // destination in the this directory.
    if (empty($destination) && $site_root && is_dir($full_site_root)) {
      $destination = pm_dl_destination_lookup($type, $backdrop_root, $full_site_root, TRUE);
    }
  }
  // Attempt 6: If we didn't find a valid directory yet (or we somehow found
  // one that doesn't exist) we always fall back to the current directory.
  if (empty($destination) || !is_dir($destination)) {
    $destination = brush_cwd();
  }

  return $destination;
}

/*
 * Pick most appropriate release from XML list.
 *
 * @param array $request
 *   An array of version specifications as returned by pm_parse_project_version().
 * @param resource $xml
 *   A handle to the XML document.
 */
function pm_parse_release($request, $xml) {

  if (!empty($request['version'])) {
    $releases = $xml->xpath("/project/releases/release[status='published'][version='" . $request['version'] . "']");
    if (empty($releases)) {
      brush_log(dt("Could not locate specified project version, downloading latest stable version"), 'warning');
    }
  }
  // If that did not work, we will get the first published release for the
  // recommended major version.
  if (empty($releases)) {
    if ($recommended_major = $xml->xpath("/project/recommended_major")) {
      $xpath_releases = "/project/releases/release[status='published'][version_major=" . (string)$recommended_major[0] . "]";
      $releases = @$xml->xpath($xpath_releases);
    }
  }
  // If there are recommended releases (no 'version_extra' elements), then use
  // only recommended releases.  Otherwise, use all; in this case, the
  // recommended release defaults to the latest published release with the
  // right recommended major version number.
  $recommended_releases = array();
  if (!empty($releases)) {
    foreach ($releases as $one_release) {
      if (!array_key_exists('version_extra', $one_release)) {
        $recommended_releases[] = $one_release;
      }
    }
  }
  if (!empty($recommended_releases)) {
    $releases = $recommended_releases;
  }
  $release_type = 'recommended';

  if (brush_get_option('dev', FALSE)) {
    $simple_object = $request;
    $remote_url = 'https://github.com/backdrop-contrib/'.$request['name'].'.git';
    $output = `wget -qO- $remote_url`;
    $dom = new DOMDocument;
    libxml_use_internal_errors(true);
    $dom->loadHTML($output);
    $divs = $dom->getElementsByTagName('a');
    foreach ($divs as $div) {
      foreach ($div->attributes as $attr) {
        $name = $attr->nodeName;
        $value = $attr->nodeValue;
        if (strpos($value, '/backdrop-contrib/'.$request['name'].'/archive/') !== false) {
          $zip_file = substr($value, strrpos($value, '/') + 1);
          $version = substr($zip_file, 0, strrpos($zip_file, '.'));
          $simple_object['name'] = $simple_object['name'].' '.$version;
          $simple_object['version'] = $version;
          unset($simple_object['backdrop_version']);
          unset($simple_object['project_version']);
          $simple_object['download_link'] = 'https://github.com'.$value;

        }
      }
    }
    if ($simple_object) {
      $new_xml = new SimpleXMLElement('<root/>');
      $simple_object = array_flip($simple_object);
      array_walk_recursive($simple_object, array($new_xml, 'addChild'));
      $new_xml_array[0] = $new_xml;
      $releases = $new_xml_array;
    }
  }

  if (brush_get_option('select', FALSE) || empty($releases)) {
    if (empty($releases)) {
      brush_print(dt('There is no !type release for project !project.', array('!type' => $release_type, '!project' => $request['name'])));
    }
    $options = _brush_pm_download_releases_choice($xml, $request['name'], brush_get_option('all', FALSE), brush_get_option('dev', FALSE));
    $choice = brush_choice($options, dt('Choose one of the available releases for project !project:', array('!project' => $request['name'])));
    if ($choice) {
      $releases = $xml->xpath("/project/releases/release[status='published'][version='" . $choice . "']");
    }
    else {
      return FALSE;
    }
  }

  // First published release for the recommended major version is just the
  // first value in $releases.
  return (array)$releases[0];
}

/**
 * Parse out the project name and version and return as a structured array
 *
 * @param $requests an array of project names
 */
function pm_parse_project_version($requests) {
  $requestdata = array();
  $backdrop_version_default = brush_get_context('BRUSH_BACKDROP_MAJOR_VERSION', brush_get_option('default-major',7)) . '.x';
  $backdrop_bootstrap = brush_get_context('BRUSH_BOOTSTRAP_PHASE') > 0;
  foreach($requests as $request) {
    $backdrop_version = $backdrop_version_default;
    $project_version = NULL;
    $version = NULL;
    $project = $request;
    // project-HEAD or project-5.x-1.0-beta
    // '5.x-' is optional, as is '-beta'
    preg_match('/-+(HEAD|(?:(\d+\.x)-+)?(\d+\.[\dx]+.*))$/', $request, $matches);
    if (isset($matches[1])) {
      // The project is whatever we have prior to the version part of the request.
      $project = trim(substr($request, 0, strlen($request) - strlen($matches[0])), ' -');

      if ($matches[1] == 'HEAD' || $matches[2] == 'HEAD') {
        brush_log('BRUSH_PM_HEAD', 'Can\'t download HEAD releases because Backdrop project information only provides for numbered release nodes.', 'warning');
        continue;
      }
      if (!empty($matches[2])) {
        // We have a specified Backdrop core version.
        $backdrop_version = trim($matches[2], '-.');
      }
      if (!empty($matches[3])) {
        if (!$backdrop_bootstrap && empty($matches[2]) && $project != 'backdrop') {
          // We are not working on a bootstrapped site, and the project is not Backdrop itself,
          // so we assume this value is the Backdrop core version and we want the stable project.
          $backdrop_version = trim($matches[3], '-.');
        }
        else {
          // We are working on a bootstrapped site, or the user specified a Backdrop version,
          // so this value must be a specified project version.
          $project_version = trim($matches[3], '-.');
          if (substr($project_version, -1, 1) == 'x') {
            // If a dev branch was requested, we add a -dev suffix.
            $project_version .= '-dev';
          }
        }
      }
    }
    // special checking for 'backdrop-VERSION', == backdrop latest stable release
    elseif ((substr($request,0,7) == 'backdrop-') && (is_numeric(substr($request,7)))) {
      $project = 'backdrop';
      $backdrop_version = substr($request,7) . '.x';
      $project_version = $version;
    }
    if ($project_version) {
      if ($project == 'backdrop') {
        // For project Backdrop, ensure the major version branch is correct, so
        // we can locate the requested or stable release for that branch.
        $project_version_array = explode('.', $project_version);
        $backdrop_version = $project_version_array[0] . '.x';
        // We use the project version only, since it is core.
        $version = $project_version;
      }
      else {
        // For regular projects the version string includes the Backdrop core version.
        $version = $backdrop_version . '-' . $project_version;
      }
    }
    $requestdata[$project] = array(
      'name' => $project,
      'version' => $version,
      'backdrop_version' => $backdrop_version,
      'project_version' => $project_version,
    );
  }
  return $requestdata;
}

function pm_project_types() {
  // Lookup the 'Project type' vocabulary to some standard strings.
  $types = array(
    'core' => 'Backdrop core',
    'profile' => 'Installation profiles',
    'module' => 'Modules',
    'theme' => 'Themes',
    'theme engine' => 'Theme engines',
    'translation' => 'Translations'
  );
  return $types;
}

/**
 * Used by dl and updatecode commands to determine how to download/checkout new projects and acquire updates to projects.
 */
function pm_brush_engine_package_handler() {
  return array(
    'wget' => array(),
    'cvs' => array(
      'options' => array(
        'package-handler=cvs' => 'Use CVS to checkout and update projects.',
      ),
      'sub-options' => array(
        'package-handler=cvs' => array(
          'cvsparams' => 'Add options to the `cvs` program',
          'cvsmethod' => 'Force cvs updates or checkouts (checkout is default unless the directory is managed by a supported version control system).',
          'cvscredentials' => 'A username and password that is sent for cvs checkout command. Defaults to anonymous:anonymous',
        ),
      ),
      'examples' => array(
          'brush [command] cck --cvscredentials=\"name:password\"' =>  'Checkout should use these credentials.',
          'brush [command] cck --cvsparams=\"-C\"' =>  'Overwrite all local changes (Quotes are required).',
          'brush [command] cck --cvsmethod=update' =>  'Will update the project, and try to merge changes, rather than overwriting them. Any conflicts will need to be resolved manually.',
      ),
    ),
    'git_backdroporg' => array(
      'options' => array(
        'package-handler=git_backdroporg' => 'Use git.drupal.org to checkout and update projects.',
      ),
      'sub-options' => array(
        'package-handler=git_backdroporg' => array(
          'gitusername' => 'Your git username as shown on user/[uid]/edit/git. Typically, this is set this in brushrc.php. Omitting this prevents users from pushing changes back to git.drupal.org.',
          'gitsubmodule' => 'Use git submodules for checking out new projects. Existing git checkouts are unaffected, and will continue to (not) use submodules regardless of this setting.',
          'gitcheckoutparams' => 'Add options to the `git checkout` command.',
          'gitcloneparams' => 'Add options to the `git clone` command.',
          'gitfetchparams' => 'Add options to the `git fetch` command.',
          'gitpullparams' => 'Add options to the `git pull` command.',
          'gitsubmoduleaddparams' => 'Add options to the `git submodule add` command.',
          'cache' => 'Use a local git cache and clone projects using --reference.',
        ),
      ),
    ),
  );
}

/**
 * Integration with VCS in order to easily commit your changes to projects.
 */
function pm_brush_engine_version_control() {
  return array(
    'backup' => array(
      'options' => array(
        'version-control=backup' => 'Default engine. Backup all project files before updates.',
      ),
      'sub-options' => array(
        'version-control=backup' => array(
          'no-backup' => 'Do not perform backups.',
          'backup-dir' => 'Specify a directory to backup projects into. Defaults to brush-backups within the home directory of the user running the command. It is forbidden to specify a directory inside your backdrop root.',
        ),
      ),
    ),
    'bzr' => array(
      'signature' => 'bzr root %s',
      'options' => array(
        'version-control=bzr' => 'Quickly add/remove/commit your project changes to Bazaar.',
      ),
      'sub-options' => array(
        'version-control=bzr' => array(
          'bzrsync' => 'Automatically add new files to the Bazaar repository and remove deleted files. Caution.',
          'bzrcommit' => 'Automatically commit changes to Bazaar repository. You must also usw the --bzrsync option.',
        ),
        'bzrcommit' => array(
          'bzrmessage' => 'Override default commit message which is: Brush automatic commit. Project <name> <type> Command: <the brush command line used>',
        )
      ),
      'examples' => array(
        'brush dl cck --version-control=bzr --bzrsync --bzrcommit' =>  'Download the cck project and then add it and commit it to Bazaar.'
      ),
    ),
    'svn' => array(
      'signature' => 'svn info %s',
      'options' => array(
        'version-control=svn' => 'Quickly add/remove/commit your project changes to Subversion.',
      ),
      'sub-options' => array(
        'version-control=svn' => array(
          'svnsync' => 'Automatically add new files to the SVN repository and remove deleted files. Caution.',
          'svncommit' => 'Automatically commit changes to SVN repository. You must also using the --svnsync option.',
          'svnstatusparams' => "Add options to the 'svn status' command",
          'svnaddparams' => 'Add options to the `svn add` command',
          'svnremoveparams' => 'Add options to the `svn remove` command',
          'svnrevertparams' => 'Add options to the `svn revert` command',
          'svncommitparams' => 'Add options to the `svn commit` command',
        ),
        'svncommit' => array(
          'svnmessage' => 'Override default commit message which is: Brush automatic commit: <the brush command line used>',
        )
      ),
      'examples' => array(
        'brush [command] cck --svncommitparams=\"--username joe\"' =>  'Commit changes as the user \'joe\' (Quotes are required).'
      ),
    ),
  );
}

/**
 * Interface for version control systems.
 * We use a simple object layer because we conceivably need more than one
 * loaded at a time.
 */
interface brush_pm_version_control {
  function pre_update(&$project);
  function rollback($project);
  function post_update($project);
  function post_download($project);
  static function reserved_files();
}

/**
 * A simple factory function that tests for version control systems, in a user
 * specified order, and return the one that appears to be appropriate for a
 * specific directory.
 */
function brush_pm_include_version_control($directory = '.') {
  $version_control_engines = brush_get_engines('version_control');
  $version_controls = brush_get_option('version-control', FALSE);
  // If no version control was given, use a list of defaults.
  if (!$version_controls) {
    // Backup engine is the last option.
    $version_controls = array_reverse(array_keys($version_control_engines));
  }
  else {
    $version_controls = array($version_controls);
  }

  // Find the first valid engine in the list, checking signatures if needed.
  $engine = FALSE;
  while (!$engine && count($version_controls)) {
    $version_control = array_shift($version_controls);
    if (isset($version_control_engines[$version_control])) {
      if (!empty($version_control_engines[$version_control]['signature'])) {
        brush_log(dt('Verifying signature for !vcs version control engine.', array('!vcs' => $version_control)), 'debug');
        if (brush_shell_exec($version_control_engines[$version_control]['signature'], $directory)) {
          $engine = $version_control;
        }
      }
      else {
        $engine = $version_control;
      }
    }
  }
  if (!$engine) {
    return brush_set_error('BRUSH_PM_NO_VERSION_CONTROL', dt('No valid version control or backup engine found (the --version-control option was set to "!version-control").', array('!version-control' => $version_control)));
  }

  brush_include_engine('version_control', $version_control);
  $class = 'brush_pm_version_control_' . $engine;
  $instance = new $class();
  $instance->engine = $engine;
  return $instance;
}

/**
 * Implementation of brush_COMMAND_validate().
 */
function brush_pm_download_validate() {
  // Validate the user specified destination directory.
  $destination = brush_get_option('destination');
  if (!empty($destination)) {
    $destination = rtrim($destination, DIRECTORY_SEPARATOR);
    if (!is_dir($destination)) {
      brush_print(dt("The directory !destination does not exist.", array('!destination' => $destination)));
      if (!brush_get_context('BRUSH_SIMULATE')) {
        if (brush_confirm(dt('Would you like to create it?'))) {
          brush_mkdir($destination);
        }
        if (!is_dir($destination)) {
          return brush_set_error('BRUSH_PM_NO_DESTINATION', dt('Unable to create destination directory !destination.', array('!destination' => $destination)));
        }
      }
    }
    if (!is_writable($destination)) {
      return brush_set_error('BRUSH_PM_NO_DESTINATION', dt('Destination directory !destination is not writable.', array('!destination' => $destination)));
    }
    // Ignore --use-site-dir, if given.
    if (brush_get_option('use-site-dir', FALSE)) {
      brush_set_option('use-site-dir', FALSE);
    }
  }

  // Validate --variant or enforce a sane default.
  $variant = brush_get_option('variant', FALSE);
  if ($variant) {
    if (!in_array($variant, array('full', 'projects', 'profile-only'))) {
      brush_log(dt('Unknown variant !variant. Valid values: !variations', array('!variant' => $variant, '!variations' => implode(', ', $variations))), 'error');
    }
  }
  // 'full' and 'projects' variants are only valid for wget package handler.
  $package_handler = brush_get_option('package-handler', 'wget');
  if (($package_handler != 'wget') && ($variant != 'profile-only')) {
    $new_variant = 'profile-only';
    if ($variant) {
      brush_log(dt('Variant !variant is incompatible with !ph package-handler.', array('!variant' => $variant, '!ph' => $package_handler)), 'warning');
    }
  }
  // If we are working on a backdrop root, full variant is not an option.
  else if (brush_get_context('BRUSH_BOOTSTRAP_PHASE') >= BRUSH_BOOTSTRAP_BACKDROP_ROOT) {
    if ((!$variant) || (($variant == 'full') && (!isset($new_variant)))) {
      $new_variant = 'projects';
    }
    if ($variant == 'full') {
      brush_log(dt('Variant full is not a valid option within a Backdrop root.'), 'warning');
    }
  }

  if (isset($new_variant)) {
    brush_set_option('variant', $new_variant);
    if ($variant) {
      brush_log(dt('Switching to --variant=!variant.', array('!variant' => $new_variant)), 'ok');
    }
  }

  // Validate package-handler.
  $package_handler = brush_get_option('package-handler', 'wget');
  brush_include_engine('package_handler', $package_handler);
  // Return value not currently used.
  return package_handler_validate();
}

/**
 * Download the release history xml for the specified request.
 */
function _brush_pm_get_release_history_xml($request) {
  // Backdroped.
    for ($i=1; $i < 10; $i++) {
      $request['project_version'] = $i.'.x';
      $url = brush_get_option('source', 'https://updates.backdropcms.org/release-history') . '/' . $request['name'] . '/' . $request['project_version'];

      if ($path = brush_download_file($url, brush_tempnam($request['name']), brush_get_option('cache-duration-releasexml', 24*3600))) {
        $xml = simplexml_load_file($path);

        if (!$xml->xpath('/error')) {
          // Found the right $xml;
          // Unpublished project?
          $project_status = $xml->xpath('/project/project_status');
          if ($project_status[0][0] == 'unpublished') {
            return brush_set_error('BRUSH_PM_PROJECT_UNPUBLISHED', dt("Project @project is unpublished and has no releases available.", array('@project' => $request['name'])), 'warning');
          }
          brush_log('Downloading release history from ' . $url);
          return $xml;
          break;
        }
      }
    }
    brush_set_error(dt('No release history found for @project_name', array('@project_name' => $request['name'])));
    brush_set_error(dt('Could not download requested project(s).'));
}

/**
 * Command callback. Download Backdrop core or any project.
 */
function brush_pm_download() {
  $package_handler = brush_get_option('package-handler', 'wget');
  brush_include_engine('package_handler', $package_handler);

  if (!$requests = _convert_csv_to_array(func_get_args())) {
    $requests = array('backdrop');
  }

  // Parse out project name and version.
  $requests = pm_parse_project_version($requests);

  // Get release history for each request and download the project.
  $project_types = pm_project_types();
  $project_types_xpath = '(value="' . implode('" or value="', $project_types) . '")';
  foreach ($requests as $name => $request) {

    // Prevent brush from downloading Backdrop.
    if ($request['name'] == 'backdrop') {
      if (brush_get_context('BRUSH_BOOTSTRAP_PHASE') > '0') {
        brush_set_error(dt('Opps, downloading Backdrop inside of another Backdrop is not a good idea!'));
        return;
      }
    }

    $xml = _brush_pm_get_release_history_xml($request);
    if (!$xml) {
      continue;
    }

    // Identify the most appropriate release.
    $release = pm_parse_release($request, $xml);
    if (!$release) {
      continue;
    }

    // Determine what type of project we are to download.
    $json = json_encode($xml);
    $xml_array = json_decode($json,TRUE);
    $request['version'] = $xml_array['api_version'];
    $request['project_version'] = $xml_array['releases']['release']['version'];
    $project_type = $xml_array['type'];
    if ($project_type == 'project_module') {
      $request['project_type'] = 'module';
    }
    elseif ($project_type == 'project_theme')  {
      $request['project_type'] = 'theme';
    }
    elseif ($project_type != 'core') {
      brush_print_r('Not supported project type: '.$project_type);
    }

    // For the other project types we want the project name. Including core
    // variant for profiles.  Note those come with backdrop-x.y in the .tar.gz.
    $request['project_dir'] = $request['name'];

    // Download the project to a temporary location.
    $request['base_project_path'] = brush_tempdir();
    $request['full_project_path'] = $request['base_project_path'] .'/'. $request['project_dir'];

    brush_log(dt('Downloading project !name to !dir ...', array('!name' => $request['name'], '!dir' => $request['base_project_path'])));

    if (!package_handler_download_project($request, $release)) {
      brush_log('Error downloading '.$request['name']);
      continue;
    }

    // Determine the install location for the project.  User provided
    // --destination has preference.
    $destination = brush_get_option('destination');
    if (!empty($destination)) {
      $request['project_install_location'] = brush_get_context('BRUSH_BACKDROP_ROOT') . '/' .$destination;
      $request['project_install_location'] = realpath($destination);
    }
    else {
      $request['project_install_location'] = pm_dl_destination($request['project_type']);
    }

    // If user did not provide --destination, then call the
    // download-destination-alter hook to give the chance to any commandfiles
    // to adjust the install location or abort it.
    if (empty($destination)) {
      $result = brush_command_invoke_all_ref('brush_pm_download_destination_alter', $request, $release);
      if (array_search(FALSE, $result, TRUE) !== FALSE) {
        return FALSE;
      }
    }

    // Load version control engine and detect if (the parent directory of) the
    // project install location is under a vcs.
    if (!$version_control = brush_pm_include_version_control($request['project_install_location'])) {
      continue;
    }

    // Check for brush self update
    if ($request['project_install_location'] == BRUSH_BASE_PATH && $request['name'] == 'brush') {
      if (($backup_dir = brush_prepare_backup_dir()) === FALSE) {
        return FALSE;
      }
      // Move the running brush out of the way
      $brush_backup = $backup_dir . "/brush";
      if (brush_move_dir(BRUSH_BASE_PATH, $brush_backup, TRUE) == FALSE) {
        return brush_set_error('BRUSH_PM_BACKUP_FAILED', dt('Failed to move brush directory !brush to !backup_target', array('!brush' => BRUSH_BASE_PATH, '!backup_target' => $brush_backup)));
      }
      else {
        brush_log(dt("brush backed up to !targetdir", array('!targetdir' => $brush_backup)), "ok");
      }
    }
    else {
      // For all other projects, the final project install location will go in the project_dir.
      $request['project_install_location'] .= '/' . $request['project_dir'];
    }

    // Check if install location already exists.
    if (is_dir($request['project_install_location'])) {
      if (!brush_confirm(dt('Install location !location already exists. Do you want to overwrite it?', array('!location' => $request['project_install_location'])))) {
        brush_log(dt("Skip installation of !project to !dest.", array('!project' => $request['name'], '!dest' => $request['project_install_location'])), 'warning');
        continue;
      }
    }

    // Change full project path for development version as its zip file extracts
    // to directory project-name-version unlike releases (extract to 'project-name').
    if (!is_dir($request['full_project_path'])) {
      $request['full_project_path'] = $request['full_project_path'].'-'.$release['version'];
    }

    // Move the project to the install location.
    if (brush_move_dir($request['full_project_path'], $request['project_install_location'], TRUE)) {
      brush_log(dt("Project !project (!version) downloaded to !dest.", array('!project' => $request['name'], '!version' => $release['version'], '!dest' => $request['project_install_location'])), 'success');
      $request['base_project_path'] = basename($request['project_install_location']);
      $request['full_project_path'] = $request['project_install_location'];
      if ($request['project_install_location'] == BRUSH_BASE_PATH) {
        brush_log(dt("Brush successfully updated to version !version.", array('!version' => $release['version'])), 'success');
      }
    }
    else {
      brush_log(dt("Project !project (!version) could not be downloaded to !dest.", array('!project' => $request['name'], '!version' => $release['version'], '!dest' => $request['project_install_location'])), 'error');
      continue;
    }

    // Post download actions.
    package_handler_post_download($request);
    brush_command_invoke_all('brush_pm_post_download', $request, $release);
    $version_control->post_download($request);

    // Inform the user about available modules a/o themes in the downloaded project.
    brush_pm_extensions_in_project($request);
  }
}

/**
 * Implementation of hook_brush_pm_download_destination_alter().
 *
 * Built-in download-destination-alter hook. This particular version of
 * the hook will move modules that contain only brush commands to
 * /usr/share/brush/commands if it exists, or $HOME/.brush if the
 * site-wide location does not exist.
 */
function pm_brush_pm_download_destination_alter(&$project, $release) {
  // A module is a pure brush command if it has no .module and contain
  // .brush.inc files.
  if ($project['project_type'] == 'module') {
    $module_files = brush_scan_directory($project['full_project_path'], '/.*\.module/');
    if (empty($module_files)) {
      if ($project['name'] == 'brush') {
        // $project['version'] is empty here, so compose the version from the $release structure.
        $brush_release_version = $release['version_major'] . "." . $release['version_patch'] . (empty($release['version_extra']) ? '' : ('-' . $release['version_extra']));
        if(($project['project_install_location'] != BRUSH_BASE_PATH) && ($release['version_major'] >= '4')) {
          $backup_dir = brush_preflight_backup_dir();
          if (brush_confirm(dt('Would you like to back up your current brush version !currentversion to !backup and replace it with brush !version?', array('!version' => $brush_release_version, '!backup' => $backup_dir, '!currentversion' => BRUSH_VERSION)))) {
            $project['project_install_location'] = BRUSH_BASE_PATH;
          }
          else {
            // If we are called via 'brush self-update', then "no" means "do nothing".
            // If we are called via 'brush dl brush', then "no" means "download to cwd".
            if (brush_get_option('self-update', FALSE)) {
              return brush_user_cancel();
            }
          }
        }
      }
      else {
        $brush_command_files = brush_scan_directory($project['full_project_path'], '/.*\.brush.inc/');
        if (!empty($brush_command_files)) {
          $install_dir = brush_get_context('SHARE_PREFIX', '/usr') . '/share/brush/commands';
          if (!is_dir($install_dir) || !is_writable($install_dir)) {
            $install_dir = brush_server_home() . '/.brush';
          }
          // Make the .brush dir if it does not already exist.
          if (!is_dir($install_dir)) {
            brush_mkdir($install_dir);
          }
          // Change the location if the mkdir worked.
          if (is_dir($install_dir)) {
            $project['project_install_location'] = $install_dir;
          }
        }
      }
    }
  }
}

/**
 * Update the locked status of all of the candidate projects
 * to be updated.
 *
 * @param array &$projects
 *   The projects array from pm_updatecode.  $project['locked'] will
 *   be set for every file where a persistent lockfile can be found.
 *   The 'lock' and 'unlock' operations are processed first.
 * @param array $projects_to_lock
 *   A list of projects to create peristent lock files for
 * @param array $projects_to_unlock
 *   A list of projects to clear the persistent lock on
 * @param string $lock_message
 *   The reason the project is being locked; stored in the lockfile.
 *
 * @return array
 *   A list of projects that are locked.
 */
function brush_pm_update_lock(&$projects, $projects_to_lock, $projects_to_unlock, $lock_message = NULL) {
  $locked_result = array();

  // Warn about ambiguous lock / unlock values
  if ($projects_to_lock == array('1')) {
    $projects_to_lock = array();
    brush_log(dt('Ignoring --lock with no value.'), 'warning');
  }
  if ($projects_to_unlock == array('1')) {
    $projects_to_unlock = array();
    brush_log(dt('Ignoring --unlock with no value.'), 'warning');
  }

  // Log if we are going to lock or unlock anything
  if (!empty($projects_to_unlock)) {
    brush_log(dt('Unlocking !projects', array('!projects' => implode(',', $projects_to_unlock))), 'ok');
  }
  if (!empty($projects_to_lock)) {
    brush_log(dt('Locking !projects', array('!projects' => implode(',', $projects_to_lock))), 'ok');
  }

  $backdrop_root = brush_get_context('BRUSH_BACKDROP_ROOT');
  foreach ($projects as $name => $project) {
    if ($name == 'backdrop') {
      continue;
    }
    $message = NULL;
    $lockfile = $backdrop_root . '/' . $project['path'] . '/.brush-lock-update';

    // Remove the lock file if the --unlock option was specified
    if (((in_array($name, $projects_to_unlock)) || (in_array('all', $projects_to_unlock))) && (file_exists($lockfile))) {
      brush_op('unlink', $lockfile);
    }

    // Create the lock file if the --lock option was specified
    if ((in_array($name, $projects_to_lock)) || (in_array('all', $projects_to_lock))) {
      brush_op('file_put_contents', $lockfile, $lock_message != NULL ? $lock_message : "Locked via brush.");
      // Note that the project is locked.  This will work even if we are simulated,
      // or if we get permission denied from the file_put_contents.
      // If the lock is -not- simulated or transient, then the lock message will be
      // read from the lock file below.
      $message = brush_get_context('BRUSH_SIMULATE') ? 'Simulated lock.' : 'Transient lock.';
    }

    // If the persistent lock file exists, then mark the project as locked.
    if (file_exists($lockfile)) {
      $message = trim(file_get_contents($lockfile));
    }

    // If there is a message set, then mark the project as locked.
    if (isset($message)) {
      $projects[$name]['locked'] = !empty($message) ? $message : "Locked.";
      $locked_result[$name] = $project;
    }
  }

  return $locked_result;
}

function _brush_pm_extension_cache_file() {
  return brush_server_home() . "/.brush/brush-extension-cache.inc";
}

function _brush_pm_get_extension_cache() {
  $extension_cache = array();
  $cache_file = _brush_pm_extension_cache_file();

  if (file_exists($cache_file)) {
    include $cache_file;
  }
  if (!array_key_exists('extension-map', $extension_cache)) {
    $extension_cache['extension-map'] = array();
  }
  return $extension_cache;
}

function brush_pm_lookup_extension_in_cache($extension) {
  $result = NULL;
  $extension_cache = _brush_pm_get_extension_cache();
  if (!empty($extension_cache) && array_key_exists($extension, $extension_cache)) {
    $result = $extension_cache[$extension];
  }
  return $result;
}

function brush_pm_put_extension_cache($extension_cache) {
}

function brush_pm_cache_project_extensions($project, $found) {
  $extension_cache = _brush_pm_get_extension_cache();
  foreach($found as $extension) {
    // Simple cache does not handle conflicts
    // We could keep an array of projects, and count
    // how many times each one has been seen...
    $extension_cache[$extension] = $project['name'];
  }
  brush_pm_put_extension_cache($extension_cache);
}

/**
 * Print out all extensions (modules/themes/profiles) found in specified project.
 *
 * Find .info files in the project path and identify modules, themes and
 * profiles. It handles two kind of projects: backdrop core/profiles and
 * modules/themes.
 * It does nothing with theme engine projects.
 */
function brush_pm_extensions_in_project($project) {
  // Mask for brush_scan_directory, to  avoid tests directories.
  $nomask = array('.', '..', 'CVS', 'tests');

  // Backdrop core and profiles can contain modules, themes and profiles.
  if (in_array($project['project_type'], array('core', 'profile'))) {
    $found = array('profile' => array(), 'theme' => array(), 'module' => array());
    // Find all of the .info files
    foreach (brush_scan_directory($project['project_install_location'], "/.*\.info$/", $nomask) as $filename => $info) {
      // Find the project type corresponding the .info file.
     // (Only backdrop >=7.x has .info for .profile)
      $base = dirname($filename) . '/' . $info->name;
      if (is_file($base . '.module')) {
        $found['module'][] = $info->name;
      }
      else if (is_file($base . '.profile')) {
        $found['profile'][] = $info->name;
      }
      else {
        $found['theme'][] = $info->name;
      }
    }
    // Special case: find profiles for backdrop < 7.x (no .info)
    if ($project['backdrop_version'][0] < 7) {
      foreach (brush_scan_directory($project['project_install_location'], "/.*\.profile$/", $nomask) as $filename => $info) {
        $found['profile'][] = $info->name;
      }
    }
    // Log results.
    $msg = "Project !project contains:\n";
    $args = array('!project' => $project['name']);
    foreach (array_keys($found) as $type) {
      if ($count = count($found[$type])) {
        $msg .= " - !count_$type !type_$type: !found_$type\n";
        $args += array("!count_$type" => $count, "!type_$type" => $type, "!found_$type" => implode(', ', $found[$type]));
        if ($count > 1) {
          $args["!type_$type"] = $type.'s';
        }
      }
    }
    brush_log(dt($msg, $args), 'success');
    brush_print_pipe(call_user_func_array('array_merge', array_values($found)));
  }
  // Modules and themes can only contain other extensions of the same type.
  elseif (in_array($project['project_type'], array('module', 'theme'))) {
    // Find all of the .info files
    $found = array();
    foreach (brush_scan_directory($project['project_install_location'], "/.*\.info$/", $nomask) as $filename => $info) {
      $found[] = $info->name;
    }
    // Log results.
    // If there is only one module / theme in the project, only print out
    // the message if is different than the project name.
    if (count($found) == 1) {
      if ($found[0] != $project['name']) {
        $msg = "Project !project contains a !type named !found.";
      }
    }
    // If there are multiple modules or themes in the project, list them all.
    else {
      $msg = "Project !project contains !count !types: !found.";
    }
    if (isset($msg)) {
      brush_print(dt($msg, array('!project' => $project['name'], '!count' => count($found), '!type' => $project['project_type'], '!found' => implode(', ', $found))));
    }
    brush_print_pipe($found);
    // Cache results.
    brush_pm_cache_project_extensions($project, $found);
  }
}

/**
 * Return an array of empty directories.
 *
 * Walk a directory and return an array of subdirectories that are empty. Will
 * return the given directory if it's empty.
 * If a list of items to exclude is provided, subdirectories will be condidered
 * empty even if they include any of the items in the list.
 *
 * @param string $dir
 *   Path to the directory to work in.
 * @param array $exclude
 *   Array of files or directory to exclude in the check.
 */
function brush_find_empty_directories($dir, $exclude = array()) {
  $to_exclude = array_merge(array('.', '..'), $exclude);
  $empty = array();
  $dir_is_empty = TRUE;
  if ($dh = opendir($dir)) {
    while (($file = readdir($dh)) !== FALSE) {
      if (in_array($file, $to_exclude)) {
        continue;
      }
      if (is_dir($dir .'/'. $file)) {
        $subdir = $dir .'/'. $file;
        $subdir_is_empty = TRUE;
        if ($dh2 = opendir($subdir)) {
          while (($file2 = readdir($dh2)) !== FALSE) {
            if (in_array($file2, $to_exclude)) {
              continue;
            }
            $subdir_is_empty = FALSE;
            if (is_dir($subdir . '/'. $file2)) {
              $empty2 = brush_find_empty_directories($subdir . '/'. $file2, $exclude);
              $empty = array_merge($empty, $empty2);
            }
          }
          if ($subdir_is_empty) {
            $empty[] = $subdir . '/'. $file2;
          }
        }
        closedir($dh2);
      }
      $dir_is_empty = FALSE;
    }
  }
  closedir($dh);

  if ($dir_is_empty) {
    return array($dir);
  }

  return $empty;
}
