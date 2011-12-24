/* Copyright (C) 2007-2010 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

/*
 * Contains declarations of types, constants, and routines used by memory
 * checking framework.
 */

#ifndef QEMU_MEMCHECK_MEMCHECK_H
#define QEMU_MEMCHECK_MEMCHECK_H

#include "memcheck_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes memory access checking framework.
 * This routine is called from emulator's main routine on condition,
 * that emulator has been started with -memcheck option.
 * Param:
 *  tracing_flags - Parameters set for the -memcheck option. These parameters
 *  contain abbreviation for memchecking tracing messages that should be enabled
 *  for the emulator and guest systems.
 */
void memcheck_init(const char* tracing_flags);

// =============================================================================
// Handlers for memory allocation events, generated by the guest system.
// =============================================================================

/* Libc.so has been initialized by a process in guest's system.
 * This routine is called in response to TRACE_DEV_REG_LIBC_INIT event that is
 * fired up by the guest system on /dev/qemu_trace mapped page.
 * Param:
 *  pid - ID of the process in context of which libc.so has been initialized.
 */
void memcheck_guest_libc_initialized(uint32_t pid);

/* Guest system has allocated memory from heap.
 * This routine is called in response to TRACE_DEV_REG_MALLOC event that is
 * fired up by the guest system on /dev/qemu_trace mapped page.
 * Param:
 *  guest_address - Virtual address of allocation descriptor (MallocDesc) that
 *      contains information about allocated memory block. Note that this
 *      descriptor is located in the guests's user memory. Note also that
 *      emulator reports failure back to the guest by zeroing out libc_pid field
 *      of the structure, addressed by this parameter.
 */
void memcheck_guest_alloc(target_ulong guest_address);

/* Guest system is freeing memory to heap.
 * This routine is called in response to TRACE_DEV_REG_FREE_PTR event,
 * fired up by the guest system on /dev/qemu_trace mapped page.
 * Param:
 *  guest_address - Virtual address of free descriptor (MallocFree) that
 *      contains information about memory block that's being freed. Note that
 *      this descriptor is located in the guests's user memory.  Note also that
 *      emulator reports failure back to the guest by zeroing out libc_pid field
 *      of the structure, addressed by this parameter.
 */
void memcheck_guest_free(target_ulong guest_address);

/* Guest system has queried information about an address in its virtual memory.
 * This routine is called in response to TRACE_DEV_REG_QUERY_MALLOC event,
 * fired up by the guest system on /dev/qemu_trace mapped page.
 * Param:
 *  guest_address - Virtual address in the guest's space of the MallocDescQuery
 *      structure, that describes the query and receives the response. Note
 *      that emulator reports failure back to the guest by zeroing out libc_pid
 *      field of the structure, addressed by this parameter.
 */
void memcheck_guest_query_malloc(target_ulong guest_address);

/* Prints a string to emulator's stdout.
 * This routine is called in response to TRACE_DEV_REG_PRINT_USER_STR event,
 * fired up by the guest system on /dev/qemu_trace mapped page.
 * Param:
 *  str - Virtual address in the guest's space of the string to print.
 */
void memcheck_guest_print_str(target_ulong str);

// =============================================================================
// Handlers for events, generated by the kernel.
// =============================================================================

/* Handles PID initialization event.
 * This routine is called in response to TRACE_DEV_REG_INIT_PID event, which
 * indicates that new process has been initialized (but not yet executed).
 * Param:
 *  pid - ID of the process that is being initialized. This value will also be
 *      used as main thread ID for the intializing process.
 */
void memcheck_init_pid(uint32_t pid);

/* Handles thread switch event.
 * This routine is called in response to TRACE_DEV_REG_SWITCH event, which
 * indicates that thread switch occurred in the guest system.
 * Param:
 *  tid - ID of the thread that becomes active.
 */
void memcheck_switch(uint32_t tid);

/* Handles process forking / new process creation event.
 * This routine is called in response to TRACE_DEV_REG_FORK event, which
 * indicates that new process has been forked / created. It's assumed, that
 * process that is forking new process is the current process.
 * Param:
 *  tgid - TODO: Clarify that!
 *  new_pid - Process ID that's been assigned to the forked process.
 */
void memcheck_fork(uint32_t tgid, uint32_t new_pid);

/* Handles new thread creation event.
 * This routine is called in response to TRACE_DEV_REG_CLONE event, which
 * indicates that new thread has been created in context of the current process.
 * Param:
 *  tgid - TODO: Clarify that!
 *  new_tid - Thread ID that's been assigned to the new thread.
 */
void memcheck_clone(uint32_t tgid, uint32_t new_tid);

/* Sets process command line.
 * This routine is called in response to TRACE_DEV_REG_CMDLINE event, which
 * is used to grab first command line argument, and use it is image path to
 * the current process.
 * Param:
 *  cmg_arg - Command line arguments.
 *  cmdlen - Length of the command line arguments string.
 */
void memcheck_set_cmd_line(const char* cmd_arg, unsigned cmdlen);

/* Handles thread / process exiting event.
 * This routine is called in response to TRACE_DEV_REG_EXIT event, which
 * indicates that current thread is exiting. We consider that process is
 * exiting when last thread for that process is exiting.
 * Param:
 *  exit_code - Thread exit code.
 */
void memcheck_exit(uint32_t exit_code);

/* Handles memory mapping of a module in guest address space.
 * This routine is called in response to TRACE_DEV_REG_EXECVE_VMSTART,
 * TRACE_DEV_REG_EXECVE_VMEND, TRACE_DEV_REG_EXECVE_OFFSET, and
 * TRACE_DEV_REG_MMAP_EXEPATH events, which indicate that a module has been
 * loaded and mapped on the guest system.
 * Param:
 *  vstart - Guest address where mapping starts.
 *  vend - Guest address where mapping ends.
 *  exec_offset - Exec offset inside module mapping.
 *  path - Path to the module that has been mapped.
 */
void memcheck_mmap_exepath(target_ulong vstart,
                           target_ulong vend,
                           target_ulong exec_offset,
                           const char* path);

/* Handles memory unmapping of a module in guest address space.
 * This routine is called in response to TRACE_DEV_REG_UNMAP_START, and
 * TRACE_DEV_REG_UNMAP_END events, which indicate that a module has been
 * unmapped on the guest system.
 * Param:
 *  vstart - Guest address where unmapping starts.
 *  vend - Guest address where unmapping ends.
 */
void memcheck_unmap(target_ulong vstart, target_ulong vend);

/* Global flag, indicating whether or not memchecking has been enabled
 * for the current emulator session. If set to zero, indicates that memchecking
 * is not enabled. Value other than zero indicates that memchecking is enabled
 * for the current emulator session.
 */
extern int memcheck_enabled;

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  // QEMU_MEMCHECK_MEMCHECK_H
