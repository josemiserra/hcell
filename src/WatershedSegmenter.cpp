#include "WatershedSegmenter.h"



double WatershedSegmenter::check_multiple( vector<double> &tgt, vector<double> &src, int & ind, IntList & nb, SeedList & seeds, double & tolerance, int & nx, int & ny ) {
    
	if ( nb.size() == 1 ) return nb.front();
    if ( nb.size() <  1 ) return 0.0; // dumb protection

    double diff, maxdiff = 0.0, res = 0.0;
    int i;
    IntList::iterator  it;
    SeedList::iterator sit;
    Point ptsit, pt = pointFromIndex( ind, nx );
    double distx, dist = FLT_MAX;

    /* maxdiff */
    for ( it = nb.begin(); it != nb.end(); it++ ) {
        if ( !get_seed( seeds, *it, sit ) ) continue;
        diff = fabs( src[ ind ] - src[ (*sit).index ] );
        if ( diff > maxdiff ) {
            maxdiff = diff;
            /* assign result to the steepest until and if it not assigned to closest over the tolerance */
            if ( dist == FLT_MAX )
                res = *it;
        }
        /* we assign to the closest centre which is above tolerance, if none
		than to maxdiff */
        if ( diff >= tolerance ) {
            ptsit = pointFromIndex( (*sit).index, nx );
            distx = distanceXY( pt, ptsit);
            if ( distx < dist ) {
                dist =  distx;
                res = * it;
            }
        }

    }
    /* assign all that need assignment to res, which has maxdiff */
    for ( it = nb.begin(); it != nb.end(); it++ ) {
        if ( *it == res ) continue;
        if ( !get_seed( seeds, *it, sit ) ) continue;
        if ( fabs( src[ ind ] - src[ (*sit).index ] ) >= tolerance ) continue;
        for ( i = 0; i < nx * ny; i++ )
            if ( tgt[ i ] == *it )
                tgt[ i ] = res;
        seeds.erase( sit );
    }
    return res;
}