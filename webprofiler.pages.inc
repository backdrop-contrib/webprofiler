<?php
/**
 * @file Contains functions to display profile data to users of the module.
 */

/**
 * A page to display a list of profiles.
 */
function webprofiler_list_profiles($form, &$form_state) {
  $params = backdrop_get_query_parameters();

  $path = '';
  if (isset($params['path']) && !empty($params['path'])) {
    $path = $params['path'];
  }

  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
    '#collapsible' => TRUE,
  );

  $form['filter']['path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path'),
    '#size' => 30,
    '#default_value' => $path,
  );
  $form['filter']['search'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#submit' => array('webprofiler_list_profiles_form_search_submit'),
  );
  $form['filter']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#submit' => array('webprofiler_list_profiles_form_reset_submit'),
  );

  // Format the table.
  $table = array(
    'header' => array(
      '',
      array(
        'data' => t('Path'),
        'field' => 'path',
      ),
      array(
        'data' => t('Date'),
        'field' => 'timestamp',
        'sort' => 'desc',
      ),
      array(
        'data' => t('Execution Time'),
        'field' => 'time',
      ),
      array(
        'data' => t('Peak Memory'),
        'field' => 'peak_memory',
      ),
      array(
        'data' => t('Queries'),
        'field' => 'queries',
      ),
      array(
        'data' => t('Function Calls'),
        'field' => 'function_calls',
      ),
      array(
        'data' => t('Actions'),
      ),
    ),
    'rows' => array(),
  );

  $query = db_select('webprofiler_profiles', 'wpp')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('wpp')
    ->limit(50)
    ->orderByHeader($table['header']);

  if (!empty($path)) {
    $query->condition('wpp.path', '%' . db_like($path) . '%', 'LIKE');
  }

  $results = $query->execute();

  foreach ($results as $row) {
    $table['rows'][] = array(
      '',
      l($row->path, url('admin/reports/webprofiler/profiles/' . $row->id)),
      format_date($row->timestamp, 'short'),
      $row->time . ' ms',
      WebprofilerProfile::formatMemory($row->peak_memory),
      $row->queries,
      $row->function_calls,
      l(t('Delete'), url('admin/reports/webprofiler/profiles/delete/' . $row->id)),
    );
  }

  $form['table'] = array(
    '#markup' => theme('table', $table),
  );

  $form['pager'] = array(
    '#markup' => theme('pager'),
  );

  return $form;
}

/**
 * A page callback to allow searching for profiles.
 */
function webprofiler_list_profiles_form_search_submit($form, &$form_state) {
  $params = backdrop_get_query_parameters();
  if (isset($form_state['values']['path']) && !empty($form_state['values']['path'])) {
    $path = urlencode($form_state['values']['path']);
    $params['path'] = $path;
  }

  backdrop_goto('admin/reports/webprofiler/profiles', array('query' => $params));
}

/**
 * A page callback to reset filters on the main profile listing.
 */
function webprofiler_list_profiles_form_reset_submit($form, &$form_state) {
  backdrop_goto('admin/reports/webprofiler/profiles');
}

/**
 * A page to view a summary of a given profile.
 */
function webprofiler_profile_view($profile_id) {
  $profile = webprofiler_get_profile($profile_id);
  if (!$profile) {
    return FALSE;
  }

  $output = array(
    '#theme' => 'table',
    '#rows' => array(
      array(
        array(
          'data' => t('Path'),
          'header' => TRUE,
        ),
        $profile->page['path'],
      ),
      array(
        array(
          'data' => t('Date'),
          'header' => TRUE,
        ),
        format_date($profile->page['timestamp']),
      ),
      array(
        array(
          'data' => t('Execution Time'),
          'header' => TRUE,
        ),
        $profile->page['time'] . ' ms',
      ),
      array(
        array(
          'data' => t('Peak Memory'),
          'header' => TRUE,
        ),
        $profile->page['peak_memory'],
      ),
      array(
        array(
          'data' => t('Function calls'),
          'header' => TRUE,
        ),
        $profile->countFunctionCalls(),
      ),
      array(
        array(
          'data' => t('queries'),
          'header' => TRUE,
        ),
        $profile->countQueries(),
      ),
    ),
  );

  return $output;
}

/**
 * A page to list functions from a given profile.
 */
