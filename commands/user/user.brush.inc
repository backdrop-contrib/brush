<?php
// $Id:

/**
 * @file Brush User Management commands
 */

function user_brush_help($section) {
  switch ($section) {
    case 'meta:user:title':
      return dt('User commands');
    case 'meta:user:summary':
      return dt('Add, modify and delete users.');
  }
}

/**
 * Implementation of hook_brush_command().
 */
function user_brush_command() {
  $items['user-information'] = array(
    'callback' => 'brush_user_information',
    'description' => 'Print information about the specified user(s).',
    'aliases' => array('uinf'),
    'examples' => array(
      'brush user-information 2,3,someguy,somegal,billgates@microsoft.com' =>
        'Display information about any users with uids, names, or mail addresses matching the strings between commas.',
    ),
    'arguments' => array(
      'users' => 'A comma delimited list of uids, user names, or email addresses.',
    ),
    'options' => array(
      'full' => 'show extended information about the user',
      'short' => 'show basic information about the user (this is the default)',
    ),
  );
  $items['user-block'] = array(
    'callback' => 'brush_user_block',
    'description' => 'Block the specified user(s).',
    'aliases' => array('ublk'),
    'arguments' => array(
      'users' => 'A comma delimited list of uids, user names, or email addresses.',
    ),
    'examples' => array(
      'brush user-block 5,user3 --uid=2,3 --name=someguy,somegal --mail=billgates@microsoft.com' =>
        'Block the users with name, id, or email 5 or user3, uids 2 and 3, names someguy and somegal, and email address of billgates@microsoft.com',
    ),
    'options' => array(
      'uid' => 'A comma delimited list of uids to block',
      'name' => 'A comma delimited list of user names to block',
      'mail' => 'A comma delimited list of user mail addresses to block',
    ),
  );
  $items['user-unblock'] = array(
    'callback' => 'brush_user_unblock',
    'description' => 'Unblock the specified user(s).',
    'aliases' => array('uublk'),
    'arguments' => array(
      'users' => 'A comma delimited list of uids, user names, or email addresses.',
    ),
    'examples' => array(
      'brush user-unblock 5,user3 --uid=2,3 --name=someguy,somegal --mail=billgates@microsoft.com' =>
        'Unblock the users with name, id, or email 5 or user3, uids 2 and 3, names someguy and somegal, and email address of billgates@microsoft.com',
    ),
    'options' => array(
      'uid' => 'A comma delimited list of uids to unblock',
      'name' => 'A comma delimited list of user names to unblock',
      'mail' => 'A comma delimited list of user mail addresses to unblock',
    ),
  );
  $items['user-add-role'] = array(
    'callback' => 'brush_user_add_role',
    'description' => 'Add a role to the specified user accounts.',
    'aliases' => array('urol'),
    'arguments' => array(
      'role' => 'The name of the role to add',
      'users' => '(optional) A comma delimited list of uids, user names, or email addresses.',
    ),
    'examples' => array(
      'brush user-add-role "power user" 5,user3 --uid=2,3 --name=someguy,somegal --mail=billgates@microsoft.com' =>
        'Add the "power user" role to the accounts with name, id, or email 5 or user3, uids 2 and 3, names someguy and somegal, and email address of billgates@microsoft.com',
    ),
    'options' => array(
      'uid' => 'A comma delimited list of uids',
      'name' => 'A comma delimited list of user names',
      'mail' => 'A comma delimited list of user mail addresses',
    ),
  );
  $items['user-remove-role'] = array(
    'callback' => 'brush_user_remove_role',
    'description' => 'Remove a role from the specified user accounts.',
    'aliases' => array('urrol'),
    'arguments' => array(
      'role' => 'The name of the role to remove',
      'users' => '(optional) A comma delimited list of uids, user names, or email addresses.',
    ),
    'examples' => array(
      'brush user-remove-role "power user" 5,user3 --uid=2,3 --name=someguy,somegal --mail=billgates@microsoft.com' =>
        'Remove the "power user" role from the accounts with name, id, or email 5 or user3, uids 2 and 3, names someguy and somegal, and email address of billgates@microsoft.com',
    ),
    'options' => array(
      'uid' => 'A comma delimited list of uids',
      'name' => 'A comma delimited list of user names',
      'mail' => 'A comma delimited list of user mail addresses',
    ),
  );
  $items['user-create'] = array(
    'callback' => 'brush_user_create',
    'description' => 'Create a user account with the specified name.',
    'aliases' => array('ucrt'),
    'arguments' => array(
      'name' => 'The name of the account to add'
    ),
    'examples' => array(
      'brush user-create newuser --mail="person@example.com" --password="letmein"' =>
        'Create a new user account with the name newuser, the email address person@example.com, and the password letmein',
    ),
    'options' => array(
      'password' => 'The password for the new account',
      'mail' => 'The email address for the new account',
    ),
  );
  $items['user-cancel'] = array(
    'callback' => 'brush_user_cancel',
    'description' => 'Cancel a user account with the specified name.',
    'aliases' => array('ucan'),
    'arguments' => array(
      'name' => 'The name of the account to cancel',
    ),
    'examples' => array(
      'brush user-cancel username' =>
        'Cancel the user account with the name username and anonymize all content created by that user.',
    ),
  );
  $items['user-password'] = array(
    'callback' => 'brush_user_password',
    'description' => '(Re)Set the password for the user account with the specified name.',
    'aliases' => array('upwd'),
    'arguments' => array(
      'name' => 'The name of the account to modify'
    ),
    'options' => array(
      'password' => '(required) The new password for the account',
    ),
    'examples' => array(
      'brush user-password someuser --password="gr3@tP@$s"' =>
        'Set the password for the username someuser to gr3@tP@$s.',
    ),
  );
  $items['user-login'] = array(
    'callback' => 'brush_user_login',
    'description' => 'Display a one time login link for the given user account (defaults to uid 1).',
    'aliases' => array('uli'),
    'arguments' => array(
      'name' => 'The name of the account to log in as. Leave it empty to log in as uid 1.'
    ),
    'examples' => array(
      'brush user-login ryan' => 'Displays a one-time login link for the user ryan.',
      'open `brush user-login ryan`' => 'Open web browser and login as user ryan.',
    ),
  );

  $items['user-cancel']['options'] = array(
    'delete-content' => 'Delete all content created by the user',
  );
  $items['user-cancel']['examples']['brush user-cancel --delete-content username'] =
    'Cancel the user account with the name username and delete all content created by that user.';

  return $items;
}

