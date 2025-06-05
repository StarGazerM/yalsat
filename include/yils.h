#ifndef YILS_H_INCLUDED
#define YILS_H_INCLUDED

#ifndef YALSINTERNAL
// #error "this file is internal to 'libyals'"
#endif

/*------------------------------------------------------------------------*/

#include "yals.h"
#include "gpu/gpu.h"

/*------------------------------------------------------------------------*/

#include <stdlib.h>

/*------------------------------------------------------------------------*/

#ifndef NDEBUG
void yals_logging (Yals *, int logging);
void yals_checking (Yals *, int checking);
#endif

/*------------------------------------------------------------------------*/

void yals_abort (Yals *, const char * fmt, ...);
void yals_exit (Yals *, int exit_code, const char * fmt, ...);
void yals_msg (Yals *, int level, const char * fmt, ...);

const char * yals_default_prefix (void);
const char * yals_version ();
void yals_banner (const char * prefix);

/*------------------------------------------------------------------------*/

double yals_process_time ();				// process time

double yals_sec (Yals *);				// time in 'yals_sat'
size_t yals_max_allocated (Yals *);		// max allocated bytes

/*------------------------------------------------------------------------*/

void * yals_malloc (Yals *, size_t);
void yals_free (Yals *, void*, size_t);
void * yals_realloc (Yals *, void*, size_t, size_t);

/*------------------------------------------------------------------------*/

void yals_srand (Yals *, unsigned long long);
  
/*------------------------------------------------------------------------*/

extern const char * yals_bright_red_color_code;
extern const char * yals_normal_color_code;
extern const char * yals_bold_color_code;


/*------------------------------------------------------------------------*/

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(__linux__)
#include <fpu_control.h>	// Set FPU to double precision on Linux.
#endif

/*------------------------------------------------------------------------*/

#define YALS_INT64_MAX		(0x7fffffffffffffffll)
#define YALS_DEFAULT_PREFIX	"c "
/*------------------------------------------------------------------------*/

#define NEWN(P,N) \
  do { (P) = yals_malloc (yals, (N)*sizeof *(P)); } while (0)

#define DELN(P,N) \
  do { yals_free (yals, (P), (N)*sizeof *(P)); } while (0)

#define RSZ(P,O,N) \
do { \
  (P) = yals_realloc (yals, (P), (O)*sizeof *(P), (N)*sizeof *(P)); \
} while (0)

/*------------------------------------------------------------------------*/

#define STACK(T) \
  struct { T * start; T * top; T * end; }

#define INIT(S) \
  do { (S).start = (S).top = (S).end = 0; } while (0)

#define SIZE(S) \
  ((S).end - (S).start)

#define COUNT(S) \
  ((S).top - (S).start)

#define EMPTY(S) \
  ((S).top == (S).start)

#define FULL(S) \
  ((S).top == (S).end)

#define CLEAR(S) \
  do { (S).top = (S).start; } while (0)

#define ENLARGE(S) \
do { \
  size_t OS = SIZE (S); \
  size_t OC = COUNT (S); \
  size_t NS = OS ? 2*OS : 1; \
  assert (OC <= OS); \
  RSZ ((S).start, OS, NS); \
  (S).top = (S).start + OC; \
  (S).end = (S).start + NS; \
} while (0)

#define FIT(S) \
do { \
  size_t OS = SIZE (S); \
  size_t NS = COUNT (S); \
  RSZ ((S).start, OS, NS); \
  (S).top = (S).start + NS; \
  (S).end = (S).start + NS; \
} while (0)

#define RESET(S,N) \
do { \
  assert ((N) <= SIZE (S) ); \
  (S).top = (S).start + (N); \
} while (0)

#define PUSH(S,E) \
do { \
  if (FULL(S)) ENLARGE (S); \
  *(S).top++ = (E); \
} while (0)

#define POP(S) \
  (assert (!EMPTY (S)), *--(S).top)

#define TOP(S) \
  (assert (!EMPTY (S)), (S).top[-1])

#define PEEK(S,P) \
  (assert ((P) < COUNT(S)), (S).start[(P)])

#define POKE(S,P,E) \
  do { assert ((P) < COUNT(S)); (S).start[(P)] = (E); } while (0)

