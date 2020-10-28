<?php

/**
 * Implementation of hook_brush_command().
 */
function cache_brush_command() {
  $items = array();

  $items['cache-get'] = array(
    'description' => 'Fetch a cached object and display it.',
    'examples' => array(
      'brush cache-get schema' => 'Display the data for the cache id "schema" from the "cache" bin.',
      'brush cache-get update_available_releases update' => 'Display the data for the cache id "update_available_releases" from the "update" bin.',
    ),
    'arguments' => array(
      'cid' => 'The id of the object to fetch.',
      'bin' => 'Optional. The cache bin to fetch from.',
    ),
    'options' => array(
      'format' => 'Format to output the object. Use "print_r" for print_r (default), "export" for var_export, and "json" for JSON.',
    ),
    'aliases' => array('cg'),
  );
  $items['cache-clear'] = array(
    'description' => 'Clear a specific cache, or all backdrop caches.',
    'arguments' => array(
      'type' => 'The particular cache to clear. Omit this argument to choose from available caches.',
    ),
    'aliases' => array('cc'),
  );
  $items['cache-set'] = array(
    'description' => 'Cache an object expressed in JSON or var_export() format.',
    'arguments' => array(
      'cid' => 'The id of the object to set.',
      'data' => 'The object to set in the cache. Use \'-\' to read the object from STDIN.',
      'bin' => 'Optional. The cache bin to store the object in.',
      'expire' => 'Optional. CACHE_PERMANENT, CACHE_TEMPORARY, or a Unix timestamp.',
    ),
    'options' => array(
      'format' => 'Format to parse the object. Use "string" for string (default), and "json" for JSON.',
      'cache-get' => 'If the object is the result a previous fetch from the cache, only store the value in the "data" property of the object in the cache.',
    ),
    'aliases' => array('cs'),
  );

  return $items;
}

/**
 * Command callback for brush cache-clear.
 */
function brush_cache_clear($type = NULL) {
  switch (brush_backdrop_major_version()) {
    case 5:
      // clear preprocessor cache
      backdrop_clear_css_cache();

      // clear core tables
      $core = array('cache', 'cache_filter', 'cache_menu', 'cache_page');
      $alltables = array_merge($core, module_invoke_all('devel_caches'));
      foreach ($alltables as $table) {
        cache_clear_all('*', $table, TRUE);
      }
      brush_print(bt('Cache cleared.'));
      break;
    case 6:
    case 7:
    default:
      $types = brush_cache_clear_types();
      // Check if the provided type ($type) is a valid cache type.
      if ($type && !key_exists($type, $types)) {
        return brush_set_error(bt("'!type' cache is not a valid cache type", array('!type' => $type)));
      }

      if ($type) {
        brush_op($types[$type]);
        brush_log(bt("'!name' cache was cleared", array('!name' => $type)), 'success');
      }
      else {
        $choice = brush_choice($types, 'Enter a number to choose which cache to clear.', '!key');
        if ($choice !== FALSE) {
          call_user_func($types[$choice]);
          brush_log(bt("'!name' cache was cleared", array('!name' => $choice)), 'success');
        }
      }
      break;
  }
}

/**
 * Print an object returned from the cache.
 *
 * @param $cid
 *   The cache ID of the object to fetch.
 * @param $bin
 *   Optional parameter to specify a specific bin to fetch from.
 */
function brush_cache_get($cid = NULL, $bin = NULL) {
  if (!$cid) {
    brush_log(bt('You must specify a cache id to fetch.'), 'error');
    return;
  }

  if (!$bin) {
    $bin = 'cache';
  }

  $result = cache_get($cid, $bin);
  if (!empty($result)) {
    switch (brush_get_option('format', 'print_r')) {
      case 'export':
        brush_print(var_export($result, TRUE));
        return;
      case 'json':
        brush_print(brush_json_encode($result));
        return;
      case 'print_r':
      default:
        brush_print_r($result);
        return;
    }
  }
  else {
    brush_log(bt('The !cid object in the !bin cache bin was not found.', array('!cid' => $cid, '!bin' => $bin)), 'error');
  }
}

/**
 * Set an object in the cache.
 *
 * @param $cid
 *   The cache ID of the object to fetch.
 * @param $data
 *   The data to save to the cache, or '-' to read from STDIN.
 * @param $bin
 *   Optional parameter to specify a specific bin to fetch from.
 * @param $expire
 *   Optional parameter to specify the expiry of the cached object.
 */
function brush_cache_set($cid = NULL, $data = '', $bin = NULL, $expire = CACHE_PERMANENT) {
  if (!$cid) {
    brush_log(bt('You must specify a cache id to set.'), 'error');
    return;
  }

  if ($data === '') {
    brush_log(bt('The data to set must be non-empty.'), 'error');
    return;
  }

  if (!$bin) {
    $bin = 'cache';
  }

  if ($data == '-') {
    $data = stream_get_contents(STDIN);
  }

  // Now, we parse the object.
  switch (brush_get_option('format', 'string')) {
    case 'json':
      $data = brush_json_decode($data);
      break;
  }

  if (brush_get_option('cache-get')) {
    $data = $data->data;
  }

  switch(brush_backdrop_major_version()) {
    case 5:
      cache_set($cid, $bin, $data, $expire);
      break;
    case 6:
    case 7:
    default:
      cache_set($cid, $data, $bin, $expire);
  }
}

function brush_cache_clear_types() {
  $types = array(
    'all' => 'backdrop_flush_all_caches',
    'theme registry' => 'brush_cache_clear_theme_registry',
    'menu' => 'menu_rebuild',
    'css+js' => 'brush_cache_clear_css_js',
    'block' => 'brush_cache_clear_block',
    'module list' => 'brush_get_modules',
    'theme list' => 'brush_get_themes',
  );

  $types['registry'] = 'registry_update';
  
  if (count(module_implements('node_grants'))) {
    $types['nodeaccess'] = 'node_access_rebuild';
  }

  // Include the appropriate environment engine, so callbacks can use core
  // version specific cache clearing functions directly.
  brush_include_engine('backdrop', 'environment');

  // Command files may customize $types as desired.
  brush_command_invoke_all_ref('brush_cache_clear', $types);
  return $types;
}

function brush_cache_clear_theme_registry() {
  brush_db_delete('cache', 'cid LIKE :theme_registry', array(':theme_registry' => 'theme_registry%'));
}

function brush_cache_clear_css_js() {
  _backdrop_flush_css_js();
  backdrop_clear_css_cache();
  backdrop_clear_js_cache();
}

/**
 * Clear the cache of the block output.
 */
function brush_cache_clear_block() {
  cache_clear_all(NULL, 'cache_block');
}