/**
 * Prints information about the specified user(s).
 */
function brush_user_information($users = '0') {
  $uids = _brush_user_get_users_from_arguments($users);
  foreach($uids as $uid) {
    _brush_user_print_info($uid);
  }
}

/**
 * Block the specified user(s).
 */
function brush_user_block($users = '') {
  $uids = _brush_user_get_users_from_options_and_arguments($users);
  if (!empty($uids)) {
    brush_op('user_user_operations_block', $uids);
  }
  else {
    return brush_set_error("Could not find any valid uids!");
  }
}

/**
 * Unblock the specified user(s).
 */
function brush_user_unblock($users = '') {
  $uids = _brush_user_get_users_from_options_and_arguments($users);
  if (!empty($uids)) {
    brush_op('user_user_operations_unblock', $uids);
  }
  else {
    return brush_set_error("Could not find any valid uids!");
  }
}

/**
 * Add a role to the specified user accounts.
 */
function brush_user_add_role($role, $users = '') {
  $uids = _brush_user_get_users_from_options_and_arguments($users);
  $rid_query = db_query("SELECT rid FROM {role} WHERE name = :role", array(':role' => $role));

  if (!empty($uids)) {
    if ($rid = brush_db_result($rid_query)) {
      brush_op('user_multiple_role_edit', $uids, 'add_role', $rid);
      foreach($uids as $uid) {
        brush_log(dt("Added the %role role to uid %uid", array('%role' => $role, '%uid' => $uid)), 'success');
      }
    }
    else {
      return brush_set_error("There is no role named: \"$role\"!");
    }
  }
  else {
    return brush_set_error("Could not find any valid uids!");
  }
}

/**
 * Remove a role from the specified user accounts.
 */
