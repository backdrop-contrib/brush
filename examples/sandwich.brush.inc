<?php

/**
 * @file
 *   Example brush command.
 *
 *   To run this *fun* command, execute `sudo brush --include=./examples mmas`
 *   from within your brush directory.
 *
 *   See `brush topic docs-commands` for more information about command authoring. 
 *
 *   You can copy this file to any of the following
 *     1. A .brush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/brush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 */

/**
 * Implementation of hook_brush_command().
 *
 * In this hook, you specify which commands your
 * brush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * See `brush topic docs-commands` for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function sandwich_brush_command() {
  $items = array();

  // The 'make-me-a-sandwich' command
  $items['make-me-a-sandwich'] = array(
    'description' => "Makes a delicious sandwich.",
    'arguments' => array(
      'filling' => 'The type of the sandwich (turkey, cheese, etc.)',
    ),
    'options' => array(
      'spreads' => 'Comma delimited list of spreads (e.g. mayonnaise, mustard)',
    ),
    'examples' => array(
      'brush mmas turkey --spreads=ketchup,mustard' => 'Make a terrible-tasting sandwich that is lacking in pickles.',
    ),
    'aliases' => array('mmas'),
    'bootstrap' => BRUSH_BOOTSTRAP_DROP, // No bootstrap at all.
  );
  
  // Commandfiles may also add topics.  These will appear in
  // the list of topics when `brush topic` is executed.
  // To view this topic, run `brush --include=/full/path/to/examples topic`
  $items['sandwich-exposition'] = array(
    'description' => 'Ruminations on the true meaning and philosophy of sandwiches.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_DROP,
    'callback' => 'brush_print_file',
    'callback arguments' => array(dirname(__FILE__) . '/sandwich-topic.txt'),
  );

  return $items;
}

/**
 * Implementation of hook_brush_help().
 *
 * This function is called whenever a brush user calls
 * 'brush help <name-of-your-command>'. This hook is optional. If a command
 * does not implement this hook, the command's description is used instead.
 *
 * This hook is also used to look up help metadata, such as help
 * category title and summary.  See the comments below for a description.
 *
 * @param
 *   A string with the help section (prepend with 'brush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function sandwich_brush_help($section) {
  switch ($section) {
    case 'brush:make-me-a-sandwich':
      return dt("This command will make you a delicious sandwich, just how you like it.");
    // The 'title' meta item is used to name a group of
    // commands in `brush help`.  If a title is not defined,
    // the default is "All commands in ___", with the
    // specific name of the commandfile (e.g. sandwich).
    // Command files with less than four commands will 
    // be placed in the "Other commands" section, _unless_
    // they define a title.  It is therefore preferable
    // to not define a title unless the file defines a lot
    // of commands.
    case 'meta:sandwich:title':
      return dt("Sandwich commands");
    // The 'summary' meta item is displayed in `brush help --filter`,
    // and is used to give a general idea what the commands in this
    // command file do, and what they have in common.
    case 'meta:sandwich:summary':
      return dt("Automates your sandwich-making business workflows.");
  }
}

/**
 * Implementation of brush_hook_COMMAND_validate().
 *
 * The validate command should exit with
 * `return brush_set_error(...)` to stop execution of
 * the command.  In practice, calling brush_set_error
 * OR returning FALSE is sufficient.  See brush.api.php
 * for more details.
 */
function brush_sandwich_make_me_a_sandwich_validate() {
  $name = posix_getpwuid(posix_geteuid());
  if ($name['name'] !== 'root') {
    return brush_set_error('MAKE_IT_YOUSELF', dt('What? Make your own sandwich.'));
  }
}

/**
 * Example brush command callback. This is where the action takes place.
 *
 * The function name should be same as command name but with dashes turned to
 * underscores and 'brush_commandfile_' prepended, where 'commandfile' is
 * taken from the file 'commandfile.brush.inc', which in this case is 'sandwich'.
 * Note also that a simplification step is also done in instances where
 * the commandfile name is the same as the beginning of the command name,
 * "brush_example_example_foo" is simplified to just "brush_example_foo".
 * To also implement a hook that is called before your command, implement
 * "brush_hook_pre_example_foo".  For a list of all available hooks for a
 * given command, run brush in --debug mode.
 *
 * If for some reason you do not want your hook function to be named
 * after your command, you may define a 'callback' item in your command
 * object that specifies the exact name of the function that should be
 * called.  However, the specified callback function must still begin
 * with "brush_commandfile_" (e.g. 'callback' => "brush_example_foo_execute")
 * if you want that all hook functions are still called (e.g.
 * brush_example_pre_foo_execute, and so on).
 *
 * In this function, all of Backdrop's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * @see brush_invoke()
 * @see brush.api.php
 */
function brush_sandwich_make_me_a_sandwich($filling = 'ascii') {
  $str_spreads = '';
  if ($spreads = brush_get_option('spreads')) {
    $list = implode(' and ', explode(',', $spreads));
    $str_spreads = ' with just a dash of ' . $list;
  }
  $msg = dt('Okay. Enjoy this !filling sandwich!str_spreads.',
            array('!filling' => $filling, '!str_spreads' => $str_spreads)
         );
  brush_print("\n" . $msg . "\n");
  brush_print(file_get_contents(dirname(__FILE__) . '/sandwich.txt'));
}
