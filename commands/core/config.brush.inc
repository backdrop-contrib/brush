<?php

/**
 * Implementation of hook_brush_command().
 *
 * @return
 *   An associative array describing your command(s).
 */
function config_brush_command() {
  $items['config-list'] = array(
    'description' => 'Display a list all configuration files.',
    'arguments' => array(
      'name' => 'A string to filter the configuration files by.',
    ),
    'examples' => array(
      'brush config-list' => 'List all configuration files.',
      'brush config-list user' => 'List all configuration files beginning with the string "user".',
    ),
    'options' => array(
      'pipe' => 'Use var_export() to emit executable PHP. Useful for pasting into code.',
    ),
    'aliases' => array('clist', 'vlist'),
  );
  $items['config-get'] = array(
    'description' => 'Get a list of some or all site variables and values.',
    'arguments' => array(
      'config-name' => 'The config object name, for example "system.core".',
      'key' => 'The config key, for example "clean_url".',
    ),
    'examples' => array(
      'brush config-get' => 'List all variables and values.',
      'brush config-get system.core' => 'List all variables in "system.core.json" file.',
      'brush config-get system.core clean_url' => 'List value of the "clean_url" variable in "system.core.json" file.',
      'brush config-get clean_url' => 'List value of the "clean_url" variable in "system.core.json" file.',
    ),
    'options' => array(
      'pipe' => 'Use var_export() to emit executable PHP. Useful for pasting into code.',
    ),
    'aliases' => array('cget','vget'),
  );
  $items['config-set'] = array(
    'description' => "Set a configuration variable.",
    'arguments' => array(
      'name' => 'The name of a variable or the first few letters of its name.',
      'value' => 'The value to assign to the variable.',
    ),
    'options' => array(
      'yes' => 'Skip confirmation if only one variable name matches.',
      'always-set' => 'Always skip confirmation.',
    ),
    'examples' => array(
      'brush vset --yes preprocess_css 1' => 'Set the preprocess_css variable to true. Skip confirmation if variable already exists.',
      'brush vset --always-set site_offline 1' => 'Take the site offline; Skips confirmation even if site_offline variable does not exist.',
      'brush vset pr 1' => 'Choose from a list of variables beginning with "pr" to set to true.',
    ),
    'aliases' => array('cset','vset'),
  );
  $items['config-delete'] = array(
    'description' => "Delete a variable.",
    'arguments' => array(
      'name' => 'The name of a variable or the first few letters of its name.',
    ),
    'options' => array(
      'yes' => 'Skip confirmation if only one variable name matches.',
    ),
    'examples' => array(
      'brush vdel user_pictures' => 'Delete the user_pictures variable.',
      'brush vdel u' => 'Choose from a list of variables beginning with "u" to delete.',
    ),
    'aliases' => array('cdel','vdel'),
  );

  return $items;
}

/**
 * Command callback.
 * List Backdrop site's variables.
 */
function brush_config_list() {
  $args = func_get_args();
  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);
  $config_files = array();
  $rows = array();

  $rows[] = array('File name', 'Label', 'Group');

  foreach ($config_storage->listAll() as $config_name) {
    if (!empty($args['0'])) {
      if (strpos($config_name, $args['0']) !== false) {
        $info = config_get_info($config_name);
        $label = '';
        $group = '';

        if (isset($info['label'])) {
          $label = $info['label'];
        }
        if (isset($info['label_key'])) {
          $label = config_get($config_name, $info['label_key']);
        }
        if (isset($info['label_callback'])) {
          $config = $config_storage->read($config_name);
          $label = $info['label_callback']($config, $config_name);
        }
        if (isset($info['group'])) {
          $group = $info['group'];
        }

        $rows[] = array(
          $config_name,
          $label,
          $group,
        );
      }
    }
    else {
      $info = config_get_info($config_name);
      $label = '';
      $group = '';

      if (isset($info['label'])) {
        $label = $info['label'];
      }
      if (isset($info['label_key'])) {
        $label = config_get($config_name, $info['label_key']);
      }
      if (isset($info['label_callback'])) {
        $config = $config_storage->read($config_name);
        $label = $info['label_callback']($config, $config_name);
      }
      if (isset($info['group'])) {
        $group = $info['group'];
      }

      $rows[] = array(
        $config_name,
        $label,
        $group,
      );
    }
  }

  if (!empty($rows['1'])) {
    brush_print_table($rows, TRUE);
  }
  else {
    brush_set_error('No configuration found for '.$args['0']);
  }
}
/**
 * Command callback.
 * Get a variable's value.
 */
