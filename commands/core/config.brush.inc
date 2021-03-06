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
      'brush clist' => 'List all configuration files.',
      'brush clist user' => 'List all configuration files containing the string "user" in their names.',
    ),
    'options' => array(
      'pipe' => 'Use var_export() to emit executable PHP. Useful for pasting into code.',
    ),
    'aliases' => array('clist', 'vlist'),
  );
  $items['config-get'] = array(
    'description' => 'Get a list of some or all site variables and values.',
    'arguments' => array(
      'config-name' => 'The configuration object name, for example "system.core".',
      'key' => 'A string to filter a single configuration key by, for example "clean_url".',
      '*key*' => 'A wildcard string to filter multiple configuration keys by.',
    ),
    'examples' => array(
      'brush cget' => 'List all variables and values.',
      'brush cget system.core' => 'List all variables in "system.core.json" file.',
      'brush cget system.core clean_url' => 'Display value of "clean_url" key in "system.core.json" file.',
      'brush cget entity_type' => 'List values of the "entity_type" key in all configuration files.',
      'brush cget *entity*' => 'List variables whose keys contain string "entity".',
    ),
    'options' => array(
      'pipe' => 'Use var_export() to emit executable PHP. Useful for pasting into code.',
    ),
    'aliases' => array('cget','vget'),
  );
  $items['config-set'] = array(
    'description' => "Set a configuration key.",
    'arguments' => array(
      'config-name' => 'The configuration object name, for example "system.core".',
      'key' => 'The config key, for example "site_slogan".',
      'value' => 'The value to assign to the config key. Arrays can be input as JSON strings.',
    ),
    'options' => array(
      'yes' => 'Skip confirmation if only one variable name matches.',
      'always-set' => 'Always skip confirmation.',
    ),
    'examples' => array(
      'brush cset --yes preprocess_css 1' => 'Set the preprocess_css variable to true.',
      'brush cset system.core watchdog_enabled_severity_levels \'{"0":0,"1":1}\'' => 'Set severity of logs to Emergency & Alert. See /admin/config/development/logging.',
    ),
    'aliases' => array('cset','vset'),
  );
  $items['config-clear'] = array(
    'description' => "Delete a single value from a config file.",
    'arguments' => array(
      'config-name' => 'The configuration object name, for example "system.core".',
      'key' => 'A configuration key to clear, for example "site_frontpage".',
    ),
    'options' => array(
      'yes' => 'Skip confirmation if only one variable name matches.',
    ),
    'examples' => array(
      'brush cdel system.core site_frontpage' => 'Delete the site_frontpage variable.',
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
    if (!empty($args[0])) {
      if (strpos($config_name, $args[0]) !== false) {
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
    brush_set_error('No configuration found for '.$args[0]);
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

  if (!$args[0]) {
    foreach ($config_storage->listAll() as $config_name) {
      if (config_get_info($config_name)['label']) {
        $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
      }
      else {
        $title = $config_name.'.json';
      }
      $config = $config_storage->read($config_name);
      foreach ($config as $key => $value) {
        if (is_array($value)) {
          $value = json_encode($value);
        }
        $rows[] = array(
          $key,
          $value,
        );
      }
      brush_print();
      brush_print($title);
      brush_print_table($rows, TRUE);
      unset($rows);
    }
    $printed = TRUE;
  }


  foreach ($config_storage->listAll() as $config_name) {
    $config = $config_storage->read($config_name);
    if ($args[0]) {
      if (strpos($args[0], '.') !== FALSE) { // each $config_name contains dot.
        // Clean up the incorrectly entered $config_name
        if (substr($args[0], -5) == '.json') {
          $args[0] = substr($args[0], 0, -5);
        }
        if ($args[0] == $config_name) {
          if (config_get_info($config_name)['label']) {
            $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
          }
          else {
            $title = $config_name.'.json';
          }
          foreach ($config as $key => $value) {
            if ($args[1]) {

              if ($key == $args[1]) {
                if (is_array($value)) {
                  $value = json_encode($value);
                }
                $rows[] = array(
                  $key,
                  $value,
                );
              }
            }
            else {
              if (is_array($value)) {
                $value = json_encode($value);
              }
              $rows[] = array(
                $key,
                $value,
              );
            }
          }
          if (empty($rows)) {
            return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $args[1])));
          }
          else {
            $printed = FALSE;
          }
        }
      }
       // $args[0] without dot is probably $key.
       elseif (array_key_exists($args[0], $config)) {
        $rows = array();
        // The first row serves as table header.
        $rows[] = array(
          'Configuration key',
          'Value',
        );
        if (config_get_info($config_name)['label']) {
          $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
        }
        else {
          $title = $config_name.'.json';
        }
        foreach ($config as $key => $value) {
          if ($key == $args[0]) {
            if (is_array($value)) {
              $value = json_encode($value);
            }
            $rows[] = array(
              $key,
              $value,
            );
            brush_print();
            brush_print($title);
            brush_print_table($rows, TRUE);
            $printed = TRUE;
          }
        }
      }
      // Wildcard argument.
      elseif($args[0][0] == '*' && substr($args[0],-1) == '*') {
        $search = str_replace( '\*', '.*?', preg_quote($args[0], '/' ));
        $results = preg_grep( '/^' . $search . '$/i', array_keys($config) );
        if (!empty($results)) {
          $rows = array();
          // The first row serves as table header.
          $rows[] = array(
            'Configuration key',
            'Value',
          );
          if (config_get_info($config_name)['label']) {
            $title = config_get_info($config_name)['label'].' ('.$config_name.'.json)';
          }
          else {
            $title = $config_name.'.json';
          }
          foreach ($results as $result) {
            foreach ($config as $key => $value) {
              if ($key == $result) {
                if (is_array($value)) {
                  $value = json_encode($value);
                }
                $rows[] = array(
                  $key,
                  $value,
                );
              }
            }
          }
          brush_print();
          brush_print($title);
          brush_print_table($rows, TRUE);
          $printed = TRUE;
        }
      }
    }
  }
  if (isset($printed)) {
    if ($printed == FALSE) {
      brush_print();
      brush_print($title);
      $header = array(
        'Configuration key',
        'Value',
      );
      array_unshift($rows, $header);
      brush_print_table($rows, TRUE);
    }
  }
  else {
    brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $args[0])));
  }
}


