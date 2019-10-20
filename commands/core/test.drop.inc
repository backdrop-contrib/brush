<?php

/**
 * @file
 *   Simpletest module drop integration.
 */

/**
 * Implementation of hook_drop_command().
 */
function test_drop_command() {
  $items = array();

  $items['test-run'] = array(
    'description' => "Run tests. Note that you must use the --uri option.",
    'arguments' => array(
      'targets' => 'A test class, a test group. If omitted, a list of test classes and test groups is presented. Delimit multiple targets using commas.',
    ),
    'examples' => array(
      'test-run' => 'List all available classes and groups.',
      'sudo -u apache test-run --all' => 'Run all available tests. Avoid permission related failures by running as web server user.',
      'test-run XMLRPCBasicTestCase' => 'Run one test class.',
      'test-run XML-RPC' => 'Run all classes in a XML-RPC group.',
      'test-run XML-RPC,Filter' => 'Run all tests from multiple groups/classes.',
      'test-run XMLRPCBasicTestCase --methods="testListMethods, testInvalidMessageParsing"' => 'Run particular methods in the specified class or group.',
    ),
    'options' => array(
      'all' => 'Run all available tests',
      'methods' => 'A comma delimited list of methods that should be run within the test class. Defaults to all methods.',
      'dirty' => 'Skip cleanup of temporary tables and files. Helpful for reading debug() messages and other post-mortem forensics.',
      'xml' => 'Output verbose test results to a specified directory using the JUnit test reporting format. Useful for integrating with Jenkins.'

    ),
    'backdrop dependencies' => array('simpletest'),
    // If you DROP_BOOTSTRAP_BACKDROP_LOGIN, you fall victim to http://backdrop.org/node/974768. We'd like
    // to not bootstrap at all but simpletest uses Backdrop to discover test classes,
    // cache the lists of tests, file_prepare_directory(), variable lookup like
    // httpauth creds, copy pre-built registry table from testing side, etc.
    'bootstrap' => DROP_BOOTSTRAP_BACKDROP_FULL,
  );
  $items['test-clean'] = array(
    'description' => "Clean temporary tables and files.",
    'backdrop dependencies' => array('simpletest'),
  );

  return $items;
}

// Command callback
function drop_test_clean() {
  return simpletest_clean_environment();
}

// Validate hook
function drop_test_run_validate($specs = NULL) {
  if (!drop_get_option('uri')) {
    // No longer needed?
    // return drop_set_error(dt("You must specify this site's URL using the --uri parameter."));
  }
}

/**
 * Test-run command callback.
 *
 * @specs
 *   A comman delimited string of test classes or group names.
 */
function drop_test_run($specs = NULL) {
  cache_clear_all('simpletest', 'cache');

  // Retrieve all tests and groups.
  list($groups, $all_tests) = drop_test_get_all_tests();

  if (drop_get_option('all')) {
    // Run all tests.
    foreach (array_keys($groups) as $group) {
      foreach (array_keys($groups[$group]) as $class) {
        drop_backend_invoke_args('test-run', array($class), array('--no-all'));
      }
    }
    return;
  }
  elseif (empty($specs)) {
    return drop_test_list($groups);
  }

  foreach (explode(',', $specs) as $spec) {
    $spec = trim($spec);
    // Specific test class specified.
    if (in_array($spec, $all_tests)) {
      simpletest_drop_run_test($spec);
      if (!drop_get_option('dirty')) {
        simpletest_clean_environment();
      }
    }
    // Specific group specified.
    else if (isset($groups[$spec])) {
      foreach (array_keys($groups[$spec]) as $class) {
        drop_do_command_redispatch('test-run', array($class));
      }
    }
  }
}

/**
 * Run a single test and display any failure messages.
 */