function webprofiler_profile_functions_view($profile_id) {
  $profile = webprofiler_get_profile($profile_id);
  if (!$profile) {
    return FALSE;
  }

  backdrop_add_js(backdrop_get_path('module', 'webprofiler') . '/js/webprofiler.admin.js');
  backdrop_add_css(backdrop_get_path('module', 'webprofiler') . '/css/webprofiler.admin.css');

  backdrop_set_title('Functions for ' .$profile->page['path']);

  if (!isset($profile->functions) || empty($profile->functions)) {
    return t('This profile has no function data to display. It was probably generated without the Xhprof PHP extension.');
  }

  $output = array(
    '#theme' => 'table',
    '#header' => array(
      t('Function'),
      array(
        'data' => t('Count'),
        'field' => 'ct',
      ),
      array(
        'data' => t('Exclusive Wall Time'),
        'field' => 'ewt',
      ),
      array(
        'data' => t('Inclusive Wall Time'),
        'field' => 'wt',
        'sort' => 'desc',
      ),
      array(
        'data' => t('Exclusive CPU'),
        'field' => 'ecpu',
      ),
      array(
        'data' => t('Inclusive CPU'),
        'field' => 'cpu',
      ),
      array(
        'data' => t('Exclusive Memory'),
        'field' => 'emu',
      ),
      array(
        'data' => t('Inclusive Memory'),
        'field' => 'mu',
      ),
      array(
        'data' => t('Peak Memory'),
        'field' => 'pmu',
      ),
    ),
    '#rows' => [],
    '#prefix' => '<div class="form-item form-type-textfield form-item-path">
      <label for="function-search">Search</label>
      <input type="text" id="function-search" name="function-search" value="" class="form-text">
      </div>',
    '#attributes' => ['id' => 'function-table'],
  );

  $sort = tablesort_get_order($output['#header']);
  if ($sort['sql'] !== NULL) {
    $sort = $sort['sql'];
    $direction = tablesort_get_sort($output['#header']);

    uasort($profile->functions, function($a, $b) use ($sort, $direction) {
      if ($a[$sort] == $b[$sort]) {
          return 0;
      }
      if ($direction === 'asc') {
        return ($a[$sort] < $b[$sort]) ? -1 : 1;
      }
      return ($a[$sort] > $b[$sort]) ? -1 : 1;
    });
  }

  foreach ($profile->functions as $function_name => $stats) {
    $output['#rows'][] = [
      'data' => [
        _webprofiler_function_link($profile_id, $function_name),
        $stats['ct'],
        $profile->formatTime($stats['ewt']),
        $profile->formatTime($stats['wt']),
        $profile->formatTime($stats['ecpu']),
        $profile->formatTime($stats['cpu']),
        $profile->formatMemory($stats['emu']),
        $profile->formatMemory($stats['mu']),
        $profile->formatMemory($stats['pmu']),
      ],
      'data-function' => $function_name,
    ];
  }

  return $output;
}

/**
 * The menu callback for viewing a list of queries from a profile.
 *
 * @param string $profile_id
 *   The profile id of the profile that the queries should be loaded from.
 * @return bool|array
 */
function webprofiler_profile_queries_view($profile_id) {
  $profile = webprofiler_get_profile($profile_id);
  if (!$profile) {
    return FALSE;
  }

  backdrop_set_title('Database queries for ' .$profile->page['path']);

  $output = array(
    '#theme' => 'table',
    '#header' => array(
      array(
        'data' => t('Count'),
        'field' => 'count',
      ),
      array(
        'data' => t('Function'),
        'field' => 'function',
      ),
      t('Query String'),
      array(
        'data' => t('Duration'),
        'field' => 'time',
        'sort' => 'desc',
      ),
      t('Target'),
    ),
    '#rows' => array(),
  );

  // Count how many times each unique query string was used.
  $counts = array();
  foreach ($profile->queries as $key => $query) {
    if (!isset($counts[$query['query']])) {
      $counts[$query['query']] = 1;
    }
    else {
      $counts[$query['query']]++;
    }
  }

  // Sort the query list.
  $sort = tablesort_get_order($output['#header']);
  if ($sort['sql'] !== NULL) {
    $sort = $sort['sql'];
    $direction = tablesort_get_sort($output['#header']);

    if ($sort === 'count') {
      uasort($profile->queries, function($a, $b) use ($counts, $direction) {
        if ($counts[$a['query']] == $counts[$b['query']]) {
            return 0;
        }
        if ($direction === 'asc') {
          return ($counts[$a['query']] < $counts[$b['query']]) ? -1 : 1;
        }
        return ($counts[$a['query']] > $counts[$b['query']]) ? -1 : 1;
      });
    }
    else if ($sort === 'function') {
      uasort($profile->queries, function($a, $b) use ($direction) {
        $a_function = isset($a['caller']['class']) ? $a['caller']['class'] . '::' . $a['caller']['function'] : $a['caller']['function'];
        $b_function = isset($b['caller']['class']) ? $b['caller']['class'] . '::' . $b['caller']['function'] : $b['caller']['function'];
        if ($a_function === $b_function) {
            return 0;
        }
        if ($direction === 'asc') {
          return ($a_function < $b_function) ? -1 : 1;
        }
        return ($a_function > $b_function) ? -1 : 1;
      });
    }
    else {
      uasort($profile->queries, function($a, $b) use ($sort, $direction) {
        if ($a[$sort] == $b[$sort]) {
            return 0;
        }
        if ($direction === 'asc') {
          return ($a[$sort] < $b[$sort]) ? -1 : 1;
        }
        return ($a[$sort] > $b[$sort]) ? -1 : 1;
      });
    }
  }

  // Set the table rows.
  foreach ($profile->queries as $key => $query) {
    $row = array();

    if (isset($query['caller']['class'])) {
      $function_name = $query['caller']['class'] . '::' . $query['caller']['function'];
    }
    else {
      $function_name = $query['caller']['function'];
    }
    $row[] = $counts[$query['query']];
    $row[] = _webprofiler_function_link($profile_id, $function_name);
    $row[] = $query['query'];
    $row[] = WebprofilerProfile::formatQueryTime($query['time']);
    $row[] = $query['target'];
    $output['#rows'][] = $row;
  }

  return $output;
}

