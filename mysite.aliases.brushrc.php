<?php
/**
 * @file mysite.aliases.brushrc.php
 * Site aliases for [your site domain]
 * Place this file at ~/.brush/  (~/ means your home path)
 *
 * Usage:
 *   To copy the development database to your local site:
 *   $ brush sql-sync @mysite.local @mysite.prod
 *   To copy your local database to the development site:
 *   $ brush sql-sync @mysite.local @mysite.dev --structure-tables-key=common --no-ordered-dump --sanitize=0 --no-cache
 *   To copy the production database to your local site:
 *   $ brush sql-sync @mysite.prod @mysite.local
 *   To copy all files in development site to your local site:
 *   $ brush rsync @mysite.dev:%files @mysite.local:%files
 *   Clear the cache in production:
 *   $ brush @mysite.prod clear-cache all
 *
 * You can copy the site alias configuration of an existing site into a file
 * with the following commands:
 *   $ cd /path/to/settings.php/of/the/site/
 *   $ brush site-alias @self --full --with-optional >> ~/.drush/mysite.aliases.brushrc.php
 * Then edit that file to wrap the code in < ? php ? > tags.
 */
 
/**
 * Local alias
 * Set the root and site_path values to point to your local site
 */
 
$aliases['mysite.local'] = array (
  'root' => '/var/www/docroot',
  'uri' => 'my.docksal',
  'path-aliases' => 
  array (
    '%drush' => '/usr/local/bin',
    '%site' => 'sites/default/',
  ),
);
 
$aliases['mysite.prod'] = array (
  'uri' => 'my.altagrade.com',
  'root' => '/home/altacom/domains/my.altagrade.com/public_html',
  'remote-user' => 'altacom',
  'remote-host' => '66.160.206.201',
  'ssh-options'  => '-p 19753',  // To change the default port on remote server
  'path-aliases' => array(
    '%dump-dir' => '/tmp',
  ),
  'source-command-specific' => array (
    'sql-sync' => array (
      'no-cache' => TRUE,
      'structure-tables-key' => 'common',
    ),
  ),
  // No need to modify the following settings
  'command-specific' => array (
    'sql-sync' => array (
      'sanitize' => TRUE,
      'no-ordered-dump' => TRUE,
      'structure-tables' => array(
       // You can add more tables which contain data to be ignored by the database dump
        'common' => array('cache', 'cache_filter', 'cache_menu', 'cache_page', 'history', 'sessions', 'watchdog'),
      ),
    ),
  ),
);
?>
