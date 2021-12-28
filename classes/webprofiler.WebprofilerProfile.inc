<?php

class WebprofilerProfile {

  public $page = array();

  public $queries = array();

  public $xhprof = array();

  /**
   * @var Array The aggregated data for each function called.
   */
  public $functions = array();

  /**
   * @var array Each key is a function name and the values are arrays of performance data for the child functions.
   */
  public $children = array();

  function __construct($profile) {
    if (isset($profile['page'])) {
      $this->page = $profile['page'];
    }
    if (isset($profile['queries'])) {
      $this->queries = $profile['queries'];
    }
    if (isset($profile['xhprof'])) {
      $this->xhprof = $profile['xhprof'];
    }

    $this->process();
  }

  public function process() {
    foreach ($this->xhprof as $name => $stats) {
      list($parent, $function) = $this->splitXhprofName($name);

      if (isset($this->functions[$function])) {
        foreach ($stats as $stat_name => $stat_value) {
          $this->functions[$function][$stat_name] += $stat_value;
        }
        $this->functions[$function]['parents'][] = $parent;
      }
      else {
        $this->functions[$function] = $this->_getFunctionDefaults();
        foreach ($stats as $stat_name => $stat_value) {
          $this->functions[$function][$stat_name] = $stat_value;
        }
        $this->functions[$function]['parents'] = [$parent];
      }

      if (!isset($this->children[$parent])) {
        $this->children[$parent] = [];
      }
      $this->children[$parent][$function] = $stats;
    }

    foreach ($this->functions as $function_name => &$function) {
      $function['ewt'] = $function['wt'];
      $function['ecpu'] = $function['cpu'];
      $function['emu'] = $function['mu'];

      if (isset($this->children[$function_name])) {
        foreach ($this->children[$function_name] as $child) {
          $function['ewt'] -= $child['wt'];
          $function['ecpu'] -= $child['cpu'];
          $function['emu'] -= $child['mu'];
        }
      }
    }
  }

  public function splitXhprofName($name) {
    $parts = explode('==>', $name);
    if (isset($parts[1])) {
      return $parts;
    }
    return [ '__top__', $parts[0]];
  }

  protected function _getFunctionDefaults() {
    return [
      'ct' => 0,
      'ewt' => 0,
      'wt' => 0,
      'ecpu' => 0,
      'cpu' => 0,
      'emu' => 0,
      'mu' => 0,
      'pmu' => 0,
      'parents' => [],
    ];
  }

  public function countFunctionCalls() {
    $count = 0;

    foreach ($this->functions as $function) {
      $count += $function['ct'];
    }

    return $count;
  }

  public function countQueries() {
    return count($this->queries);
  }

  public function getQueriesByCaller($function) {
    $results = array();

    foreach ($this->queries as $query) {
      if ($query['caller']['class'] === NULL) {
        $caller = $query['caller']['function'];
      }
      else {
        $caller = $query['caller']['class'] . '::' . $query['caller']['function'];
      }
      if ($caller === $function) {
        $results[] = $query;
      }
    }

    return $results;
  }

  public function getPageTime() {
    if (isset($this->page['time'])) {
      if ($this->page['time'] > 1000) {
        return round($this->page['time'] / 1000, 2) . ' s';
      }
      return $this->page['time'] . ' ms';
    }
    return 0;
  }

  /**
   * Formats amounts of microseconds so they are easier to read.
   *
   * @param int $time An integer representing a number of microseconds.
   */
  public static function formatTime($time) {
    return number_format($time) . ' µs';
  }

  /**
   * Formats memory amounts so they are easier to read.
   *
   * @param int $memory An integer representing an amount of bytes of memory.
   */
  public static function formatMemory($memory) {
    $suffix = '';

    if (abs($memory) > 1024) {
      $memory = $memory / 1024;
      $suffix = ' KB';
    }

    if (abs($memory) > 1024) {
      $memory = $memory / 1024;
      $suffix = ' MB';
    }
    return number_format($memory, 1) . $suffix;
  }
}
