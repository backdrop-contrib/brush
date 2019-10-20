<?php
/**
 * @file
 *    Drop batch API.
 *
 * This file contains a fork of the Backdrop Batch API that has been drastically
 * simplified and tailored to Drop's unique use case.
 *
 * The existing API is very targeted towards environments that are web accessible,
 * and would frequently attempt to redirect the user which would result in the
 * drop process being completely destroyed with no hope of recovery.
 *
 * While the original API does offer a 'non progressive' mode which simply
 * calls each operation in sequence within the current process, in most
 * implementations (Backdrop 5 and 6), it would still attempt to redirect
 * unless very specific conditions were met.
 *
 * When operating in 'non progressive' mode, Drop would experience the problems
 * that the API was written to solve in the first place, specifically that processes
 * would exceed the available memory and exit with an error.
 *
 * Each major release of Backdrop has also had slightly different implementations
 * of the batch API, and this provides a uniform interface to all of these
 * implementations.
 *
 */


/**
 * Process a Backdrop batch by spawning multiple Drop processes.
 *
 * This function will include the correct batch engine for the current
 * major version of Backdrop, and will make use of the drop_backend_invoke
 * system to spawn multiple worker threads to handle the processing of
 * the current batch, while keeping track of available memory.
 *
 * The batch system will process as many batch sets as possible until
 * the entire batch has been completed or half of the available memory
 * has been used.
 *
 * This function is a drop in replacement for the existing batch_process()
 * function of Backdrop.
 *
 * @param command
 *   The command to call for the back end process. By default this will be
 *   the 'backend-process' command, but some commands such as updatedb will
 *   have special initialization requirements, and will need to define and
 *   use their own command.
 *
 */
function drop_backend_batch_process($command = 'batch-process') {
  drop_include_engine('backdrop', 'batch', drop_backdrop_major_version());
  _drop_backend_batch_process($command);
}

/**
 * Process sets from the specified batch.
 *
 * This function is called by the worker process that is spawned by the
 * drop_backend_batch_process function.
 *
 * The command called needs to call this function after it's special bootstrap
 * requirements have been taken care of.
 */
function drop_batch_command($id) {
  include_once('core/includes/batch.inc');
  drop_include_engine('backdrop', 'batch', drop_backdrop_major_version());
  _drop_batch_command($id);
}
