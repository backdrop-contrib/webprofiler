<?php

/**
 * Form for Webprofiler settings.
 */
function webprofiler_settings_form($form, &$form_state) {
  $config = config('webprofiler.settings');
  $form['#config'] = 'webprofiler.settings';

//  $form['toolbar'] = array(
//    '#type' => 'fieldset',
//    '#title' => t('Toolbar'),
//  );
//  $form['toolbar']['show_toolbar'] = array(
//    '#type' => 'checkbox',
//    '#title' => t('Show summary of performance stats in a toolbar.'),
//    '#default_value' => $config->get('show_toolbar'),
//  );

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
    '#title' => t('Pages'),
    '#default_value' => $config->get('ignored_pages'),
  );
  $form['data_collection']['reverse_pages'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reverse pages'),
    '#default_value' => $config->get('reverse_pages'),
  );
//  $form['data_collection']['roles'] = array(
//    '#type' => 'textarea',
//    '#title' => t('Roles'),
//    '#default_value' => $config->get('roles'),
//  );
//  $form['data_collection']['reverse_roles'] = array(
//    '#type' => 'checkbox',
//    '#title' => t('Reverse roles'),
//    '#default_value' => $config->get('reverse_roles'),
//  );

//  $form['ignored_functions'] = array(
//    '#type' => 'fieldset',
//    '#title' => t('Ignored functions'),
//  );
//  $form['ignored_functions']['ignored_functions'] = array(
//    '#type' => 'textarea',
//    '#title' => t('Functions to be ignored by Xhprof when generating profiles.'),
//    '#description' => t('Put each function name on a seperate line. Class methods can be listed as ClassName::methodName.'),
//    '#default_value' => $config->get('ignored_functions'),
//  );

  return system_settings_form($form);
}
