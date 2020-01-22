<?php

/**
 * Install Backdrop 1+
 */
function brush_core_site_install_version($profile, array $additional_form_options = array()) {
  if (is_null($profile)) {
    $profile = 'standard';
  }

  define('MAINTENANCE_MODE', 'install');
  require_once BACKDROP_ROOT . '/core/includes/install.core.inc';

  $db_spec = _brush_sql_get_db_spec();

  $account_pass = brush_get_option('account-pass', 'admin');
  $settings = array(
    'parameters' => array(
      'profile' => $profile,
      'locale' => brush_get_option('locale', 'en'),
    ),
    'forms' => array(
      'install_settings_form' => array(
        'driver' => $db_spec['driver'],
        $db_spec['driver'] => $db_spec,
        'op' => dt('Save and continue'),
      ),
      'install_configure_form' => array(
        'site_name' => brush_get_option('site-name', 'Site-Install'),
        'site_mail' => brush_get_option('site-mail', 'admin@example.com'),
        'account' => array(
          'name' => brush_get_option('account-name', 'admin'),
          'mail' => brush_get_option('account-mail', 'admin@example.com'),
          'pass' => array(
            'pass1' => $account_pass,
            'pass2' => $account_pass,
          ),
        ),
        'update_status_module' => array(
          1 => TRUE,
          2 => TRUE,
        ),
        'clean_url' => brush_get_option('clean-url', TRUE),
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

  brush_log(dt('Starting Backdrop installation. This takes a few seconds ...'), 'ok');
  install_backdrop($settings);
}