/**
 * Command callback.
 * Set a variable.
 */

function brush_config_set() {
  $args = func_get_args();
  $config_name = $args[0];
  if (!isset($config_name)) {
    return brush_set_error('BRUSH_config_ERROR', bt('No configuration name specified.'));
  }
  elseif (strpos($config_name, '.') == FALSE) {
    return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $config_name)));
  }
  $key = $args[1];
  if (!isset($key)) {
    return brush_set_error('BRUSH_config_ERROR', bt('No configuration key specified.'));
  }
  $value = $args[2];
  if (!isset($value)) {
    return brush_set_error('BRUSH_config_ERROR', bt('No configuration value specified.'));
  }

  $config_dir = config_get_config_directory();
  $config_storage = new ConfigFileStorage($config_dir);
  // Clean up the incorrectly entered $config_name
  if (substr($config_name, -5) == '.json') {
    $config_name = substr($config_name, 0, -5);
  }
  if ($config = $config_storage->read($config_name)) {
    if (isset($config[$key])) {
      if (is_array($config[$key])) {
        $value = json_decode($value, true);
        if (json_last_error() !== 0) {
          return brush_set_error('BRUSH_config_ERROR', bt('To set array values use correct JSON string format. For example, \'{"0":0,"1":1,"2":2,"3":3}\''));
        }
      }
      $choice = brush_choice(array('1' => 'Yes'), bt('Are you sure you want to update "!name"?', array('!name' => $key)));
      if ($choice == '1') {
        config_set($config_name, $key, $value);
        brush_print(bt('Configuration key "!name" has been succesfully updated.', array('!name' => $key)));
      }
    }
    // Key not found.
    else {
      $choice = brush_choice(array('1' => 'Yes'), bt('Are you sure you want to create a new config "!name"?', array('!name' => $key)));
      if ($choice == '1') {
        config_set($config_name, $key, $value);
        brush_print(bt('Configuration key !name has been succesfully created.', array('!name' => $key)));
      }
    }
  }
  else {
    return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $config_name)));
  }
}


/**
 * Command callback.
 * Delete a variable.
 */
 function brush_config_clear() {
   $args = func_get_args();
   $config_name = $args[0];
   if (!isset($config_name)) {
     return brush_set_error('BRUSH_config_ERROR', bt('No configuration name specified.'));
   }
   elseif (strpos($config_name, '.') == FALSE) {
     return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $config_name)));
   }
   $key = $args[1];
   if (!isset($key)) {
     return brush_set_error('BRUSH_config_ERROR', bt('No configuration key specified.'));
   }

   $config_dir = config_get_config_directory();
   $config_storage = new ConfigFileStorage($config_dir);
   // Clean up the incorrectly entered $config_name
   if (substr($config_name, -5) == '.json') {
     $config_name = substr($config_name, 0, -5);
   }
   if ($config = $config_storage->read($config_name)) {
     if (isset($config[$key])) {
       $choice = brush_choice(array('1' => 'Yes'), bt('Are you sure you want to clear "!name"?', array('!name' => $key)));
       if ($choice == '1') {
         config_clear($config_name, $key);
         brush_print(bt('Configuration key "!name" has been succesfully cleared.', array('!name' => $key)));
       }
     }
     else {
       return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $key)));
     }
   }
   else {
     return brush_set_error('BRUSH_config_ERROR', bt('No configuration found for !name', array('!name' => $config_name)));
   }
 }
