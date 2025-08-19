#include "RageUtil_StatsFns.h"

float calc_mean(const float* pStart, const float* pEnd)
{
	/* The Kahan summation algorithm is used here to prevent
	 * situations where the low order bits may be lost.
	 * https://en.wikipedia.org/wiki/Kahan_summation_algorithm */

	if (pStart == pEnd)
		return 0.0f;
	
	float sum = 0.0f;
	float c = 0.0f;
	for (const float* p = pStart; p != pEnd; ++p)
	{
		float y = *p - c;
		float t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
	return sum / (pEnd - pStart);
}

float calc_stddev( const float *pStart, const float *pEnd, bool bSample )
{
	/* Calculate the mean. */
	float fMean = calc_mean( pStart, pEnd );

	/* Calculate stddev. */
	float fDev = 0.0f;
	for( const float *i=pStart; i != pEnd; ++i )
		fDev += (*i - fMean) * (*i - fMean);
	fDev /= std::distance( pStart, pEnd ) - (bSample ? 1 : 0);
	fDev = std::sqrt( fDev );

	return fDev;
}

bool CalcLeastSquares( const std::vector<std::pair<float, float>> &vCoordinates,
                       float &fSlope, float &fIntercept, float &fError )
{
	if( vCoordinates.empty() )
		return false;
	float fSumXX = 0.0f, fSumXY = 0.0f, fSumX = 0.0f, fSumY = 0.0f;
	for( unsigned i = 0; i < vCoordinates.size(); ++i )
	{
		fSumXX += vCoordinates[i].first * vCoordinates[i].first;
		fSumXY += vCoordinates[i].first * vCoordinates[i].second;
		fSumX += vCoordinates[i].first;
		fSumY += vCoordinates[i].second;
	}
	const float fDenominator = vCoordinates.size() * fSumXX - fSumX * fSumX;
	fSlope = (vCoordinates.size() * fSumXY - fSumX * fSumY) / fDenominator;
	fIntercept = (fSumXX * fSumY - fSumX * fSumXY) / fDenominator;

	fError = 0.0f;
	for( unsigned i = 0; i < vCoordinates.size(); ++i )
	{
		const float fOneError = fIntercept + fSlope * vCoordinates[i].first - vCoordinates[i].second;
		fError += fOneError * fOneError;
	}
	fError /= vCoordinates.size();
	fError = std::sqrt( fError );
	return true;
}

void FilterHighErrorPoints( std::vector<std::pair<float, float>> &vCoordinates,
                            float fSlope, float fIntercept, float fCutoff )
{
	unsigned int iOut = 0;
	for( unsigned int iIn = 0; iIn < vCoordinates.size(); ++iIn )
	{
		const float fError = fIntercept + fSlope * vCoordinates[iIn].first - vCoordinates[iIn].second;
		if( std::abs(fError) < fCutoff )
		{
			vCoordinates[iOut] = vCoordinates[iIn];
			++iOut;
		}
	}
	vCoordinates.resize( iOut );
}
