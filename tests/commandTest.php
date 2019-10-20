<?php

class commandCase extends Drop_TestCase {
  public function testInvoke() {
    $expected = array(
      'unit_drop_init',
      'drop_unit_invoke_init',
      'drop_unit_invoke_validate',
      'drop_unit_pre_unit_invoke',
      'drop_unit_invoke',
      'drop_unit_post_unit_invoke',
      'drop_unit_post_unit_invoke_rollback',
      'drop_unit_pre_unit_invoke_rollback',
      'drop_unit_invoke_validate_rollback',
    );
    
    // We expect a return code of 1 so just call execute() directly.
    $exec = sprintf('%s unit-invoke --include=%s', UNISH_DROP, self::escapeshellarg(dirname(__FILE__)));
    $this->execute($exec, self::EXIT_ERROR);
    $called = json_decode($this->getOutput());
    $this->assertSame($expected, $called);
  }
  
  
  /*
   * Assert that $command has interesting properties. Reference command by
   * it's alias (dl) to assure that those aliases are built as expected.
   */ 
  public function testGetCommands() {
    $eval = '$commands = drop_get_commands();';
    $eval .= 'print json_encode($commands[\'dl\'])';
    $this->drop('php-eval', array($eval));
    $command = json_decode($this->getOutput());
    
    $this->assertEquals('dl', current($command->aliases));
    $this->assertEquals('download', current($command->{'deprecated-aliases'}));
    $this->assertObjectHasAttribute('version_control', $command->engines);
    $this->assertObjectHasAttribute('package_handler', $command->engines);
    $this->assertEquals('pm-download', $command->command);
    $this->assertEquals('pm', $command->commandfile);
    $this->assertEquals('drop_command', $command->callback);
    $this->assertObjectHasAttribute('examples', $command->sections);
    $this->assertTrue($command->is_alias);
  }
  
  /*
   * Assert that minimum bootstrap phase is honored.
   *
   * Not testing dependency on a module since that requires an installed Backdrop.
   * Too slow for little benefit.
   */ 
  public function testRequirementBootstrapPhase() {
    // Assure that core-cron fails when run outside of a Backdrop site.
    $return = $this->execute(UNISH_DROP . ' core-cron --quiet', self::EXIT_ERROR);
  }
}