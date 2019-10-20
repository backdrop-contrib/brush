<?php

/*
 * @file
 *   Tests for user.drop.inc
 */
class userCase extends Drop_TestCase {

  /*
   * Create, edit, block, and cancel users.
   */
  public function testUser() {
    // user-create
    $env = 'dev';
    $this->setUpBackdrop($env, TRUE);
    $root = $this->sites[$env]['root'];
    $name = "example";
    $options = array(
      'root' => $root,
      'uri' => $env,
      'yes' => NULL,
    );
    $this->drop('user-create', array($name), $options + array('password' => 'password', 'mail' => "example@example.com"));
    $this->drop('user-information', array($name), $options + array('pipe' => NULL));
    $output = $this->getOutput();
    $row  = str_getcsv($output);
    $uid = $row[1];
    $this->assertEquals('example@example.com', $row[2]);
    $this->assertEquals($name, $row[0]);
    $this->assertEquals(1, $row[3], 'Newly created user is Active.');
    $this->assertEquals('authenticated user', $row[4], 'Newly created user has one role.');

    // user-block
    $this->drop('user-block', array($name), $options);
    $this->drop('user-information', array($name), $options + array('pipe' => NULL));
    $output = $this->getOutput();
    $row  = str_getcsv($output);
    $this->assertEquals(0, $row[3], 'User is blocked.');

    // user-unblock
    $this->drop('user-unblock', array($name), $options);
    $this->drop('user-information', array($name), $options + array('pipe' => NULL));
    $output = $this->getOutput();
    $row  = str_getcsv($output);
    $this->assertEquals(1, $row[3], 'User is unblocked.');

    // user-add-role
    // first, create the fole since we use testing install profile.
    $eval = "user_role_save((object)array('name' => 'administrator'))";
    $this->drop('php-eval', array($eval), $options);
    $this->drop('user-add-role', array('administrator', $name), $options);
    $this->drop('user-information', array($name), $options + array('pipe' => NULL));
    $output = $this->getOutput();
    $row  = str_getcsv($output);
    $this->assertEquals('authenticated user,administrator', $row[4], 'User has administrator role.');

    // user-remove-role
    $this->drop('user-remove-role', array('administrator', $name), $options);
    $this->drop('user-information', array($name), $options + array('pipe' => NULL));
    $output = $this->getOutput();
    $row  = str_getcsv($output);
    $this->assertEquals('authenticated user', $row[4], 'User removed administrator role.');

    // user-password
    $newpass = 'newpass';
    $this->drop('user-password', array($name), $options + array('password' => $newpass));
    $eval = "require_once BACKDROP_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');";
    $eval .= "\$account = user_load_by_name('example');";
    $eval .= "print (string) user_check_password('$newpass', \$account)";
    $this->drop('php-eval', array($eval), $options);
    $output = $this->getOutput();
    $this->assertEquals('1', $output, 'User can login with new password.');

    // user-login
    $this->drop('user-login', array($name), $options);
    $output = $this->getOutput();
    $url = parse_url($output);
    $this->assertStringStartsWith('/user/reset/' . $uid, $url['path'], 'Login returned a valid reset URL');

    // user-cancel
    // create content
    $eval = $this->create_node_types_php();
    $this->drop('php-eval', array($eval), $options);
    $eval = "
      \$node = (object) array(
        'title' => 'foo',
        'uid' => 2,
        'type' => 'page',
      );
      node_save(\$node);
    ";
    $this->drop('php-eval', array($eval), $options);
    $this->drop('user-cancel', array($name), $options + array('delete-content' => NULL));
    $eval = 'print (string) user_load(2)';
    $this->drop('php-eval', array($eval), $options);
    $output = $this->getOutput();
    $this->assertEmpty($output, 'User was deleted');
    $eval = 'print (string) node_load(2)';
    $this->drop('php-eval', array($eval), $options);
    $output = $this->getOutput();
    $this->assertEmpty($output, 'Content was deleted');
  }
}
