#ifndef LIBYALS_H_INCLUDED
#define LIBYALS_H_INCLUDED

/*------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

/*------------------------------------------------------------------------*/

typedef struct Yals Yals;

/*------------------------------------------------------------------------*/

Yals * yals_new ();
void yals_del (Yals *);

/*------------------------------------------------------------------------*/

typedef void * (*YalsMalloc)(void*,size_t);
typedef void * (*YalsRealloc)(void*,void*,size_t,size_t);
typedef void (*YalsFree)(void*,void*,size_t);

Yals * yals_new_with_mem_mgr (void*, YalsMalloc, YalsRealloc, YalsFree);

/*------------------------------------------------------------------------*/

void yals_srand (Yals *, unsigned long long seed);
int yals_setopt (Yals *, const char * name, int val);
void yals_setprefix (Yals *, const char *);
void yals_setout (Yals *, FILE *);
void yals_setphase (Yals *, int lit);
void yals_setflipslimit (Yals *, long long);
void yals_setmemslimit (Yals *, long long);

int yals_getopt (Yals *, const char * name);
void yals_usage (Yals *);
void yals_showopts (Yals *);

/*------------------------------------------------------------------------*/

void yals_add (Yals *, int lit);

int yals_sat (Yals *);

/*------------------------------------------------------------------------*/

long long yals_flips (Yals *);
long long yals_mems (Yals *);

int yals_minimum (Yals *);
int yals_lkhd (Yals *);
int yals_deref (Yals *, int lit);

const int * yals_minlits (Yals *);

/*------------------------------------------------------------------------*/

void yals_stats (Yals *);

/*------------------------------------------------------------------------*/

void yals_seterm (Yals *, int (*term)(void*), void*);

void yals_setime (Yals *, double (*time)(void));

void yals_setmsglock (Yals *,
       void (*lock)(void*), void (*unlock)(void*), void*);

#ifdef YALSTIMING
// detail running time of each operation
// in nanoseconds
struct TimeStats {
  struct timespec total;
  struct timespec flip;
  struct timespec pick;
  struct timespec clause;
  struct timespec connect;
  struct timespec preprocess;
  struct timespec malloc_time;
};

extern struct TimeStats time_stats;

// Helper function declarations
void get_current_time(struct timespec *ts);
void add_timespec(struct timespec *result, const struct timespec *a, const struct timespec *b);
void sub_timespec(struct timespec *result, const struct timespec *a, const struct timespec *b);
double timespec_to_seconds(const struct timespec *ts);

#define TIMING_START(ts) get_current_time(&ts)
#define TIMING_END(start, end, stat) do { \
  get_current_time(&end); \
  sub_timespec(&time_stats.stat, &end, &start); \
} while(0)

#define TIMING_PRINT(stat, total) do { \
  yals_msg(yals, 0, \
    #stat " time %.3f seconds (%.1f%%)", \
    timespec_to_seconds(&time_stats.stat), \
    yals_pct(timespec_to_seconds(&time_stats.stat), timespec_to_seconds(&time_stats.total))); \
} while(0)

#else
#define TIMING_START(ts) ((void)0)
#define TIMING_END(start, end, stat) ((void)0)
#define TIMING_PRINT(stat, total) ((void)0)
#endif

#endif
