#ifndef LK_TYPEDEF_LIBC_H
#define LK_TYPEDEF_LIBC_H

/*
 * Typedefs for system (ISO C / POSIX / GNU libc) struct types so that
 * the `struct` keyword does not have to be spelled out in library code.
 *
 * This header must be included before any other system header in a
 * translation unit, because it defines _POSIX_C_SOURCE which controls
 * whether features like struct sigaction are visible in <signal.h>.
 */

#if !defined(_POSIX_C_SOURCE)
    #define _POSIX_C_SOURCE 200809L
#endif

#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

typedef struct option    lk_option;
typedef struct sigaction lk_sigaction;
typedef struct stat      lk_stat;
typedef struct tm        lk_tm;
typedef struct timespec  lk_timespec;

#endif
