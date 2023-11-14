<?php
/**
 * @file Contains admin pages for managing the Webprofiler module and its data.
 */

/**
 * Form for Webprofiler settings.
 */
function webprofiler_settings_form($form, &$form_state) {
  $config = config('webprofiler.settings');
  $form['#config'] = 'webprofiler.settings';

  $form['data_collection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Data collection'),
  );

  $form['data_collection']['collect_profiles'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect data for profiles and save it for viewing later.'),
    '#default_value' => $config->get('collect_profiles'),
  );

  $form['data_collection']['ignored_pages'] = array(
    '#type' => 'textarea',
    '#title' => t('Ignored Pages'),
    '#default_value' => $config->get('ignored_pages'),
    '#description' => '',
  );
  $form['data_collection']['ignored_pages']['#description'] .= t('Put each path pattern on a new line.');
  $form['data_collection']['ignored_pages']['#description'] .= ' ' . t('Use * as a wildcard when multiple paths need to be matched.');
  $form['data_collection']['ignored_pages']['#description'] .= ' ' . t('Matched pages will not generate profiles.');

  $form['data_collection']['reverse_pages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reverse pages'),
    '#default_value' => $config->get('reverse_pages'),
    '#description' => t('Reverses the effect of the Ignored Pages setting so it will only collect profiles for matched pages.'),
  );

  $form['toolbar'] = array(
    '#type' => 'fieldset',
    '#title' => t('Toolbar'),
  );
  $form['toolbar']['show_toolbar'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show summary of performance stats in a toolbar.'),
    '#default_value' => $config->get('show_toolbar'),
    '#description' => t('The toolbar will only show up when profile data is collected.'),
  );

  $form['clear_data'] = array(
    '#type' => 'fieldset',
    '#title' => t('Clear Data'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Removes all profile indexes from the database and all profile dumps from the filesystem.'),
  );
  $form['clear_data']['clear_data'] = array(
    '#type' => 'link',
    '#title' => t('Delete all data'),
    '#href' => 'admin/config/development/webprofiler/clear_data',
    '#attributes' => array('class' => array('button', 'button-danger')),
  );

  return system_settings_form($form);
}

/**
 * Form for clearing profile data.
 */
function webprofiler_clear_data_form($form, &$form_state) {
  $files = scandir('public://webprofiler_profiles/');
  $file_count = count($files) - 2;

  $database_count = db_query('SELECT COUNT(1) FROM {webprofiler_profiles}')->fetchField();

  $form['warning'] = array(
    '#markup' => t('This will delete @file_count files and @database_count database records.', array(
      '@file_count' => $file_count,
      '@database_count' => $database_count,
    )),
  );

  $form['actions'] = array(
    '#type' => 'container',
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#attributes' => array('class' => array('button', 'button-danger')),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/development/webprofiler',
  );

  return $form;
}

/**
 * A submit handler that clears out all of the existing profile data.
 */
function webprofiler_clear_data_form_submit($form, &$form_state) {
  $files = scandir('public://webprofiler_profiles/');
  foreach ($files as $file) {
    if (!is_dir($file)) {
      unlink('public://webprofiler_profiles/' . $file);
    }
  }

  db_query('TRUNCATE TABLE {webprofiler_profiles}')->execute();
  backdrop_goto('admin/config/development/webprofiler');
}
