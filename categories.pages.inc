<?php

/**
 * @file categories.pages.inc
 *
 * @author Joe Corall <jcorall@kent.edu>
 */


function categories_node($node, $type) {
  $options = array(
    'query' => array(
      'new_category' => $type == CATEGORIES_ADD,
      'parent_category' => $node->nid,
    ),
  );

  drupal_goto("node/add/{$node->type}", $options, 301);
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
    $tid = categories_create_taxonomy_term($node->title, '', $form_state['values']['parent']);

    if ($tid) {
      $node->field_category_homepage[LANGUAGE_NONE][0]['tid'] = $tid;
      $node->field_category[LANGUAGE_NONE][0]['tid'] = $tid;
      node_save($node);

      // redirect the user to the new taxonomy term
      $form_state['redirect'] = 'taxonomy/term/' . $tid;
    }
  }
}

function categories_node_remove_category($form, &$form_state, $node) {
  // don't allow users to remove the homepage functionality if this category and any child pages/categories
  // more of a warning then anything else
  // @todo perhaps just allow an override???
  $children = db_query('SELECT nid,title FROM {field_data_field_category} c
    INNER JOIN {node} n ON n.nid = c.entity_id
    WHERE field_category_tid = :tid AND n.nid <> :nid', array(':tid' => $node->field_category_homepage[LANGUAGE_NONE][0]['tid'], ':nid' => $node->nid))->fetchAllKeyed();
  if ($children) {
    $form['warning'] = array(
      '#markup' => 'You can not turn this category into a page until it has no sub-catgories or subpages.<br>You will need to update all of these pages and move them to a different category:<br><br><ul>'
    );

    foreach ($children as $nid => $title) {
      $form['warning']['#markup'] .= '<li>' .l($title, 'node/' . $nid). '</li>';
    }

    $form['warning']['#markup'] .= '</ul>';

    return $form;
  }
  else {
    $msg = t('Are you sure you want to transform the category %title into a page?', array('%title' => $node->title));

    $form['nid'] = array('#type' => 'value', '#value' => $node->nid);

    return confirm_form($form,
      $msg,
      'node/' . $node->nid,
      t('This action can not be undone.'),
      t('Transform'),
      t('Cancel')
    );
  }
}

function categories_node_remove_category_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $node = node_load($form_state['values']['nid']);
    // get the {taxonomy_term_data}.tid of the homepage category
    $tid = $node->field_category_homepage[LANGUAGE_NONE][0]['tid'];

    // find it's parent so we can assign this page to that term
    $parent = db_query('SELECT parent FROM {taxonomy_term_hierarchy} WHERE tid = :tid', array(':tid' => $tid))->fetchField();
    $node->field_category[LANGUAGE_NONE][0]['tid'] = $parent;

    // remove the homepage value
    unset($node->field_category_homepage[LANGUAGE_NONE]);

    // save changes and remove the homepage term
    node_save($node);
    taxonomy_term_delete($tid);

    // flush menu cache
    cache_clear_all('*', 'cache_menu', TRUE);

    $form_state['redirect'] = 'node/' . $node->nid;
  }
}
