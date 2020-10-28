<?php

/**
 * @file
 *   Documentation commands providing various topics.
 */

/**
 * Implementation of hook_brush_help().
 */
function docs_brush_help($section) {
  switch ($section) {
    case 'meta:docs:title':
      return bt('Documentation commands');
    case 'meta:docs:summary':
      return bt('Show information on various brush topics.');
  }
}

/**
 * Implementation of hook_brush_command().
 *
 * @return
 *   An associative array describing your command(s).
 */
function docs_brush_command() {
  $docs_dir = brush_get_context('DOC_PREFIX', BRUSH_BASE_PATH);

  //
  // Topic commands.
  // Any commandfile may add topics.
  // Set 'topic' => TRUE to indicate the command is a topic (REQUIRED)
  // Begin the topic name with the name of the commandfile (just like
  // any other command).
  //
  $items['docs-readme'] = array(
    'description' => 'README.txt',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/README.txt'),
  );
  $items['docs-configuration'] = array(
    'description' => 'Brush configuration overview with examples',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/example.brushrc.php'),
  );
  $items['docs-aliases'] = array(
    'description' => 'Site aliases overview with examples',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/example.aliases.brushrc.php'),
  );
  $items['docs-ini-files'] = array(
    'description' => 'Configuring php.ini or brush.ini for use with brush.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/example.brush.ini'),
  );
  $items['docs-bastion'] = array(
    'description' => 'How to use a bastion server to remotely operate on Backdrop sites behind a firewall.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/bastion.html'),
  );
  $items['docs-bootstrap'] = array(
    'description' => 'Information about the brush bootstrap process.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/bootstrap.html'),
  );
  $items['docs-scripts'] = array(
    'description' => 'Overview on how to write brush shell scripts.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/shellscripts.html'),
  );
  $items['docs-commands'] = array(
    'description' => 'Overview on how to write brush commands.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/commands.html'),
  );
  $items['docs-errorcodes'] = array(
    'description' => 'Summary of brush error codes.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
  );
  $items['docs-api'] = array(
    'description' => 'Brush API',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/brush.api.php'),
  );
  $items['docs-context'] = array(
    'description' => 'Brush contexts',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/context.html'),
  );
  $items['docs-examplescript'] = array(
    'description' => 'Example brush script',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/helloworld.script'),
  );
  $items['docs-examplecommand'] = array(
    'description' => 'Example brush command file.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/sandwich.brush.inc'),
  );
  $items['docs-policy'] = array(
    'description' => 'Example policy file.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/examples/policy.brush.inc'),
  );
  $items['docs-upgrading'] = array(
    'description' => 'Upgrading Backdrop using the brush site-upgrade command.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => BRUSH_BOOTSTRAP_BRUSH,
    'callback' => 'brush_print_file',
    'callback arguments' => array($docs_dir . '/docs/upgrade.html'),
  );
  return $items;
}

/**
 * docs-error-codes command.  Print a list of all error codes
 * that can be found.
 */
function brush_docs_errorcodes() {
  $header = <<<EOD
==== Brush Error Codes ====

Brush error codes are alphanumeric constants that represent an unrecoverable error condition that may arise during the execution of some command.  They are set by the following function:

  return brush_set_error('BRUSH_ERROR_CODE', bt('Error message.'));

In general, any brush command that calls brush_set_error is expected to also return FALSE as its function result.  The brush_set_error function returns FALSE to make it easy to exit with an error code.  Error codes are returned as part of the brush backend invoke process, which is used by brush API functions such as brush_invoke_process.  An example of how to test for a specific error code is shown below:

  \$result = brush_invoke_process('some-command');
  if (array_key_exists('BRUSH_ERROR_CODE', \$result['error_log'])) {
    // handle ocurrances of BRUSH_ERROR_CODE here
  }

Some of the available brush error codes are listed in the table below.


EOD;

  // Find all of the files that we will search for error messages.
  // Start with all of the commandfiles.
  $commandfiles = brush_commandfile_list();
  $files = array_flip($commandfiles);
  // In addition to the commandfiles, we will also look for files
  // that brush will load when executing a command; examples include
  // updatecode.pm.inc and sync.sql.inc.
  $commands = brush_get_commands();
  foreach ($commands as $command_name => $command) {
    $files = array_merge($files, brush_command_get_includes($command_name));
  }
  // We will also search through all of the .inc files in the
  // brush includes directory
  $brush_include_files = brush_scan_directory(BRUSH_BASE_PATH . '/includes', '/.*\.inc$/', array('.', '..', 'CVS'), 0, FALSE);
  foreach ($brush_include_files as $filename => $info) {
    $files[$filename] = 'include';
  }

  // Extract error messages from all command files
  $error_list = array();
  foreach ($files as $file => $commandfile) {
    _brush_docs_find_set_error_calls($error_list, $file, $commandfile);
  }
  // Order error messages alphabetically by key
  ksort($error_list);
  // Convert to a table
  $data = array();
  foreach ($error_list as $error_code => $error_messages) {
    $data[] = array($error_code, '-', implode("\n", $error_messages));
  }

  $tmpfile = brush_tempnam('brush-errorcodes.');
  file_put_contents($tmpfile, $header);
  brush_print_table($data, FALSE, array(0 => 35), $tmpfile);
  brush_print_file($tmpfile);
}

/**
 * Search through a php source file looking for calls to
 * the function brush_set_error.  If found, and if the
 * first parameter is an uppercase alphanumeric identifier,
 * then record the error code and the error message in our table.
 */
function _brush_docs_find_set_error_calls(&$error_list, $filename, $shortname) {
  $lines = file($filename);
  foreach ($lines as $line) {
    $matches = array();
    // Find the error code after the brush_set_error call.  The error code
    // should consist of uppercase letters and underscores only (numbers thrown in just in case)
    $match_result = preg_match("/.*brush_set_error[^'\"]['\"]([A-Z0-9_]*)['\"][^,]*,[^'\"]*(['\"])/", $line, $matches);
    if ($match_result) {
      $error_code = $matches[1];
      $quote_char = $matches[2];
      $error_message = "";
      $message_start = strlen($matches[0]) - 1;

      // Regex adapted from http://stackoverflow.com/questions/1824325/regex-expression-for-escaped-quoted-string-wont-work-in-phps-preg-match-allif ($quote_char == '"') {
      if ($quote_char == '"') {
	$regex = '/"((?:[^\\\]*?(?:\\\")?)*?)"/';
      }
      else {
	$regex = "/'((?:[^\\\]*?(?:\\\')?)*?)'/";
      }
      $match_result = preg_match($regex, $line, $matches, 0, $message_start);

      if ($match_result) {
	$error_message = $matches[1];
      }
      $error_list[$error_code] = array_key_exists($error_code, $error_list) ? array_merge($error_list[$error_code], array($error_message)) : array($error_message);
    }
  }
}
