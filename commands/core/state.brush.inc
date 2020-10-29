<?php

/**
 * Implementation of hook_brush_command().
 *
 *
 * @return
 *   An associative array describing your command(s).
 */
function state_brush_command() {
  $items['state-get'] = array(
    'description' => 'Get a list of some or all state keys and values.',
    'arguments' => array(
      'name' => 'A string to filter a single state key by.',
      '*name*' => 'A string to filter multiple state keys by.',
    ),
    'examples' => array(
      'brush sget' => 'List all variables and values.',
      'brush sget *cron_last*' => 'List all state keys with "cron_last" in their names and their values',
      'brush sget cron_last' => 'List the "cron_last" state key and its value.',
    ),
    'options' => array(
      'pipe' => 'Use var_export() to emit executable PHP. Useful for pasting into code.',
    ),
    'aliases' => array('sget'),
  );
  $items['state-set'] = array(
    'description' => "Set a state key.",
    'arguments' => array(
      'name' => 'The name of a state key to set.',
      'value' => 'The value to assign to the state key.',
    ),
    'options' => array(
      'yes' => 'Skip confirmation if state key found.',
      'always-set' => 'Always skip confirmation.',
    ),
    'examples' => array(
      'brush sset --yes maintenance_mode 1' => 'Set the maintenance_mode state key to true by skipping confirmation.',
    ),
    'aliases' => array('sset'),
  );
  $items['state-delete'] = array(
    'description' => "Delete a state key.",
    'arguments' => array(
      'name' => 'The name of a state key.',
    ),
    'options' => array(
      'yes' => 'Skip confirmation when deleting a state key.',
    ),
    'examples' => array(
      'brush sdel maintenance_mode' => 'Delete the maintenance_mode state key.',
    ),
    'aliases' => array('sdel', 'state_del'),
  );

  return $items;
}

/**
 * Command callback.
 * List your site's state keys.
 */
function brush_state_get() {
  $args = func_get_args();
  $states = &backdrop_static('states');
  $rows[] = array('State key', 'Value');

  // No argument provided.
  if (!$args[0]) {
    foreach ($states as $key => $value) {
      if (is_array($value)) {
        $value = json_encode($value);
      }
      $rows[] = array($key, $value);
    }
    brush_print_table($rows, TRUE);
  }
  // Wildcard argument.
  elseif($args[0][0] == '*' && substr($args[0],-1) == '*') {
    $arg = str_replace( '\*', '', preg_quote($args[0], '/' ));
    $keys = array_keys($states);
    $keys = preg_grep("/{$arg}/", $keys);
    if (empty($keys)) {
      return brush_set_error('BRUSH_config_ERROR', bt('No state key found for !name', array('!name' => $arg)));
    }
    foreach ($keys as $key) {
      $rows[] = array($key, $states[$key]);
    }
    brush_print_table($rows, TRUE);
  }
  // Normal argument.
  else {
    if (!empty($states[$args[0]])) {
      $rows[] = array($args[0], $states[$args[0]]);
      brush_print_table($rows, TRUE);
    }
    else {
      brush_set_error('BRUSH_config_ERROR', bt('No state key found for !name', array('!name' => $args[0])));
    }
  }
}

/**
 * Command callback.
 * Set a state key.
 */
function brush_state_set() {
  $args = func_get_args();
  if (!isset($args[0])) {
    return brush_set_error('BRUSH_VARIABLE_ERROR', bt('No state key specified.'));
  }
  $value = $args[1];
  if (!isset($value)) {
    return brush_set_error('BRUSH_VARIABLE_ERROR', bt('No state key value specified.'));
  }
  $states = &backdrop_static('states');
  if (isset($states[$args[0]])) {
    state_set($args[0], $args[1]);
    brush_print(bt('State key "!name" has been succesfully updated.', array('!name' => $args[0])));
  }
  // Key not found.
  else {
    $choice = brush_choice(array('1' => 'Yes'), bt('State "!name" not found. Do you want to create a new key for it?', array('!name' => $args[0])));
    if ($choice == '1') {
      state_set($args[0], $args[1]);
      brush_print(bt('State key !name has been succesfully created.', array('!name' => $args[0])));
    }
  }
}

/**
 * Command callback.
 * Delete a state key.
 */
function brush_state_delete() {
  $args = func_get_args();
  if (!isset($args[0])) {
    brush_set_error('BRUSH_VARIABLE_ERROR', bt('No state key specified'));
  }
  $states = &backdrop_static('states');
  if (isset($states[$args[0]])) {
    $choice = brush_choice(array('1' => 'Yes'), bt('Are you sure you want to delete the "!name" state key?', array('!name' => $args[0])));
    if ($choice == '1') {
      state_del($args[0]);
      brush_print(bt('The !name state key has been succesfully deleted.', array('!name' => $args[0])));
    }
  }
  else {
    brush_set_error('BRUSH_config_ERROR', bt('No state key found for !name', array('!name' => $args[0])));
  }
}