#define RELEASE(S) \
do { \
  size_t N = SIZE (S); \
  DELN ((S).start, N); \
  INIT (S); \
} while (0)

/*------------------------------------------------------------------------*/

typedef unsigned Word;

#define LD_BITS_PER_WORD	5
#define BITS_PER_WORD		(8*sizeof (Word))
#define BITMAPMASK		(BITS_PER_WORD - 1)

#define WORD(BITS,N,IDX) \
  ((BITS)[ \
    assert ((IDX) >= 0), \
    assert (((IDX) >> LD_BITS_PER_WORD) < (N)), \
    ((IDX) >> LD_BITS_PER_WORD)])

#define BIT(IDX) \
  (((Word)1u) << ((IDX) & BITMAPMASK))

#define GETBIT(BITS,N,IDX) \
  (WORD(BITS,N,IDX) & BIT(IDX))

#define SETBIT(BITS,N,IDX) \
  do { WORD(BITS,N,IDX) |= BIT(IDX); } while (0)

#define CLRBIT(BITS,N,IDX) \
  do { WORD(BITS,N,IDX) &= ~BIT(IDX); } while (0)

#define NOTBIT(BITS,N,IDX) \
  do { WORD(BITS,N,IDX) ^= BIT(IDX); } while (0)

/*------------------------------------------------------------------------*/

#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))
#define ABS(A) (((A) < 0) ? (assert ((A) != INT_MIN), -(A)) : (A))

#define SWAP(T,A,B) \
  do { T TMP = (A); (A) = (B); (B) = (TMP); } while (0)