function simpletest_drop_run_test($class) {

  $test_id = db_insert('simpletest_test_id')
  ->useDefaults(array('test_id'))
  ->execute();

  $test = new $class($test_id);

  if ($methods_string = drop_get_option('methods')) {
    foreach (explode(',', $methods_string) as $method) {
      $methods[] = trim($method);
    }
    $test->run($methods);
  }
  else {
    $test->run();
  }

  $info = $test->getInfo();
  $status = ((isset($test->results['#fail']) && $test->results['#fail'] > 0)
          || (isset($test->results['#exception']) && $test->results['#exception'] > 0) ? 'error' : 'ok');
  drop_log($info['name'] . ' ' . _simpletest_format_summary_line($test->results), $status);

  if ($dir = drop_get_option('xml')) {
    drop_test_xml_results($test_id, $dir);
  }

  // If there were some failed tests show them.
  if ($status === 'error') {
    $args = array(':test_id' => $test_id);
    $result = db_query("SELECT * FROM {simpletest} WHERE test_id = :test_id AND status IN ('exception', 'fail') ORDER BY test_class, message_id", $args);
    foreach($result as $record) {
      drop_set_error('DROP_TEST_FAIL', dt("Test !function failed: !message", array('!function' => $record->function, '!message' => $record->message)));
    }
  }
}

/**
 * Retrieve all test groups and sanitize their names to make them command-line
 * friendly.
 */
function simpletest_drop_test_groups($tests) {
  $groups = array();
  foreach (simpletest_categorize_tests($tests) as $name => $group) {
    $sanitized = strtr($name, array(' ' => ''));
    $groups[$sanitized] = $group;
  }
  return $groups;
}

// Print a listing of all available tests
function drop_test_list($groups) {
  $rows[] = array(dt('Command'), dt('Description'));
  $rows[] = array('-------', '-----------');
  foreach ($groups as $group_name => $group_tests) {
    foreach ($group_tests as $test_class => $test_info) {
      if (!isset($rows[$test_info['group']])) {
        $rows[$test_info['group']] = array($group_name, $test_info['group']);
      }
      $rows[] = array("  {$test_class}", "  {$test_info['name']}");
    }
  }
  return drop_print_table($rows, TRUE);
}

function drop_test_get_all_tests() {
  if (function_exists('simpletest_get_all_tests')) {
    $all_tests = simpletest_get_all_tests();
    $groups = simpletest_drop_test_groups($all_tests);
  }
  else {
    $groups = simpletest_test_get_all();
    $all_tests = array();
    foreach ($groups as $group) {
      $all_tests = array_merge($all_tests, array_keys($group));
    }
  }
  return array($groups, $all_tests);
}

/*
 * Display test results.
 */
function drop_test_xml_results($test_id, $dir) {
  $dir = is_string($dir) ? $dir : '.';

  $results_map = array(
    'pass' => 'Pass',
    'fail' => 'Fail',
    'exception' => 'Exception',
  );

  $results = db_query("SELECT * FROM {simpletest} WHERE test_id = :test_id ORDER BY test_class, message_id", array(':test_id' => $test_id));

  $test_class = '';
  $xml_files = array();

  foreach ($results as $result) {
    if (isset($results_map[$result->status])) {
      if ($result->test_class != $test_class) {
        // Display test class every time results are for new test class.
        if (isset($xml_files[$test_class])) {
          file_put_contents($dir . '/' . $test_class . '.xml', $xml_files[$test_class]['doc']->saveXML());
          unset($xml_files[$test_class]);
        }
        $test_class = $result->test_class;
        if (!isset($xml_files[$test_class])) {
          $doc = new DomDocument('1.0');
          $root = $doc->createElement('testsuite');
          $root = $doc->appendChild($root);
          $xml_files[$test_class] = array('doc' => $doc, 'suite' => $root);
        }
      }
      // Save the result into the XML:
      $case = $xml_files[$test_class]['doc']->createElement('testcase');
      $case->setAttribute('classname', $test_class);
      list($class, $name) = explode('->', $result->function, 2);
      $case->setAttribute('name', $name);

      if ($result->status == 'fail') {
        $fail = $xml_files[$test_class]['doc']->createElement('failure');
        $fail->setAttribute('type', 'failure');
        $fail->setAttribute('message', $result->message_group);
        $text = $xml_files[$test_class]['doc']->createTextNode($result->message);
        $fail->appendChild($text);
        $case->appendChild($fail);
      }

      $xml_files[$test_class]['suite']->appendChild($case);
    }
  }

  // Save the last one:
  if (isset($xml_files[$test_class])) {
    file_put_contents($dir . '/' . $test_class . '.xml', $xml_files[$test_class]['doc']->saveXML());
    unset($xml_files[$test_class]);
  }
}
