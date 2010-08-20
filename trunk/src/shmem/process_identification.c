/* The API definition */
#include <portals4.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* System headers */
#include <assert.h>
#include <limits.h>		       /* for UINT_MAX */
#include <string.h>		       /* for memcpy() */

/* Internals */
#include "ptl_internal_commpad.h"
#include "ptl_internal_pid.h"
#include "ptl_internal_nit.h"
#include "ptl_visibility.h"
#include "ptl_internal_handles.h"

const ptl_uid_t PTL_UID_ANY = UINT_MAX;

int INTERNAL PtlInternalLogicalProcessValidator(ptl_process_t p)
{
    return (p.rank >= num_siblings);
}

int INTERNAL PtlInternalPhysicalProcessValidator(ptl_process_t p)
{
    /* pid == num_siblings is the COLLECTOR */
    return (p.phys.pid > num_siblings || p.phys.nid != 0);
}

int API_FUNC PtlGetId(
    ptl_handle_ni_t ni_handle,
    ptl_process_t * id)
{
    ptl_handle_encoding_t ni;
    memcpy(&ni, &ni_handle, sizeof(ptl_handle_ni_t));
#ifndef NO_ARG_VALIDATION
    if (comm_pad == NULL) {
	return PTL_NO_INIT;
    }
    if (ni.ni > 3 || nit.refcount[ni.ni] == 0) {
	return PTL_ARG_INVALID;
    }
#endif
    switch (ni.ni) {
	case 0: // Logical
	case 1: // Logical
	    id->rank = proc_number;    // heh
	    break;
	case 2: // Physical
	case 3: // Physical
	    id->phys.pid = proc_number;
	    id->phys.nid = 0;
	    break;
	default:
	    *(int *)0 = 0;	       // should never happen
    }
    return PTL_OK;
}
