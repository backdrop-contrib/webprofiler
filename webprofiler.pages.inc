<?php

function webprofiler_list_profiles() {
  $table = array(
    'header' => array(
      array(
        'data' => 'Path',
        'field' => 'path',
      ),
      array(
        'data' => 'Date',
        'field' => 'timestamp',
        'sort' => 'desc',
      ),
      array(
        'data' => 'Execution Time',
        'field' => 'time',
      ),
      array(
        'data' => 'Peak Memory',
        'field' => 'peak_memory',
      ),
      array(
        'data' => 'Queries',
        'field' => 'queries',
      ),
      array(
        'data' => 'Function Calls',
        'field' => 'function_calls',
      ),
    ),
    'rows' => array(),
  );

  $query = db_select('webprofiler_profiles', 'dpp')
    ->extend('PagerDefault')
    ->extend('TableSort')
    ->fields('dpp')
    ->limit(50)
    ->orderByHeader($table['header']);

  $results = $query->execute();
  foreach ($results as $row) {
    $table['rows'][] = array(
      l($row->path, url('admin/reports/webprofiler/profile/' . $row->id)),
      format_date($row->timestamp, 'short'),
      $row->time . ' ms',
      WebprofilerProfile::formatMemory($row->peak_memory),
      $row->queries,
      $row->function_calls,
    );
  }

  $output = theme('table', $table);
  $output .= theme('pager');

  return $output;
}

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

function webprofiler_profile_functions_view($profile_id) {
  $profile = webprofiler_get_profile($profile_id);
  if (!$profile) {
    return FALSE;
  }

  backdrop_set_title('Functions for ' .$profile->page['path']);

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
    '#rows' => array(),
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
      _webprofiler_function_link($profile_id, $function_name),
      $stats['ct'],
      $profile->formatTime($stats['ewt']),
      $profile->formatTime($stats['wt']),
      $profile->formatTime($stats['ecpu']),
      $profile->formatTime($stats['cpu']),
      $profile->formatMemory($stats['emu']),
      $profile->formatMemory($stats['mu']),
      $profile->formatMemory($stats['pmu']),
    ];
  }

  return $output;
}

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
        'sort' => 'desc'
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
    $row[] = WebprofilerProfile::formatTime(round($query['time'] * 1000000));
    $row[] = $query['target'];
    $output['#rows'][] = $row;
  }

  return $output;
}

function webprofiler_profile_function_view($profile_id, $function) {
  $function_name = urldecode($function);
  $profile = webprofiler_get_profile($profile_id);
  backdrop_set_title(check_plain($function_name . ' on ' . $profile->page['path']));
  $output = '';

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
        WebprofilerProfile::formatTime(round($query['time'] * 1000000)),
        $query['query'],
        $query['target'],
      );
    }

    $output .= '<h3>Queries</h3>';
    $output .= theme('table', $table);
  }

  return $output;
}