function brush_user_remove_role($role, $users = '') {
  $uids = _brush_user_get_users_from_options_and_arguments($users);
  $rid_query = db_query("SELECT rid FROM {role} WHERE name = :role", array(':role' => $role));

  if (!empty($uids)) {
    if ($rid = brush_db_result($rid_query)) {
      brush_op('user_multiple_role_edit', $uids, 'remove_role', $rid);
      foreach($uids as $uid) {
        brush_log(dt("Removed the %role role from uid %uid", array('%role' => $role, '%uid' => $uid)), 'success');
      }
    }
    else {
      return brush_set_error("There is no role named: \"$role\"!");
    }
  }
  else {
    return brush_set_error("Could not find any valid uids!");
  }
}

/**
 * Creates a new user account.
 */
function brush_user_create($name) {
  $mail = brush_get_option('mail');
  $pass = brush_get_option('password');
  $new_user = array(
    'name' => $name,
    'pass' => $pass,
    'mail' => $mail,
    'access' => '0',
    'status' => 1,
  );

  $result = db_query("SELECT uid FROM {users} WHERE name = :name OR mail = :mail", array(':name' => $name, ':mail' => $new_user['mail']));

  if (brush_db_result($result) === FALSE) {
    if (!brush_get_context('BRUSH_SIMULATE')) {
      $new_user_object = user_save(NULL, $new_user, NULL);
      if ($new_user_object !== FALSE) {
        _brush_user_print_info($new_user_object->uid);
        return $new_user_object->uid;
      }
      else {
        brush_set_error("Could not create a new user account with the name " . $name . "!");
      }
    }
  }
  else {
    brush_set_error("There is already a user account with the name " . $name . " or email address " . $new_user['mail'] . "!");
  }
}

/**
 * Cancels a user account.
 */
function brush_user_cancel($name) {
  $result = db_query("SELECT uid FROM {users} WHERE name = :name", array(':name' => $name));
  $uid = brush_db_result($result);
  if ($uid !== FALSE) {
    brush_print("Cancelling the user account with the following information:");
    _brush_user_print_info($uid);
    if (brush_get_option('delete-content')) {
      brush_print("All content created by this user will be deleted!");
    }
    if (brush_confirm('Cancel user account?: ')) {
      if (brush_get_option('delete-content')) {
        user_cancel(array(), $uid, 'user_cancel_delete');
      }
      else {
        user_cancel(array(), $uid, 'user_cancel_reassign');
      }
      // I got the following technique here: http://drupal.org/node/638712
      $batch =& batch_get();
      $batch['progressive'] = FALSE;
      batch_process();
    }
  }
  else {
    brush_set_error("Could not find a user account with the name " . $name . "!");
  }
}

/**
 * Sets the password for the account with the given username
 */
function brush_user_password($name = 1) {

  $pass = brush_get_option('password');
  if (empty($pass)) {
    return brush_set_error("You must specify a password. E.g.: 'brush user-password someuser --password=\"gr3@tP@$s\"'");
  }

  if (is_numeric($name)) {
    $user = user_load($name);
  }
  else {
    $user = user_load_by_name($name);
  }

  if ($user !== FALSE) {
    if (!brush_get_context('BRUSH_SIMULATE')) {
      $user_object = user_save($user, array('pass' => $pass));
      if ($user_object === FALSE) {
        brush_set_error("Could not change the password for the user account with the name " . $name . "!");
      }
    }
  }
  else {
    brush_set_error("The user account with the name " . $name . " could not be loaded!");
  }
}

/**
 * Displays a one time login link for the given user.
 */
function brush_user_login($name = NULL) {

  if (empty($name)) {
    $user = user_load(1);
  }
  elseif (is_numeric($name)) {
    $user = user_load($name);
  }
  else {
    $user = user_load_by_name($name);
  }

  if ($user !== FALSE && $user->status) {
    $link = user_pass_reset_url($user);
    brush_print($link);
    return $link;
  }
  else {
    brush_set_error("The user account with the name " . $name . " could not be loaded or is blocked!");
  }
}

/**
 * Print information about a given uid
 */
