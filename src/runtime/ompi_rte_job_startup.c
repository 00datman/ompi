/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 The Trustees of the University of Tennessee.
 *                         All rights reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 */
/** @file:
 *
 * 
 *
 */

/*
 * includes
 */

#include "ompi_config.h"

#include "mca/oob/oob.h"
#include "mca/oob/base/base.h"
#include "mca/ns/base/base.h"

#include "runtime/runtime.h"


int ompi_rte_job_startup(mca_ns_base_jobid_t jobid)
{
    ompi_list_t *recipients;
    ompi_buffer_t startup_msg;
    ompi_name_server_namelist_t *ptr;
    ompi_rte_process_status_t *proc_status;
    int num_procs;
    size_t buf_size;

    if (ompi_rte_debug_flag) {
        	ompi_output(0, "[%d,%d,%d] entered rte_job_startup for job %d",
        		    OMPI_NAME_ARGS(*ompi_rte_get_self()), (int)jobid);
    }

    recipients = OBJ_NEW(ompi_list_t);

    startup_msg = ompi_registry.get_startup_msg(jobid, recipients);
    ompi_registry.triggers_active(jobid);

    if (ompi_rte_debug_flag) {
        ompi_buffer_size(startup_msg, &buf_size);
        	ompi_output(0, "[%d,%d,%d] rte_job_startup: sending startup message of size %d to %d recipients",
        		    OMPI_NAME_ARGS(*ompi_rte_get_self()), (int)buf_size,
        		    ompi_list_get_size(recipients));
    }

    /* check to ensure there are recipients on list - don't send if not */
    if (0 < (num_procs = (int)ompi_list_get_size(recipients))) {
	mca_oob_xcast(ompi_rte_get_self(), recipients, startup_msg, NULL);

    if (ompi_rte_debug_flag) {
         ompi_output(0, "[%d,%d,%d] rte_job_startup: completed xcast of startup message",
                 OMPI_NAME_ARGS(*ompi_rte_get_self()));
    }

        /* for each recipient, set process status to "running" */

	while (NULL != (ptr = (ompi_name_server_namelist_t*)ompi_list_remove_first(recipients))) {
		proc_status = ompi_rte_get_process_status(ptr->name);
		proc_status->status_key = OMPI_PROC_RUNNING;
		proc_status->exit_code = 0;
		ompi_rte_set_process_status(proc_status, ptr->name);
		free(proc_status);
	}
    }

   OBJ_RELEASE(recipients);

    /* return number of processes started = number of recipients */
    return num_procs;

}