/*------------------------------------------------------------------------*/
#ifndef NDEBUG
#define LOG(ARGS...) \
do { \
  if (!yals->opts.logging.val) break; \
  yals_log_start (yals, ##ARGS); \
  yals_log_end (yals); \
} while (0)
#define LOGLITS(LITS,ARGS...) \
do { \
  const int * P; \
  if (!yals->opts.logging.val) break; \
  yals_log_start (yals, ##ARGS); \
  fprintf (yals->out, " clause :"); \
  for (P = (LITS); *P; P++) \
    fprintf (yals->out, " %d", *P); \
  yals_log_end (yals); \
} while (0)
#define LOGCIDX(CIDX,ARGS...) \
do { \
  const int * P, * LITS = yals_lits (yals, (CIDX)); \
  if (!yals->opts.logging.val) break; \
  yals_log_start (yals, ##ARGS); \
  fprintf (yals->out, " clause %d :", (CIDX)); \
  for (P = (LITS); *P; P++) \
    fprintf (yals->out, " %d", *P); \
  yals_log_end (yals); \
} while (0)
#else
#define LOG(ARGS...) do { } while (0)
#define LOGLITS(ARGS...) do { } while (0)
#define LOGCIDX(ARGS...) do { } while (0)
#endif
/*------------------------------------------------------------------------*/

#define LENSHIFT	6
#define MAXLEN		((1<<LENSHIFT)-1)
#define LENMASK		MAXLEN

/*------------------------------------------------------------------------*/

enum ClausePicking {
  PSEUDO_BFS_CLAUSE_PICKING = -1,
  RANDOM_CLAUSE_PICKING = 0,
  BFS_CLAUSE_PICKING = 1,
  DFS_CLAUSE_PICKING = 2,
  RELAXED_BFS_CLAUSE_PICKING = 3,
  UNFAIR_BFS_CLAUSE_PICKING = 4,
};

/*------------------------------------------------------------------------*/

#define OPTSTEMPLATE \
  OPT (best,0,0,1,"always pick best assignment during restart"); \
  OPT (cached,1,0,1,"use cached assignment during restart"); \
  OPT (cacheduni,0,0,1,"pick random cached assignment uniformly"); \
  OPT (cachemax,(1<<10),0,(1<<20),"max cache size of saved assignments"); \
  OPT (cachemin,1,0,(1<<10),"minimum cache size of saved assignments"); \
  OPT (cb,0,0,1000,"explicit CB value in percent (< 100 disabled)"); \
  OPT (correct,0,0,1,"correct CB value depending on maximum length"); \
  OPT (crit,1,0,1,"dynamic break values (using critical lits)"); \
  OPT (defrag,1,0,1,"defragemtation of unsat queue"); \
  OPT (eager,0,0,1,"eagerly pick minimum break literals"); \
  OPT (fixed,4,0,INT_MAX,"fixed default strategy frequency (1=always)"); \
  OPT (geomfreq,66,0,100,"geometric picking first frequency (percent)"); \
  OPT (hitlim,-1,-1,INT_MAX,"minimum hit limit"); \
  OPT (keep,0,0,1,"keep assignment during restart"); \
  OPT (minchunksize,(1<<8),2,(1<<16),"minium queue chunk size"); \
  OPT (pick,4,-1,4,"-1=pbfs,0=rnd,1=bfs,2=dfs,3=rbfs,4=ubfs"); \
  OPT (pol,0,-1,1,"negative=-1 positive=1 or random=0 polarity"); \
  OPT (prep,1,0,1,"preprocessing through unit propagation"); \
  OPT (rbfsrate,10,1,INT_MAX,"relaxed BFS rate"); \
  OPT (reluctant,1,0,1,"reluctant doubling of restart interval"); \
  OPT (restart,100000,0,INT_MAX,"basic (inner) restart interval"); \
  OPT (restartouter,0,0,1,"enable restart outer"); \
  OPT (restartouterfactor,100,1,INT_MAX,"outer restart interval factor"); \
  OPT (setfpu,1,0,1,"set FPU to use double precision on Linux"); \
  OPT (target,0,0,INT_MAX,"unsatisfied clause target"); \
  OPT (termint,1000,0,INT_MAX,"termination call back check interval"); \
  OPT (toggleuniform,0,0,1,"toggle uniform strategy"); \
  OPT (unfairfreq,50,0,100,"unfair picking first frequency (percent)"); \
  OPT (uni,0,-1,1,"weighted=0,uni=1,antiuni=-1 clause weights"); \
  OPT (unipick,-1,-1,4,"clause picking strategy for uniform formulas"); \
  OPT (unirestarts,0,0,INT_MAX,"max number restarts for uniform formulas"); \
  OPT (verbose,0,0,2,"set verbose level"); \
  OPT (walk,0,0,1,"enable random walks"); \
  OPT (walkprobability,50,1,100,"random walk probability (in percent)"); \
  OPT (weight,5,1,7,"maximum clause weight"); \
  OPT (witness,1,0,1,"print witness"); \
  OPTSTEMPLATENDEBUG

#ifndef NDEBUG
#define OPTSTEMPLATENDEBUG \
  OPT (logging, 0, 0, 1, "set logging level"); \
  OPT (checking, 0, 0, 1, "set checking level");
#else
#define OPTSTEMPLATENDEBUG
#endif

#define OPT(NAME,DEFAULT,MIN,MAX,DESCRIPTION) Opt NAME

/*------------------------------------------------------------------------*/

#define STRATSTEMPLATE \
  STRAT (cached,1); \
  STRAT (correct,1); \
  STRAT (eager,1); \
  STRAT (pol,1); \
  STRAT (uni,1); \
  STRAT (walk,1); \
  STRAT (weight,1);

#define STRAT(NAME,ENABLED) int NAME

/*------------------------------------------------------------------------*/

#ifndef NYALSMEMS
#define ADD(NAME,NUM) \
do { \
  yals->stats.mems.all += (NUM); \
  yals->stats.mems.NAME += (NUM); \
} while (0)
#else
#define ADD(NAME,NUM) do { } while (0)
#endif

#define INC(NAME) ADD (NAME, 1)

/*------------------------------------------------------------------------*/

#define assert_valid_occs(OCCS) \
  do { assert (0 <= OCCS), assert (OCCS < yals->noccs); } while (0)

#define assert_valid_idx(IDX) \
  do { assert (0 <= IDX), assert (IDX < yals->nvars); } while (0)

#define assert_valid_cidx(CIDX) \
  do { assert (0 <= CIDX), assert (CIDX < yals->nclauses); } while (0)

#define assert_valid_len(LEN) \
  do { assert (0 <= LEN), assert (LEN <= MAXLEN); } while (0)

#define assert_valid_pos(POS) \
  do { \
    assert (0 <= POS), assert (POS < COUNT (yals->unsat.stack)); \
} while (0)

/*------------------------------------------------------------------------*/

typedef struct RDS { unsigned u, v; } RDS;

typedef struct RNG { uint64_t state; } RNG;

typedef struct Mem {
  void * mgr;
  YalsMalloc malloc;
  YalsRealloc realloc;
  YalsFree free;
} Mem;

typedef struct Strat { STRATSTEMPLATE } Strat;

typedef struct Stats {
  int best, worst, last, tmp, maxstacksize;
  int64_t flips, eagerflips, walks, hits, unsum;
  struct {
    struct { int64_t count; } outer;
    struct { int64_t count, maxint; } inner;
  } restart;
  struct { struct { int chunks, lnks; } max; int64_t unfair; } queue;
  struct { int64_t inserted, replaced, skipped; } cache;
  struct { int64_t search, neg, falsepos, truepos; } sig;
  struct { int64_t def, rnd; } strat;
  struct { int64_t best, cached, keep, pos, neg, rnd; } pick;
  struct { int64_t count, moved; } defrag;
  struct { size_t current, max; } allocated;
  struct { volatile double total, defrag, restart, entered; } time;
#ifdef __GNUC__
  volatile int flushing_time;
#endif
#ifndef NYALSMEMS
  struct { long long all, crit, lits, occs, read, update, weight; } mems;
#endif
#ifndef NYALSTATS
  int64_t * inc, * dec, broken, made; int nincdec;
  struct { unsigned min, max; } wb;
#endif
} Stats;

typedef struct Limits {
#ifndef NYALSMEMS
  int64_t mems;
#endif
  int64_t flips;
  struct {
    struct { int64_t lim, interval; } outer;
    struct { int64_t lim; union { int64_t interval; RDS rds; }; } inner;
  } restart;
  struct { int min; } report;
  int term;
} Limits;

typedef struct Lnk {
  int cidx;
  struct Lnk * prev, * next;
} Lnk;

typedef union Chunk {
  struct { int size; union Chunk * next; };
  Lnk lnks[1];					// actually of 'size'
} Chunk;

typedef struct Queue {
  int count, chunksize, nchunks, nlnks, nfree;
  Lnk * first, * last, * free;
  Chunk * chunks;
} Queue;

typedef struct Exp {
  struct { STACK(double) two, cb; } table;
  struct { unsigned two, cb; } max;
  struct { double two, cb; } eps;
} Exp;

typedef struct Opt { int val, def, min, max; } Opt;

typedef struct Opts { char * prefix; OPTSTEMPLATE } Opts;

typedef struct Callbacks {
  double (*time)(void);
  struct { void * state; int (*fun)(void*); } term;
  struct { void * state; void (*lock)(void*); void (*unlock)(void*); } msg;
} Callbacks;

typedef unsigned char U1;
typedef unsigned short U2;
typedef unsigned int U4;

typedef struct FPU {
#ifdef __linux__
  fpu_control_t control;
#endif
  int saved;
} FPU;

struct Yals {
  RNG rng;
  FILE * out;
  struct { int out, err; } colored;
  struct { int usequeue; Queue queue; STACK(int) stack; } unsat;
  int nvars, * refs; int64_t * flips;
  STACK(signed char) mark;
  int trivial, mt, uniform, pick;
  Word * vals, * best, * tmp, * clear, * set; int nvarwords;
  STACK(int) cdb, trail, phases, clause, mins; 
  int satcntbytes; union { U1 * satcnt1; U2 * satcnt2; U4 * satcnt4; };
  int * occs, noccs; unsigned * weights;
  uint32_t * occs_bucket_sizes;
  int * pos, * lits; Lnk ** lnk;
  int * crit; unsigned * weightedbreak;
  int nclauses, nbin, ntrn, minlen, maxlen; double avglen;
  STACK(unsigned) breaks; STACK(double) scores; STACK(int) cands;
  STACK(Word*) cache; int cachesizetarget; STACK(Word) sigs;
  STACK(int) minlits;
  Callbacks cbs;
  Limits limits;
  Strat strat;
  Stats stats;
  Opts opts;
  Mem mem;
  FPU fpu;
  Exp exp;

  // GPU support
  GPUMemoryManager * gpu_memory_manager;
  Yalsg * yalsg;
};


#endif