function _brush_user_print_info($uid) {

  $userinfo = user_load($uid);

  if (brush_get_option('full')) {
    $userinfo = (array)$userinfo;
    $userinfo_pipe = array();
    unset($userinfo['data']);
    unset($userinfo['block']);
    unset($userinfo['form_build_id']);
    foreach($userinfo as $key => $val) {
      if (is_array($val)) {
        brush_print($key . ': ');
        brush_print_r($val);
        $userinfo_pipe[] = '"' . implode(",", $val) . '"';
      }
      else {
        if ($key === 'created' OR $key === 'access' OR $key === 'login') {
          brush_print($key . ': ' . format_date($val));
          $userinfo_pipe[] = $val;
        }
        else {
          brush_print($key . ': ' . $val);
          $userinfo_pipe[] = $val;
        }
      }
    }
    brush_print_pipe(implode(",", $userinfo_pipe));
    brush_print_pipe("\n");
  }
  else {
    $userinfo_short = array(
      'User ID' => $userinfo->uid,
      'User name' => $userinfo->name,
      'User mail' => $userinfo->mail,
    );
    $userinfo_short['User roles'] = implode(', ', $userinfo->roles);
    $userinfo->status ? $userinfo_short['User status'] = 'active' : $userinfo_short['User status'] = 'blocked';
    brush_print_table(brush_key_value_to_array_table($userinfo_short));
    brush_print_pipe("$userinfo->name,$userinfo->uid,$userinfo->mail,$userinfo->status,\"" . implode(',', $userinfo->roles) . "\"\n");
  }
}

/**
 * Given a comma-separated list of users, return uids
 * for users that match either by uid or email address.
 */
function _brush_user_get_users_from_arguments($users) {
  $uids = array();
  if ($users !== '') {
    $users = explode(',', $users);
    foreach($users as $user) {
      $uid = _brush_user_get_uid($user);
      if ($uid !== FALSE) {
        $uids[] = $uid;
      }
    }
  }
  return $uids;
}

/**
 * Return the list of matching uids given
 */
function _brush_user_get_users_from_options_and_arguments($users) {
  $uids = brush_get_option_list('uids');

  foreach (array('uid', 'name', 'mail' ) as $user_attr) {
    if ($arg = brush_get_option($user_attr)) {
      foreach(explode(',', $arg) as $search) {
        $uid_query = FALSE;
        switch ($user_attr) {
          case 'uid':
            $uid_query = db_query("SELECT uid FROM {users} WHERE uid = :uid", array(':uid' => $search));
            break;
          case 'name':
            $uid_query = db_query("SELECT uid FROM {users} WHERE name = :name", array(':name' => $search));
            break;
          case 'mail':
            $uid_query = db_query("SELECT uid FROM {users} WHERE mail = :mail", array(':mail' => $search));
            break;
        }
        if ($uid_query !== FALSE) {
          if ($uid = brush_db_result($uid_query)) {
            $uids[] = $uid;
          }
          else {
            brush_set_error("Could not find a uid for $user_attr = $search");
          }
        }
      }
    }
  }

  return array_merge($uids, _brush_user_get_users_from_arguments($users));
}

/**
 * Get uid(s) from a uid, user name, or email address.
 * Returns a uid, or FALSE if none found.
 */
function _brush_user_get_uid($search) {
  // We use a DB query while looking for the uid to keep things speedy.
  $uids = array();
  if (is_numeric($search)) {
    $uid_query = db_query("SELECT uid, name FROM {users} WHERE uid = :uid OR name = :name", array(':uid' => $search, ':name' => $search));
  }
  else {
    $uid_query = db_query("SELECT uid, name FROM {users} WHERE mail = :mail OR name = :name", array(':mail' => $search, ':name' => $search));
  }
  while ($uid = brush_db_fetch_object($uid_query)) {
    $uids[$uid->uid] = $uid->name;
  }
  switch (count($uids)) {
    case 0:
      return brush_set_error("Could not find a uid for the search term '" . $search . "'!");
      break;
    case 1:
      return array_pop(array_keys($uids));
      break;
    default:
      brush_print('More than one user account was found for the search string "' . $search . '".');
      return(brush_choice($uids, 'Please choose a name:', '!value (uid=!key)'));
  }
}
