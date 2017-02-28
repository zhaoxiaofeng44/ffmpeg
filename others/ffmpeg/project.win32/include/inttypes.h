#ifndef TYPE_INTTYPES

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#define PRIx64 
#define PRIu32 
#define PRId64 
#define PRIX64 
#define inline __inline

#endif

