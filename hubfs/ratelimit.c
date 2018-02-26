#include <u.h>
#include <libc.h>
#include "ratelimit.h"

/*
 * Limits can be set in terms of bytes-per-second,
 * minimum permitted interval between writes, or both.
 * Limit tracking resets after a specified interval, default 60 sec.
*/

/* startlimit initializes the Limiter structure and returns a pointer to it */
Limiter*
startlimit(vlong nsperbyte, vlong nsmingap, vlong nstoreset)
{
	Limiter *limiter;

	limiter=(Limiter*)malloc(sizeof(Limiter));
	if(!limiter)
		sysfatal("out of memory");
	limiter->nspb = nsperbyte;
	limiter->sept = nsmingap;
	limiter->resett = nstoreset;
	limiter->startt = 0;
	limiter->lastt = 0;
	limiter->curt = 0;
	return limiter;
}

/* limit is called whenever a write happens */
void
limit(Limiter *lp, vlong bytes)
{
	lp->curt = nsec();
	lp->totalbytes += bytes;
	/* initialize timers if this is the first message written to a hub */
	if(lp->startt == 0){
		lp->startt = lp->curt;
		lp->lastt = lp->curt;
		return;
	}
	/* check if the message has arrived before the minimum interval */
	if(lp->curt - lp->lastt < lp->sept){
		lp->sleept = (lp->sept - (lp->curt - lp->lastt)) / 1000000;
		sleep(lp->sleept);
		lp->lastt = nsec();
		return;
	}
	/* reset timer if the interval between messages is sufficient */
	if(lp->curt - lp->lastt > lp->resett){
		lp->startt = lp->curt;
		lp->lastt = lp->curt;
		lp->totalbytes = bytes;
		return;
	}
	/* check the required elapsed time vs actual elapsed time */
	lp->difft = (lp->nspb * lp->totalbytes) - (lp->curt - lp->startt);
	if(lp->difft > 1000000){
		lp->sleept = lp->difft / 1000000;
		sleep(lp->sleept);
	}
	lp->lastt = nsec();
}