/**
 * A page that displays data about a given function for a given profile.
 */
function webprofiler_profile_function_view($profile_id, $function) {
  $function_name = urldecode($function);
  $profile = webprofiler_get_profile($profile_id);
  backdrop_set_title(check_plain($function_name . ' on ' . $profile->page['path']));
  $output = '';

  $output = _webprofiler_documentaion_link($function_name);

  if (isset($profile->functions) && !empty($profile->functions)) {
    if (isset($profile->functions[$function_name])) {
      $stats = $profile->functions[$function_name];

      $table = array(
        'header' => array(
          t('Count'),
          t('Exclusive Wall Time'),
          t('Inclusive Wall Time'),
          t('Exclusive CPU'),
          t('Inclusive CPU'),
          t('Exclusive Memory'),
          t('Inclusive Memory'),
          t('Peak Memory'),
        ),
        'rows' => array(array(
          $stats['ct'],
          $profile->formatTime($stats['ewt']),
          $profile->formatTime($stats['wt']),
          $profile->formatTime($stats['ecpu']),
          $profile->formatTime($stats['cpu']),
          $profile->formatMemory($stats['emu']),
          $profile->formatMemory($stats['mu']),
          $profile->formatMemory($stats['pmu']),
        )),
      );

      $output .= theme('table', $table);
    }

    if (!empty($profile->functions[$function_name]['parents'])) {
      $table = array(
        'header' => array(
          t('Function'),
          t('Count'),
          t('Exclusive Wall Time'),
          t('Inclusive Wall Time'),
          t('Exclusive CPU'),
          t('Inclusive CPU'),
          t('Exclusive Memory'),
          t('Inclusive Memory'),
          t('Peak Memory'),
        ),
        'rows' => array(),
      );
      foreach ($profile->functions[$function_name]['parents'] as $parent) {
        $parent_stats = $profile->functions[$parent];

        $table['rows'][] = array(
          _webprofiler_function_link($profile_id, $parent),
          $parent_stats['ct'],
          $profile->formatTime($parent_stats['ewt']),
          $profile->formatTime($parent_stats['wt']),
          $profile->formatTime($parent_stats['ecpu']),
          $profile->formatTime($parent_stats['cpu']),
          $profile->formatMemory($parent_stats['emu']),
          $profile->formatMemory($parent_stats['mu']),
          $profile->formatMemory($parent_stats['pmu']),
        );
      }

      $output .= '<h3>Parent functions</h3>';
      $output .= theme('table', $table);
    }

    if (isset($profile->children[$function_name])) {
      $table = array(
        'header' => array(
          t('Function'),
          t('Count'),
          t('Inclusive Wall Time'),
          t('Inclusive CPU'),
          t('Inclusive Memory'),
          t('Peak Memory'),
        ),
        'rows' => array(),
      );

      foreach ($profile->children[$function_name] as $child_function => $child_stats) {
        $table['rows'][] = array(
          _webprofiler_function_link($profile_id, $child_function),
          $child_stats['ct'],
          $profile->formatTime($child_stats['wt']),
          $profile->formatTime($child_stats['cpu']),
          $profile->formatMemory($child_stats['mu']),
          $profile->formatMemory($child_stats['pmu']),
        );
      }

      $output .= '<h3>Child functions</h3>';
      $output .= theme('table', $table);
    }
  }
  else {
    $output .= t('This profile has no function data to display. It was probably generated without the Xhprof PHP extension.');
  }

  $queries = $profile->getQueriesByCaller($function_name);
  if (!empty($queries)) {
    $table = array(
      'header' => array(
        t('Count'),
        t('Execution time'),
        t('Query string'),
        t('Target'),
      ),
      'rows' => array(),
    );

    $counts = array();
    foreach ($queries as $query) {
      if (!isset($counts[$query['query']])) {
        $counts[$query['query']] = 1;
      }
      else {
        $counts[$query['query']]++;
      }
    }

    uasort($queries, function($a, $b) {
      if ($a['time'] == $b['time']) {
        return 0;
      }
      return ($a['time'] > $b['time']) ? -1 : 1;
    });

    foreach ($queries as $query) {
      $table['rows'][] = array(
        $counts[$query['query']],
        WebprofilerProfile::formatQueryTime($query['time']),
        $query['query'],
        $query['target'],
      );
    }

    $output .= '<h3>Queries</h3>';
    $output .= theme('table', $table);
  }

  return $output;
}

/**
 * A page callback to delete a profile.
 */
function webprofiler_profile_delete($profile_id) {
  db_delete('webprofiler_profiles')
    ->condition('id', $profile_id)
    ->execute();

  $filename = 'public://webprofiler_profiles/' . $profile_id;
  if (file_exists($filename)) {
    unlink($filename);
  }

  // @TODO: fix redirecting after deleting a profile.
  backdrop_goto('admin/reports/webprofiler/profiles');
}
