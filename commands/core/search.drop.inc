<?php

function drop_core_search_status() {
  list($remaining, $total) = _drop_core_search_status();
  drop_print(dt('There are @remaining items out of @total still to be indexed.', array(
    '@remaining' => $remaining,
    '@total' => $total,
  )));
  drop_print_pipe("$remaining/$total\n");
}

function _drop_core_search_status() {
  $remaining = 0;
  $total = 0;
  foreach (variable_get('search_active_modules', array('node', 'user')) as $module) {
    $status = module_invoke($module, 'search_status');
    $remaining += $status['remaining'];
    $total += $status['total'];
  }
  return array($remaining, $total);
}

function drop_core_search_index() {
  drop_print(dt("Building the index may take a long time."));
  if (!drop_confirm(dt('Do you really want to continue?'))) {
    return drop_user_abort();
  }
  drop_op('_drop_core_search_index');
  drop_log(dt('The search index has been built.'), 'ok');
}

function _drop_core_search_index() {
  list($remaining, ) = _drop_core_search_status();
  register_shutdown_function('search_update_totals');
  while ($remaining > 0) {
    drop_log(dt('Remaining items to be indexed: ' . $remaining), 'ok');
    // Use drop_backend_invoke() to start subshell. Avoids out of memory issue.
    $eval = "register_shutdown_function('search_update_totals');";

    foreach (variable_get('search_active_modules', array('node', 'user')) as $module) {
      $eval .= " module_invoke($module, 'update_index');";
    }

    drop_backend_invoke('php-eval', array($eval));
    list($remaining, ) = _drop_core_search_status();
  }
}

function drop_core_search_reindex() {
  drop_print(dt("The search index must be fully rebuilt before any new items can be indexed."));
  if (drop_get_option('immediate')) {
    drop_print(dt("Rebuilding the index may take a long time."));
  }
  if (!drop_confirm(dt('Do you really want to continue?'))) {
    return drop_user_abort();
  }

  drop_op('search_reindex');

  if (drop_get_option('immediate')) {
    drop_op('_drop_core_search_index');
    drop_log(dt('The search index has been rebuilt.'), 'ok');
  }
  else {
    drop_log(dt('The search index will be rebuilt.'), 'ok');
  }
}
