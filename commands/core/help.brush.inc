<?php

/**
 * Build a fake command for the purposes of showing examples and options.
 */
function brush_global_options_command($brief = FALSE) {
  $global_options_help = array(
    'description' => 'Execute a brush command. Run `brush help [command]` to view command-specific help.  Run `brush topic` to read even more documentation.',
    'sections' => array(
      'options' => 'Global options (see `brush topic core-global-options` for the full list).',
    ),
    'options' => brush_get_global_options($brief),
    'examples' => array(
      'brush dl cck zen' => 'Download CCK module and Zen theme.',
      'brush --uri=http://example.com status' => 'Show status command for the example.com multi-site.',
    ),
  );
  $global_options_help += brush_command_defaults('global-options', 'global_options', __FILE__);

  return $global_options_help;
}

/**
 * Command callback for help command. This is the default command, when none
 * other has been specified.
 */
function brush_core_help() {
  $commands = func_get_args();

  if (empty($commands)) {
    // For speed, only bootstrap up to BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION.
    brush_bootstrap_max(BRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION);
    $implemented = brush_get_commands();
    
    // Organize all commands into categories
    $command_categories = array();
    $category_map = array();
    foreach ($implemented as $key => $candidate) {
      if ((!array_key_exists('is_alias', $candidate) || !$candidate['is_alias']) && !$candidate['hidden']) {
	$category = $candidate['commandfile'];
	// If we have decided to remap a category, remap every command
	if (array_key_exists($category, $category_map)) {
	  $category = $category_map[$category];
	}
	if (!array_key_exists($category, $command_categories)) {
          $title = brush_command_invoke_all('brush_help', "meta:$category:title");
	  $alternate_title = '';
	  if (!$title) {
	    // If there is no title, then check to see if the
	    // command file is stored in a folder with the same
	    // name as some other command file (e.g. 'core') that
	    // defines a title.
	    $alternate = basename($candidate['path']);
            $alternate_title = brush_command_invoke_all('brush_help', "meta:$alternate:title");
	  }
	  if (!empty($alternate_title)) {
	    $category_map[$category] = $alternate;
	    $category = $alternate;
	    $title = $alternate_title;
	  }
	  $command_categories[$category]['title'] = empty($title) ? '' : $title[0];
          $summary = brush_command_invoke_all('brush_help', "meta:$category:summary");
          if ($summary) {
            $command_categories[$category]['summary'] = $summary[0];
	  }
	}
	$command_categories[$category]['commands'][] = $key;
      }
    }
    
    // Sort the command categories; make sure that 'core' is
    // always first in the list
    $core_category = array('core' => $command_categories['core']);
    unset($command_categories['core']);
    // Post-process the categories that have no title.
    // Any that have fewer than 4 commands will go into a
    // section called "other".
    $processed_categories = array();
    $misc_categories = array();
    $other_commands = array();
    $other_categories = array();
    foreach ($command_categories as $key => $info) {
      if (empty($info['title'])) {
        $one_category = $key;
        if (count($info['commands']) < 4) {
	  $other_commands = array_merge($other_commands, $info['commands']);
	  $other_categories[] = $one_category;
	}
	else {
	  $info['title'] = bt("All commands in !category", array('!category' => $key));
          $misc_categories[$one_category] = $info;
	}
      }
      else {
        $processed_categories[$key] = $info;
      }
    }
    $other_category = array();
    if (!empty($other_categories)) {
      $other_category[implode(',', $other_categories)] = array('title' => bt("Other commands"), 'commands' => $other_commands);
    }
    asort($processed_categories);
    asort($misc_categories);
    $command_categories = array_merge($core_category, $processed_categories, $misc_categories, $other_category);
    
    // If the user specified --filter w/out a value, then
    // present a list of help options.
    if (brush_get_option('filter', FALSE) === TRUE) {
      $help_categories = array();
      foreach ($command_categories as $key => $info) {
        $description = $info['title'];
	if (array_key_exists('summary', $info)) {
	  $description .= ": " . $info['summary'];
	}
	$help_categories[$key] = $description;
       }
      $result = brush_choice($help_categories, 'Select a help category:');
      if (!$result) {
	return brush_user_abort();
      }
      brush_set_option('filter', $result);
    }
    
    if (brush_get_option('html')) {
      brush_print(brush_help_html_header());
    }
    
    // Make a fake command section to hold the global options, then print it.
    $global_options_help = brush_global_options_command(TRUE);
    if ((!brush_get_option('html')) && (!brush_get_option('filter'))) {
      brush_print_help($global_options_help);
    }
    
    // Filter out categories that the user does not want to see
    if ($filter_category = brush_get_option('filter')) {
      if (!array_key_exists($filter_category, $command_categories)) {
        return brush_set_error('BRUSH_NO_CATEGORY', bt("The specified command category !filter does not exist.", array('!filter' => $filter_category)));
      }
      $command_categories = array($filter_category => $command_categories[$filter_category]);
    }

    // If the user specified --sort, then merge all of the remaining
    // categories together
    if (brush_get_option('sort', FALSE)) {
      $combined_commands = array();
      foreach ($command_categories as $key => $info) {
        $combined_commands = array_merge($combined_commands, $info['commands']);
      }
      $command_categories = array('all' => array('commands' => $combined_commands, 'title' => bt("Commands:")));
    }
    
    // Next, print out the table of commands by category.
    $all_commands = array();
    foreach ($command_categories as $key => $info) {
      // Get the commands in this category and sort them
      $commands = $info['commands'];
      sort($commands);
      
      // Remove hidden commands and build output for brush_print_table.
      $rows = array();
      foreach($commands as $cmd) {
        $command = $implemented[$cmd];
	$all_commands[$cmd] = $command;
        $name = $command['aliases'] ? $cmd . ' (' . implode(', ', $command['aliases']) . ')': $cmd;
        $rows[$cmd] = array($name, $command['description']);
        $pipe[] = $cmd;
      }
      
      // Vary the output by mode: text or html
      if (brush_get_option('html')) {
        $sorted_commands = array();
        foreach($commands as $cmd) {
          $sorted_commands[$cmd] = $implemented[$cmd];
	}
	
        brush_print("<h3>" . $info['title'] . "</h3>");
        brush_print(brush_help_html_command_list($sorted_commands));
      }
      else {      
        brush_print($info['title'] . ": (" . $key . ")");
	brush_print_table($rows, FALSE, array(0 => 20));
      }
    }
    
    // Print out the long-form help for all commands
    if (brush_get_option('html')) {
      brush_print(brush_help_html_global_options($global_options_help));
      brush_print(brush_help_html($all_commands));
    }
    
    // Newline-delimited list for use by other scripts. Set the --pipe option.
    if (brush_get_option('pipe') && isset($pipe)) {
    sort($pipe);
      brush_print_pipe(implode(" ", $pipe));
    }
    return;
  }
  else {
    $result = TRUE;
    while ((count($commands) > 0) && !brush_get_error()) {
      $result =  brush_show_help(array_shift($commands));
    }
    return $result;
  }

  return brush_set_error('BRUSH_COMMAND_NOT_FOUND', bt('Invalid command !command.', array('!command' => implode(" ", $commands))));
}


