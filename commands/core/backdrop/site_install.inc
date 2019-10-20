<?php

/**
 * Install Backdrop 1+
 */
function drop_core_site_install_version($profile, array $additional_form_options = array()) {
  if (is_null($profile)) {
    $profile = 'standard';
  }

  define('MAINTENANCE_MODE', 'install');
  require_once BACKDROP_ROOT . '/core/includes/install.core.inc';

  $db_spec = _drop_sql_get_db_spec();

  $account_pass = drop_get_option('account-pass', 'admin');
  $settings = array(
    'parameters' => array(
      'profile' => $profile,
      'locale' => drop_get_option('locale', 'en'),
    ),
    'forms' => array(
      'install_settings_form' => array(
        'driver' => $db_spec['driver'],
        $db_spec['driver'] => $db_spec,
        'op' => dt('Save and continue'),
      ),
      'install_configure_form' => array(
        'site_name' => drop_get_option('site-name', 'Site-Install'),
        'site_mail' => drop_get_option('site-mail', 'admin@example.com'),
        'account' => array(
          'name' => drop_get_option('account-name', 'admin'),
          'mail' => drop_get_option('account-mail', 'admin@example.com'),
          'pass' => array(
            'pass1' => $account_pass,
            'pass2' => $account_pass,
          ),
        ),
        'update_status_module' => array(
          1 => TRUE,
          2 => TRUE,
        ),
        'clean_url' => drop_get_option('clean-url', TRUE),
        'op' => dt('Save and continue'),
      ),
    ),
  );

  // Merge in the additional options.
  foreach ($additional_form_options as $key => $value) {
    $current = &$settings['forms'];
    foreach (explode('.', $key) as $param) {
      $current = &$current[$param];
    }
    $current = $value;
  }

  drop_log(dt('Starting Backdrop installation. This takes a few seconds ...'), 'ok');
  install_backdrop($settings);
}
