<?php

$plugin = array(
  'single' => TRUE,
  'title' => t('Pages in this category'),
  'description' => t('Display pages that belong to a category homepage'),
  'category' => t('Widgets'),
  'edit form' => 'categories_nodes_edit_form',
  'render callback' => 'categories_nodes_render',
  'admin info' => 'categories_nodes_admin_info',
  'defaults' => array(
    'text' => '',
  ),
  'all contexts' => TRUE,
);

/**
 * An edit form for the pane's settings.
 */
function categories_nodes_edit_form($form, &$form_state) {
  return $form;
}

function categories_nodes_render($subtype, $conf, $args, $contexts) {

  return (object) array(
    'title' => '',
    'content' => categories_get_homepage_nodes($args[0]),
  );
}

/**
 * 'admin info' callback for panel pane.
 */
function categories_nodes_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    return (object) array( 'title' => $conf['override_title'] ? $conf['override_title_text'] : '');
  }
}
