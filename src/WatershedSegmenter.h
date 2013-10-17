#ifndef _WATERSHED_SEGMENTER_
#define _WATERSHED_SEGMENTER_
#include "opencv2/opencv.hpp"
#include <string>
#include <list>
#include <utility>
#include "utils.h"

#define BG 0.0

using namespace cv;
using namespace std;
#define IJK(i,j) ((i)*nx+(j))


#ifdef __linux__
	#if GCC_VERSION < 40600

	struct ordering {
		bool operator ()(pair<size_t, double> const& a, pair<size_t, double> const& b) {
        return (a.second) < (b.second);
		}
	};
	#else
	template <typename T>
	vector<int> sorted_indexes(const vector<T> &v)
	{

	// initialize original index locations
	vector<int> idx(v.size());
	for (unsigned int i = 0; i != idx.size(); ++i) idx[i] = i;


	sort(idx.begin(), idx.end(),
	    [&v](int i1, int i2) {return v[i1] < v[i2];});

	return idx;
	}
	#endif
#else
	template <typename T>
	vector<int> sorted_indexes(const vector<T> &v)
	{

	// initialize original index locations
	vector<int> idx(v.size());
	for (unsigned int i = 0; i != idx.size(); ++i) idx[i] = i;

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
    [&v](int i1, int i2) {return v[i1] < v[i2];});

	return idx;
	}
#endif

class WatershedSegmenter{
private:
    cv::Mat markers;

public:
	/* list of STL, C++ */
	struct TheSeed
	{
    int index, seed;
	};

	typedef std::list<int>     IntList;
	typedef std::list<TheSeed> SeedList;

	double  check_multiple( vector<double> &tgt, vector<double> &src, int & ind, IntList & nb, SeedList & seeds, double & tolerance, int & nx, int & ny );

	/*----------------------------------------------------------------------- */
	Point  pointFromIndex (const int index, const int xsize)
	{
		 Point res;
		 res.y = floor ((float) index / xsize);
		 res.x = index - res.y * xsize;
		 return res;
	}

/*----------------------------------------------------------------------- */
#ifdef __linux__
	#if GCC_VERSION < 40600
	vector<int> sorted_indexes(const vector<double> &v)
	{
		 vector<pair<size_t,double>>order(v.size());

		size_t n = 0;
		for (vector<double>::const_iterator it = v.begin(); it != v.end(); ++it, ++n)
			order[n] = make_pair(n, *it);

		sort(order.begin(), order.end(), ordering());

		vector<int> ret(v.size());
		size_t const size = v.size();
		for (size_t i = 0; i < size; ++i)
		        ret[i] = order[i].first;

		return ret;

		}
	#endif
#endif


/******************************************************************************************************/
    void setMarkers(cv::Mat& markerImage)
    {
        markerImage.convertTo(markers, CV_32SC1);
    }

    cv::Mat process(cv::Mat &image)
    {
        cv::watershed(image, markers);
        markers.convertTo(markers,CV_8U);
        return markers;
    }

/******************************************************************************************************/

