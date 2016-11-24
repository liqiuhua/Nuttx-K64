/****************************************************************************
 * include/nuttx/sched_note.h
 *
 *   Copyright (C) 2016 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_SCHED_NOTE_H
#define __INCLUDE_NUTTX_SCHED_NOTE_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <nuttx/sched.h>

#ifdef CONFIG_SCHED_INSTRUMENTATION

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifdef CONFIG_SCHED_INSTRUMENTATION_BUFFER

/* This type identifies a note structure */

enum note_type_e
{
  NOTE_START = 0,
  NOTE_STOP,
  NOTE_SUSPEND,
  NOTE_RESUME
#ifdef CONFIG_SCHED_INSTRUMENTATION_PREEMPTION
  ,
  NOTE_PREEMPT_LOCK,
  NOTE_PREEMPT_UNLOCK
#endif
#ifdef CONFIG_SCHED_INSTRUMENTATION_CSECTION
  ,
  NOTE_CSECTION_ENTER,
  NOTE_CSECTION_LEAVE
#endif
};

/* This structure provides the common header of each note */

struct note_common_s
{
  uint8_t nc_length;           /* Length of the note */
  uint8_t nc_type;             /* See enum note_type_e */
  uint8_t nc_priority;         /* Thread/task priority */
#ifdef CONFIG_SMP
  uint8_t nc_cpu;              /* CPU thread/task running on */
#endif
  uint8_t nc_pid[2];           /* ID of the thread/task */
  uint8_t nc_systime[4];       /* Time when note buffered */
};

/* This is the specific form of the NOTE_START note */

struct note_start_s
{
  struct note_common_s nst_cmn; /* Common note parameters */
#if CONFIG_TASK_NAME_SIZE > 0
  char    nst_name[1];          /* Start of the name of the thread/task */
#endif
};

/* This is the specific form of the NOTE_STOP note */

struct note_stop_s
{
  struct note_common_s nsp_cmn; /* Common note parameters */
};

/* This is the specific form of the NOTE_SUSPEND note */

struct note_suspend_s
{
  struct note_common_s nsu_cmn; /* Common note parameters */
  uint8_t nsu_state;            /* Task state */
};

/* This is the specific form of the NOTE_RESUME note */

struct note_resume_s
{
  struct note_common_s nre_cmn; /* Common note parameters */
};

#ifdef CONFIG_SCHED_INSTRUMENTATION_PREEMPTION
/* This is the specific form of the NOTE_PREEMPT_LOCK/UNLOCK note */

struct note_preempt_s
{
  struct note_common_s npr_cmn; /* Common note parameters */
  uint8_t npr_count[2];         /* Count of nested locks */
};
#endif /* CONFIG_SCHED_INSTRUMENTATION_PREEMPTION */

#ifdef CONFIG_SCHED_INSTRUMENTATION_CSECTION
/* This is the specific form of the NOTE_CSECTION_ENTER/LEAVE note */

struct note_csection_s
{
  struct note_common_s ncs_cmn; /* Common note parameters */
#ifdef CONFIG_SMP
  uint8_t ncs_count[2];         /* Count of nested csections */
#endif
};
#endif /* CONFIG_SCHED_INSTRUMENTATION_CSECTION */
#endif /* CONFIG_SCHED_INSTRUMENTATION_BUFFER */

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/********************************************************************************
 * Name: sched_note_*
 *
 * Description:
 *   If instrumentation of the scheduler is enabled, then some outboard logic
 *   must provide the following interfaces.  These interfaces are not availalble
 *   to application code.
 *
 *   NOTE: if CONFIG_SCHED_INSTRUMENTATION_BUFFER, then these interfaces are
 *   *not* available to the platform-specific logic.  Rather, they provided by
 *   the note buffering logic.  See sched_note_get() below.
 *
 * Input Parameters:
 *   tcb - The TCB of the thread.
 *
 * Returned Value:
 *   None
 *
 ********************************************************************************/

void sched_note_start(FAR struct tcb_s *tcb);
void sched_note_stop(FAR struct tcb_s *tcb);
void sched_note_suspend(FAR struct tcb_s *tcb);
void sched_note_resume(FAR struct tcb_s *tcb);

#ifdef CONFIG_SCHED_INSTRUMENTATION_PREEMPTION
void sched_note_premption(FAR struct tcb_s *tcb, bool locked);
#endif

#ifdef CONFIG_SCHED_INSTRUMENTATION_CSECTION
void sched_note_csection(FAR struct tcb_s *tcb, bool enter);
#endif

/****************************************************************************
 * Name: sched_note_get
 *
 * Description:
 *   Remove the next note from the tail of the circular buffer.  The note
 *   is also removed from the circular buffer to make room for futher notes.
 *
 * Input Parameters:
 *   buffer - Location to return the next note
 *   buflen - The length of the user provided buffer.
 *
 * Returned Value:
 *   On success, the positive, non-zero length of the return note is
 *   provided.  Zero is returned only if ther circular buffer is empty.  A
 *   negated errno value is returned in the event of any failure.
 *
 ****************************************************************************/

#ifdef CONFIG_SCHED_INSTRUMENTATION_BUFFER
ssize_t sched_note_get(FAR uint8_t *buffer, size_t buflen);
#endif

/****************************************************************************
 * Name: sched_note_size
 *
 * Description:
 *   Return the size of the next note at the tail of the circular buffer.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   Zero is returned if the circular buffer is empty.  Otherwise, the size
 *   of the next note is returned.
 *
 ****************************************************************************/

#ifdef CONFIG_SCHED_INSTRUMENTATION_BUFFER
ssize_t sched_note_size(void);
#endif

/****************************************************************************
 * Name: note_register
 *
 * Description:
 *   Register a serial driver at /dev/note that can be used by an
 *   application to read data from the circular not buffer.
 *
 * Input Parameters:
 *   None.
 *
 * Returned Value:
 *   Zero is returned if the circular buffer is empty.  Otherwise, a negated
 *   errno value is returned.
 *
 ****************************************************************************/

#if defined(CONFIG_SCHED_INSTRUMENTATION_BUFFER) && \
    defined(CONFIG_DRIVER_NOTE)
int note_register(void);
#endif

#else /* CONFIG_SCHED_INSTRUMENTATION */

#  define sched_note_start(t)
#  define sched_note_stop(t)
#  define sched_note_suspend(t)
#  define sched_note_resume(t)
#  define sched_note_premption(t,l)
#  define sched_note_csection(t,e)

#endif /* CONFIG_SCHED_INSTRUMENTATION */
#endif /* __INCLUDE_NUTTX_SCHED_NOTE_H */
