<?php

function brush_core_search_status() {
  list($remaining, $total) = _brush_core_search_status();
  brush_print(bt('There are @remaining items out of @total still to be indexed.', array(
    '@remaining' => $remaining,
    '@total' => $total,
  )));
  brush_print_pipe("$remaining/$total\n");
}

function _brush_core_search_status() {
  $remaining = 0;
  $total = 0;
  foreach (config_get('search.settings', 'search_active_modules') as $module) {
    $status = module_invoke($module, 'search_status');
    $remaining += $status['remaining'];
    $total += $status['total'];
  }
  return array($remaining, $total);
}

function brush_core_search_index() {
  brush_print(bt("Building the index may take a long time."));
  if (!brush_confirm(bt('Do you really want to continue?'))) {
    return brush_user_abort();
  }
  brush_op('_brush_core_search_index');
  brush_log(bt('The search index has been built.'), 'ok');
}

function _brush_core_search_index() {
  list($remaining, ) = _brush_core_search_status();
  register_shutdown_function('search_update_totals');
  while ($remaining > 0) {
    brush_log(bt('Remaining items to be indexed: ' . $remaining), 'ok');
    // Use brush_backend_invoke() to start subshell. Avoids out of memory issue.
    $eval = "register_shutdown_function('search_update_totals');";

    foreach (config_get('search.settings', 'search_active_modules') as $module) {
      $eval .= " module_invoke($module, 'update_index');";
    }

    brush_backend_invoke('php-eval', array($eval));
    list($remaining, ) = _brush_core_search_status();
  }
}

function brush_core_search_reindex() {
  brush_print(bt("The search index must be fully rebuilt before any new items can be indexed."));
  if (brush_get_option('immediate')) {
    brush_print(bt("Rebuilding the index may take a long time."));
  }
  if (!brush_confirm(bt('Do you really want to continue?'))) {
    return brush_user_abort();
  }

  brush_op('search_reindex');

  if (brush_get_option('immediate')) {
    brush_op('_brush_core_search_index');
    brush_log(bt('The search index has been rebuilt.'), 'ok');
  }
  else {
    brush_log(bt('The search index will be rebuilt.'), 'ok');
  }
}
