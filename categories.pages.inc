<?php

function categories_node($node, $type, $bundle = 'page') {
  $options = array(
    'query' => array(
      'new_category' => $type == CATEGORIES_ADD,
      'parent_category' => $node->nid,
    ),
  );

  drupal_goto("node/add/$bundle", $options, 301);
}

function categories_node_transform($form, &$form_state, $node) {
  $msg = t('Are you sure you want to transform %title into a category?', array('%title' => $node->title));
  $form['msg'] = array('#markup' => "<h3>$msg</h3>");

  $form['parent'] = array(
    '#title' => t('Parent Category'),
    '#description' => t('Select where this category will be nested under'),
    '#type' => 'select',
    // @todo implement this function
    '#options' => ul_utils_taxonomy_options(variable_get('categories_vid')),
    '#default_value' => empty($node->field_category[LANGUAGE_NONE][0]['tid']) ? NULL : $node->field_category[LANGUAGE_NONE][0]['tid'],
  );

  $form['nid'] = array('#type' => 'value', '#value' => $node->nid);

  return confirm_form($form,
    $msg,
    'node/' . $node->nid,
    t(''),
    t('Transform'),
    t('Cancel')
  );
}

function categories_node_transform_validate($form, &$form_state) {
  // @todo admin setting to let users add to root
  if (empty($form_state['values']['parent']) && empty($form_state['values']['existing'])) {
    form_set_error('parent', 'You must select a parent category to add this new category to.');
  }
}

function categories_node_transform_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $node = node_load($form_state['values']['nid']);
    $tid = FALSE;

    // @todo fix
    $tid = categories_create_taxonomy_term($node->title, FALSE, '', $form_state['values']['parent']);

    if ($tid) {
      $node->field_category_homepage[LANGUAGE_NONE][0]['tid'] = $tid;
      $node->field_category[LANGUAGE_NONE][0]['tid'] = $tid;
      node_save($node);

      // redirect the user to the new taxonomy term
      $form_state['redirect'] = 'taxonomy/term/' . $tid;
    }
  }
}