function brush_config_get() {
  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);
  $args = func_get_args();

  if ($args['0']) { // If $config_name provided
    $rows[] = array(
      'Configuration variable',
      'Value',
    );
    $key_found == FALSE;
    foreach ($config_storage->listAll() as $config_name) {
      $config = $config_storage->read($config_name);
      if ($args['0'] == $config_name) {
        $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
        foreach ($config as $variable => $value) {
          if ($args['1']) { // If $variable provided
            if ($variable == $args['1']) {
              if (is_array($value)) {
                $value = 'array('.implode(", ", $value).')';
              }
              $rows[] = array(
                $variable,
                $value,
              );
            }
          }
          else {
            if (is_array($value)) {
              $value = 'array('.implode(", ", $value).')';
            }
            $rows[] = array(
              $variable,
              $value,
            );
          }
        }
      }
      // $args['0'] found in $config_name, probably it's a configuration value.
      if (array_key_exists($args['0'], $config)) {
        $key_found = TRUE;
        $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
        foreach ($config as $variable => $value) {
          if ($variable == $args['0']) {
            if (is_array($value)) {
              $value = 'array('.implode(", ", $value).')';
            }
            $rows[] = array(
              $variable,
              $value,
            );
          }
        }
        brush_print();
        brush_print($title);
        brush_print_table($rows, TRUE);
      }
    }
    if ($key_found == FALSE) {
      if ($title) {
        brush_print();
        brush_print($title);
        brush_print_table($rows, TRUE);
      }
      else {
        brush_set_error('No configuration found for '.$args['0']);
      }
    }  
  }
}

/**
 * Command callback.
 * Set a variable.
 */
function brush_config_set() {
  $args = func_get_args();
  if (!isset($args[0])) {
    return brush_set_error('BRUSH_config_ERROR', dt('No variable specified.'));
  }
  $value = $args[1];
  if (!isset($value)) {
    return brush_set_error('BRUSH_config_ERROR', dt('No value specified.'));
  }

  $result = brush_config_like($args[0]);

  $options[] = "$args[0] ". dt('(new variable)');
  $match = FALSE;
  while (!$match && $name = brush_db_result($result)) {
    if ($name == $args[0]) {
      $options[0] = $name;
      $match = TRUE;
    }
    else {
      $options[] = $name;
    }
  }

  if ((brush_get_option('always-set', FALSE)) || ((count($options) == 1 || $match) && brush_get_context('BRUSH_AFFIRMATIVE'))) {
    variable_set($args[0], $value);
    brush_log(dt('!name was set to !value.', array('!name' => $args[0], '!value' => $value)), 'success');
    return '';
  }
  else {
    $choice = brush_choice($options, 'Enter a number to choose which variable to set.');
    if ($choice !== FALSE) {
      $choice = $options[$choice];
      $choice = str_replace(' ' . dt('(new variable)'), '', $choice);
      brush_op('variable_set', $choice, $value);
      brush_log(dt('!name was set to !value', array('!name' => $choice, '!value' => $value)), 'success');
    }
  }
}

/**
 * Command callback.
 * Delete a variable.
 */
function brush_config_delete() {
  $args = func_get_args();
  if (!isset($args[0])) {
    brush_set_error('BRUSH_config_ERROR', dt('No variable specified'));
  }
  // Look for similar variable names.
  $result = brush_config_like($args[0]);

  $options = array();
  while ($name = brush_db_result($result)) {
    $options[] = $name;
  }

  if (count($options) == 0) {
    brush_print(dt('!name not found.', array('!name' => $args[0])));
    return '';
  }

  if ((count($options) == 1) && brush_get_context('BRUSH_AFFIRMATIVE')) {
    brush_op('variable_del', $args[0]);
    brush_log(dt('!name was deleted.', array('!name' => $args[0])), 'success');
    return '';
  }
  else {
    $choice = brush_choice($options, 'Enter a number to choose which variable to delete.');
    if ($choice !== FALSE) {
      $choice = $options[$choice];
      brush_op('variable_del', $choice);
      brush_log(dt('!choice was deleted.', array('!choice' => $choice)), 'success');
    }
  }
}

// Query for similar variable names.
function brush_config_like($arg) {
  return brush_db_select('variable', 'name', 'name LIKE :keyword', array(':keyword' => $arg . '%'), NULL, NULL, 'name');
}