/**
 * Return an HTML page header that includes all global options.
 */
function brush_help_html_header() {
  $output =  "<html><head><title>brush help</title><style>dt {font-size: 110%; font-weight: bold}</style></head><body>\n";
}

function brush_help_html_global_options($global_options_help) {
  // Global options
  $global_option_rows = brush_format_help_section($global_options_help, 'options');
  $output =  '<h3>Global Options (see `brush topic core-global-options` for the full list)</h3><table>';
  foreach ($global_option_rows as $row) {
    $output .= "<tr>";
    foreach ($row as $value) {
      $output .=  "<td>" . htmlspecialchars($value) . "</td>\n";
    }
    $output .= "</tr>";
  }
  $output .=  "</table>\n";
  
  return $output;
}

function brush_help_html_command_list($commands) {
  // Command table
  $output =  "<table>\n";
  foreach ($commands as $key => $command) {
    $output .=  "  <tr><td><a href=\"#$key\">$key</a></td><td>" . $command['description'] . "</td></tr>\n";
  }
  $output .=  "</table>\n";
  
  return $output;
}

/**
 * Return an HTML page documenting all available commands and global options.
 */
function brush_help_html($commands) {
  // Command details
  $output =  '<h3>Command detail</h3><dl>';
  foreach ($commands as $key => $command) {
    $output .=  "\n<a name=\"$key\"></a><dt>$key</dt><dd><pre>\n";
    ob_start();
    brush_show_help($key);
    $output .= ob_get_clean();
    $output .=  "</pre></dd>\n";
  }

  $output .=  "</body></html>\n";

  return $output;
}
