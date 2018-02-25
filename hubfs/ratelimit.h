#define SECOND 1000000000

typedef struct Limiter Limiter; /* Tracks time/quantity limits on writes to a hub */

struct Limiter{
	vlong nspb;					/* Minimum nanoseconds per byte */
	vlong sept;					/* Minimum nanoseconds separating messages */
	vlong startt;				/* Start time to calculate intervals from */
	vlong curt;					/* Current time (ns since epoch) */
	vlong lastt;				/* Timestamp of previous message */
	vlong resett;				/* Time after which to reset limit statistics */
	vlong totalbytes;			/* Total bytes written since start time */
	vlong difft;				/* Checks required minimum vs. actual data timing */
	ulong sleept;				/* Milliseconds of sleep time needed to throttle */
};

Limiter* startlimit(vlong nsperbyte, vlong nsmingap, vlong nstoreset);
void limit(Limiter *lp, vlong bytes);
