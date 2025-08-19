
/**
 * Various math functions, pulled from RageUtil.h
 * Not to be confused with RageMath.h, which is all functions related to RageVector/RageMatrix math  
 */ 

#ifndef RAGEUTIL_MATHFNS_H
#define RAGEUTIL_MATHFNS_H

#include <cmath>
#include <vector>

/**
 * @brief Scales x so that l1 corresponds to l2 and h1 corresponds to h2.
 *
 * This does not modify x, so it MUST assign the result to something!
 * Do the multiply before the divide to that integer scales have more precision.
 *
 * One such example: SCALE(x, 0, 1, L, H); interpolate between L and H.
 */
#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

template<typename T, typename U>
inline U lerp( T x, U l, U h )
{
	return U(x * (h - l) + l);
}

template<typename T, typename U, typename V>
inline bool CLAMP(T& x, U l, V h)
{
	if(x > static_cast<T>(h)) { x= static_cast<T>(h); return true; }
	else if(x < static_cast<T>(l)) { x= static_cast<T>(l); return true; }
	return false;
}

inline void wrap( int &x, int n )
{
	if (x<0)
		x += ((-x/n)+1)*n;
	x %= n;
}
inline void wrap( unsigned &x, unsigned n )
{
	x %= n;
}
inline void wrap( float &x, float n )
{
	if (x<0)
		x += std::trunc(((-x/n)+1))*n;
	x = std::fmod(x,n);
}

inline float fracf( float f ) { return f - std::trunc(f); }

/* return f truncated to the nearest multiple of fTruncInterval */
inline float ftruncf( const float f, const float fTruncInterval )
{
	return int( (f)/fTruncInterval ) * fTruncInterval;
}

/* return f rounded to the nearest multiple of fRoundInterval */
inline float Quantize( const float f, const float fRoundInterval )
{
	return int( (f + fRoundInterval/2)/fRoundInterval ) * fRoundInterval;
}

inline int Quantize( const int i, const int iRoundInterval )
{
	return int( (i + iRoundInterval/2)/iRoundInterval ) * iRoundInterval;
}

/* Return i rounded up to the nearest multiple of iInterval. */
inline int QuantizeUp( int i, int iInterval )
{
	return int( (i+iInterval-1)/iInterval ) * iInterval;
}

inline float QuantizeUp( float i, float iInterval )
{
	return std::ceil( i/iInterval ) * iInterval;
}

/* Return i rounded down to the nearest multiple of iInterval. */
inline int QuantizeDown( int i, int iInterval )
{
	return int( (i-iInterval+1)/iInterval ) * iInterval;
}

inline float QuantizeDown( float i, float iInterval )
{
	return std::floor( i/iInterval ) * iInterval;
}


// Move val toward other_val by to_move.
void fapproach( float& val, float other_val, float to_move );

/* Return a positive x mod y. */
float fmodfp( float x, float y );
/* Round up to the next power of 2 */
int power_of_two( int v );



/* Find the mean and standard deviation of all numbers in [start,end). */
float calc_mean( const float *pStart, const float *pEnd );

/* When bSample is true, it calculates the square root of an unbiased estimator for the population
 * variance. Note that this is not an unbiased estimator for the population standard deviation but
 * it is close and an unbiased estimator is complicated (apparently).
 * When the entire population is known, bSample should be false to calculate the exact standard
 * deviation. */
float calc_stddev( const float *pStart, const float *pEnd, bool bSample = false );

/*
 * Find the slope, intercept, and error of a linear least squares regression
 * of the points given.  Error is returned as the sqrt of the average squared
 * Y distance from the chosen line.
 * Returns true on success, false on failure.
 */
bool CalcLeastSquares( const std::vector<std::pair<float, float> > &vCoordinates,
                       float &fSlope, float &fIntercept, float &fError );

/*
 * This method throws away any points that are more than fCutoff away from
 * the line defined by fSlope and fIntercept.
 */
void FilterHighErrorPoints( std::vector<std::pair<float, float> > &vCoordinates,
                            float fSlope, float fIntercept, float fCutoff );

#endif