<?php

/**
 * Override FeedsFileFetcher and manipulate "done" state.
 */
class GluttonFetcher extends FeedsFileFetcher {

  /**
   * Implements FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $feeds_source) {
    $source_config = $feeds_source->getConfigFor($this);
    $source = $source_config['source'];
    $state = $feeds_source->state(FEEDS_FETCH);

    // Initial call: set up the fetch counter and totals.
    if (!isset($state->fetch_counter)) {
      $state->fetch_counter = 0;
    }

    // If files array is empty and we're under 'multiplier' we populate the
    // files array.
    if (empty($state->files) && $state->fetch_counter < $this->config['multiplier']) {
      $state->files = is_file($source) ? array($source) : $this->listFiles($source);
      $state->one_time_total = count($state->files);
      $state->total = count($state->files) * $this->config['multiplier'];
      $state->fetch_counter++;
    }

    // As long as there are files, return a result.
    if (count($state->files)) {
      $file = array_shift($state->files);
      $state->progress($state->total, ($state->one_time_total * $state->fetch_counter) - count($state->files));
      return new FeedsFileFetcherResult($file);
    }

    throw new Exception(t('Resource is not a file or it is an empty directory: %source', array('%source' => $source)));
  }

  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    return parent::configDefaults() + array(
      'multiplier' => 2,
    );
  }

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = array();
    $form['multiplier'] = array(
      '#type' =>'textfield',
      '#title' => t('Multiplier'),
      '#description' => t('Define how often a file / a directory should be imported.'),
      '#default_value' => $this->config['multiplier'],
    );
    return $form + parent::configForm($form_state);
  }

  /**
   * Helper. Deletes a file.
   */
  protected function deleteFile($fid, $feed_nid) {
    if ($file = file_load($fid)) {
      file_usage_delete($file, 'feeds', get_class($this), $feed_nid);
      file_delete($file);
    }
  }
}
