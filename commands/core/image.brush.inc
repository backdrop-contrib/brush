<?php

/**
 * @file
 *  Image module's brush integration.
 *
 *  @todo image-build($field_name, $bundle, $style_name)
 */

/**
 * Implementation of hook_brush_command().
 */
function image_brush_command() {
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
      'brush image-flush' => 'Pick an image style and then delete its images.',
      'brush image-flush thumbnail' => 'Delete all thumbnail images.',
      'brush image-flush --all' => 'Flush all derived images. They will be regenerated on the fly.',
    ),
  );
  return $items;
}

function brush_image_flush($style_name = NULL) {
  if (brush_get_option('all')) {
    brush_image_flush_all();
  }
  elseif (empty($style_name)) {
    $choices = backdrop_map_assoc(array_keys(image_styles()));
    $choices = array_merge(array('all' => 'all'), $choices);
    $style_name = brush_choice($choices, bt("Choose a style to flush."));
    if ($style_name == 'all') {
      brush_image_flush_all();
    }
    else {
      $commands = brush_get_commands();
      return brush_dispatch($commands['image-flush'], array($style_name));
    }
  }
  else {
    if ($style = image_style_load($style_name)) {
      image_style_flush($style);
      brush_log(bt('Image style !style_name flushed', array('!style_name' => $style_name)), 'success');
    }
    else {
      return brush_set_error(bt('Image style !style not recognized.', array('!style' => $style_name)));
    }
  }
}

function brush_image_flush_all() {
  foreach (image_styles() as $style) {
    image_style_flush($style);
  }
  brush_log(bt('All image styles flushed'), 'success');
}