	void watershed_nuc (Mat &input, Mat &ref, vloP &output, double tolerance,int ext) {
	
	Mat ref2;
    int  i, j, nx, ny, _ext;
    double _tolerance = 0.0;
	vector<double> tgt,src;
	vector<int> indexes;
	nx = input.cols;
	ny = input.rows;
 
    ut::Trace tr = ut::Trace("split by watershed::watershed_nuc",__FILE__);
	double maxVal;
	
	_tolerance =  tolerance;
    _ext = ext;

	float val;
	MatIterator_<float> it1, end;
    for( it1 = input.begin<float>(), end = input.end<float>(); it1 != end; ++it1)
	{
		val = *it1;
		src.push_back(val);
		tgt.push_back(-val);
	}

	indexes = sorted_indexes(tgt);

    SeedList seeds;  /* indexes of all seed starting points, i.e. lowest values */
	IntList  equals; /* queue of all pixels on the same gray level */
    IntList  nb;     /* seed values of assigned neighbours */
     
	int ind, indxy, nbseed, x, y, topseed = 0;
    IntList::iterator it;
    TheSeed newseed;
     
	 Point pt;
     bool isin;
     
	 /* loop through the sorted index */
     for ( i = 0; i < nx * ny && src[ indexes[i] ] > BG; ) {   //
            /* pool a queue of equally lowest values */
            ind = indexes[ i ];
            equals.push_back( ind );
            for ( i = i + 1; i < nx * ny; ) { // store all the values with the same index value
                if ( src[ indexes[i] ] != src[ ind ] ) break;
                equals.push_back( indexes[i] );
                i++;
            }
            while ( !equals.empty() ) {
                /* first check through all the pixels if we can assign them to
                 * existing objects, count checked and reset counter on each assigned
                 * -- exit when counter equals queue length */
                for ( j = 0; j < (int) equals.size(); ) {
                    ind = equals.front();
                    equals.pop_front();
                    /* check neighbours:
                     * - if exists one, assign
                     * - if two or more check what should be combined and assign to the steepest
                     * - if none, push back */
                    /* reset j to 0 every time we assign another pixel to restart the loop */
                    nb.clear();
                    pt = pointFromIndex( ind, nx );
                    /* determine which neighbour we have, push them to nb */
                    for ( x = pt.x - ext; x <= pt.x + ext; x++ )
                        for ( y = pt.y - ext; y <= pt.y + ext; y++ ) {
                            if ( x < 0 || y < 0 || x >= nx || y >= ny || (x == pt.x && y == pt.y) ) continue;
                            indxy = x + y * nx;
                            nbseed = (int) tgt[ indxy ];
                            if ( nbseed < 1 ) continue;
                            isin = false;
                            for ( it = nb.begin(); it != nb.end() && !isin; it++ )
                                if ( nbseed == *it ) isin = true;
                            if ( !isin ) nb.push_back( nbseed );
                        }
                    if ( nb.size() == 0 ) {
                        /* push the pixel back and continue with the next one */
                        equals.push_back( ind );
                        j++;
                        continue;
                    }
                    tgt[ ind ] = check_multiple(tgt, src, ind, nb, seeds, tolerance, nx, ny );
                    /* we assigned the pixel, reset j to restart neighbours detection */
                    j = 0;
                }
                /* now we have assigned all that we could */
                if ( !equals.empty() ) {
                    /* create a new seed for one pixel only and go back to assigning neighbours */
                    topseed++;
                    newseed.index = equals.front();
                    newseed.seed = topseed;
                    equals.pop_front();
                    tgt[ newseed.index ] = topseed;
                    seeds.push_back( newseed );
                }
            } // assigning equals
        } // sorted index

        /* now we need to reassign indexes while some seeds could be removed */
        double * finseed = new double[ topseed ];
        for ( i = 0; i < topseed; i++ )
            finseed[ i ] = 0;
        i = 0;
        while ( !seeds.empty() ) {
            newseed = seeds.front();
            seeds.pop_front();
            finseed[ newseed.seed - 1 ] = i + 1;
            i++;
        }
        for ( i = 0; i < nx * ny; i++ ) {
            j = (int) tgt[ i ];
            if ( 0 < j && j <= topseed )
                tgt[ i ] = finseed[ j - 1 ];
        }
        delete[] finseed;

		 maxVal = *std::max_element(tgt.begin(),tgt.end());
		
		for ( i = 0; i < (int) maxVal; i++ ) {  output.push_back(loP()); }

		for(int j = 0; j < nx; j++)
		{
		 for(int i = 0; i < ny; i++)
		 {	
			 if(tgt[IJK(i,j)]!=0)
			 {
				
				 output[tgt[IJK(i,j)]-1].push_back(Point(j,i));
			 }
		 }
		} 


    } 
	


bool	get_seed(SeedList &seeds, int & seed, SeedList::iterator & sit ) 
{
		for ( sit = seeds.begin(); sit != seeds.end(); sit++ )
        if ( (*sit).seed == seed ) return true;
		return false;
}
/*----------------------------------------------------------------------- */
double
distancexy (int x1, int y1, int x2, int y2) {
    return sqrt ( (long double)( (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) ) );
}


double
distanceXY (const Point pt1, const Point pt2) {
    return sqrt ( (long double)( (pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y) ) );
}

};

#endif