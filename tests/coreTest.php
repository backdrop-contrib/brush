<?php

/*
 * @file
 *   Tests for core commands.
 */
class coreCase extends Brush_TestCase {

  /*
   * Test standalone php-script scripts. Assure that script args and options work.
   */
  public function testStandaloneScript() {
    $this->brush('version', array('brush_version'), array('pipe' => NULL));
    $standard = $this->getOutput();

    // Write out a hellounish.script into the sandbox. The correct /path/to/brush
    // is in the shebang line.
    $filename = 'hellounish.script';
    $data = '#!/usr/bin/env [PATH-TO-BRUSH]

$arg = brush_shift();
brush_invoke("version", $arg);
';
    $data = str_replace('[PATH-TO-BRUSH]', UNISH_BRUSH, $data);
    $script = UNISH_SANDBOX . '/' . $filename;
    file_put_contents($script, $data);
    chmod($script, 0755);
    $this->execute("$script brush_version --pipe");
    $standalone = $this->getOutput();
    $this->assertEquals($standard, $standalone);
  }

  function testBackdropDirectory() {
    $this->setUpBackdrop('dev', TRUE);
    $root = $this->sites['dev']['root'];
    $options = array(
      'root' => $root,
      'uri' => 'dev',
      'verbose' => NULL,
      'yes' => NULL,
    );
    $this->brush('pm-download', array('devel-1.x-1.0'), $options);
    $this->brush('pm-enable', array('menu', 'devel'), $options);

    $this->brush('backdrop-directory', array('devel'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/all/modules/devel', $output);

    $this->brush('backdrop-directory', array('%files'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/dev/files', $output);

    $this->brush('backdrop-directory', array('%modules'), $options);
    $output = $this->getOutput();
    $this->assertEquals($root . '/sites/all/modules', $output);
  }

  function testCoreCLI() {
    /*
     * @todo
     *   - BASHRC_PATH. Same file cleanup woes as contextTest.
     *   - BRUSH_CLI
     *   - INITIAL_SITE
     *   - PS1. Hard to test in non interactive session?
     *   - on
     *   - use
     *   - cd, cdd, lsd
     *   - override, contextual
     */

    // Exercise core-cli's interactive mode.
    // Include unit.brush.inc commandfile.
    $options = array(
      'include' => dirname(__FILE__),
    );
    // These commands will throw a failure if they return non-zero exit code.
    // Assure that we create a bash function for command names.
    $options['unit-extra'] = 'core-status;exit';
    $this->brush('core-cli', array(), $options);
    // Assure that we create a bash function for command aliases.
    $options['unit-extra'] = 'st;exit';
    $this->brush('core-cli', array(), $options);

    // Assure that we create a bash alias for site aliases.
    // First, write an alias file to the sandbox.
    $path = UNISH_SANDBOX . '/aliases.brushrc.php';
    $aliases['cliAlias'] = array(
      'root' => $this->sites['dev']['root'],
      'uri' => 'dev',
    );
    $contents = $this->file_aliases($aliases);
    $return = file_put_contents($path, $contents);
    // Append a bash command which starts with alias name (i.e. @cliAlias).
    $options['unit-extra'] = sprintf('@cliAlias core-status --alias-path=%s;exit', UNISH_SANDBOX);
    $options['alias-path'] = UNISH_SANDBOX;
    $this->brush('core-cli', array(), $options);

    // $this->markTestIncomplete('In progress below.');
    // Exercise core-cli's non-interactive mode.
    // We spawn our own bash session using the --pipe feature of core-cli.
    //$options = array(
    //  'pipe' => NULL,
    //  'alias-path' => UNISH_SANDBOX,
    //);
    //$this->brush('core-cli', array(), $options);
    //$bashrc_data = $this->getOutput();
    //$bashrc_file = UNISH_SANDBOX . '/.bashrc';
    //$extra = 'cd @cliAlias;exit;';
    //$return = file_put_contents($bashrc_file, $bashrc_data . $extra);
    //$this->setUpBackdrop('dev', FALSE);
    //$this->execute('bash --rcfile ' . $bashrc_file);
    //$output = $this->getOutput();
    //$this->assertContains('????', $output);
  }
}
