<?php

/*
 * @file
 *   Tests watchdog-show and watchdog-delete commands
 */
class WatchdogCase extends Brush_TestCase {

  function testWatchdog() {
    $env = 'dev';
    $sites = $this->setUpBackdrop($env, TRUE);
    $root = $this->sites[$env]['root'];
    $options = array(
      'yes' => NULL,
      'root' => $root,
      'uri' => $env,
    );

    // Enable dblog module and verify that the watchdog messages are listed
    $this->brush('pm-enable', array('dblog'), $options);
    $this->brush('watchdog-show', array(), $options);
    $output = $this->getOutput();
    $this->assertContains('dblog module installed.', $output);
    $this->assertContains('dblog module enabled.', $output);

    // Add a new entry with a long message with the letter 'd' and verify that watchdog-show does
    // not print it completely in the listing unless --full is given.
    // As the output is formatted so lines may be splitted, assertContains does not work
    // in this scenario. Therefore, we will count the number of times a character is present.
    $message_chars = 300;
    $char = '*';
    $message = str_repeat($char, $message_chars);
    $this->brush('php-eval', array("watchdog('brush', '" . $message . "')"), $options);
    $this->brush('watchdog-show', array(), $options);
    $output = $this->getOutput();
    $this->assertGreaterThan(substr_count($output, $char), $message_chars);
    $this->brush('watchdog-show', array(), $options + array('full' => NULL));
    $output = $this->getOutput();
    $this->assertGreaterThanOrEqual($message_chars, substr_count($output, $char));

    // Tests message deletion
    $this->brush('watchdog-delete', array('all'), $options);
    $output = $this->getOutput();
    $this->brush('watchdog-show', array(), $options);
    $output = $this->getOutput();
    $this->assertEmpty($output);
  }
}
