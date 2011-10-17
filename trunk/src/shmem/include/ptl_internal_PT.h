#ifndef PTL_INTERNAL_PT_H
#define PTL_INTERNAL_PT_H

#include <pthread.h>
#include <stdint.h>                    /* for uint32_t */

#include "ptl_internal_alignment.h"
#include "ptl_internal_locks.h"

typedef struct {
    PTL_LOCK_TYPE   lock;
    ptl_handle_eq_t EQ;
    uint32_t        status;
    unsigned int    options;
    struct PTqueue {
        void *head, *tail;
    } priority,
      overflow,
      buffered_headers;
} ptl_table_entry_t ALIGNED (64);

void PtlInternalPTInit(ptl_table_entry_t *t);
int  PtlInternalPTValidate(ptl_table_entry_t *t);

#include "ptl_internal_commpad.h"

void PtlInternalPTBufferUnexpectedHeader(ptl_table_entry_t *restrict const           t,
                                         const ptl_internal_header_t *restrict const hdr,
                                         const uintptr_t                             entry_ptr,
                                         const uintptr_t                             data);

#endif /* ifndef PTL_INTERNAL_PT_H */
/* vim:set expandtab: */