<?php

/**
 * @file
 *  Image module's drop integration.
 *
 *  @todo image-build($field_name, $bundle, $style_name)
 */

/**
 * Implementation of hook_drop_command().
 */
function image_drop_command() {
  $items['image-flush'] = array(
    'description' => 'Flush all derived images for a given style.',
    'backdrop' => array('1+'),
    'backdrop_dependencies' => array('image'),
    'arguments' => array(
      'style' => 'An image style machine name. If not provided, user may choose from a list of names.',
    ),
    'options' => array(
      'all' => 'Flush all derived images',
    ),
    'examples' => array(
      'drop image-flush' => 'Pick an image style and then delete its images.',
      'drop image-flush thumbnail' => 'Delete all thumbnail images.',
      'drop image-flush --all' => 'Flush all derived images. They will be regenerated on the fly.',
    ),
  );
  return $items;
}

function drop_image_flush($style_name = NULL) {
  if (drop_get_option('all')) {
    drop_image_flush_all();
  }
  elseif (empty($style_name)) {
    $choices = backdrop_map_assoc(array_keys(image_styles()));
    $choices = array_merge(array('all' => 'all'), $choices);
    $style_name = drop_choice($choices, dt("Choose a style to flush."));
    if ($style_name == 'all') {
      drop_image_flush_all();
    }
    else {
      $commands = drop_get_commands();
      return drop_dispatch($commands['image-flush'], array($style_name));
    }
  }
  else {
    if ($style = image_style_load($style_name)) {
      image_style_flush($style);
      drop_log(dt('Image style !style_name flushed', array('!style_name' => $style_name)), 'success');
    }
    else {
      return drop_set_error(dt('Image style !style not recognized.', array('!style' => $style_name)));
    }
  }
}

function drop_image_flush_all() {
  foreach (image_styles() as $style) {
    image_style_flush($style);
  }
  drop_log(dt('All image styles flushed'), 'success');
}
