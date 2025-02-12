/* RageMath - vector/matrix math utilities. */

#ifndef RAGE_MATH_H
#define RAGE_MATH_H

#include <vector>

constexpr float PI = 3.1415926536f;

struct lua_State;
struct RageVector2;
struct RageVector3;
struct RageVector4;
struct RageMatrix;

void RageVec3ClearBounds( RageVector3 &mins, RageVector3 &maxs );
void RageVec3AddToBounds( const RageVector3 &p, RageVector3 &mins, RageVector3 &maxs );

void RageVec2Normalize( RageVector2* pOut, const RageVector2* pV );
void RageVec3Normalize( RageVector3* pOut, const RageVector3* pV );
void VectorFloatNormalize(std::vector<float>& v);
void RageVec3Cross(RageVector3* ret, RageVector3 const* a, RageVector3 const* b);
void RageVec3TransformCoord( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageVec3TransformNormal( RageVector3* pOut, const RageVector3* pV, const RageMatrix* pM );
void RageVec4TransformCoord( RageVector4* pOut, const RageVector4* pV, const RageMatrix* pM );
void RageMatrixIdentity( RageMatrix* pOut );
// pOut = pB * pA
void RageMatrixMultiply( RageMatrix* pOut, const RageMatrix* pA, const RageMatrix* pB );
void RageMatrixTranslation( RageMatrix* pOut, float x, float y, float z );
void RageMatrixScaling( RageMatrix* pOut, float x, float y, float z );
void RageMatrixSkewX( RageMatrix* pOut, float fAmount );
void RageMatrixSkewY( RageMatrix* pOut, float fAmount );
void RageMatrixTranslate( RageMatrix* pOut, float fTransX, float fTransY, float fTransZ );
void RageMatrixScale( RageMatrix* pOut, float fScaleX, float fScaleY, float fScaleZ );
void RageMatrixRotationX( RageMatrix* pOut, float fTheta );
void RageMatrixRotationY( RageMatrix* pOut, float fTheta );
void RageMatrixRotationZ( RageMatrix* pOut, float fTheta );
void RageMatrixRotationXYZ( RageMatrix* pOut, float rX, float rY, float rZ );
void RageAARotate(RageVector3* inret, RageVector3 const* axis, float angle);
void RageQuatFromHPR(RageVector4* pOut, RageVector3 hpr );
void RageQuatFromPRH(RageVector4* pOut, RageVector3 prh );
void RageMatrixFromQuat( RageMatrix* pOut, const RageVector4 q );
void RageQuatSlerp(RageVector4 *pOut, const RageVector4 &from, const RageVector4 &to, float t);
RageVector4 RageQuatFromH(float theta);
RageVector4 RageQuatFromP(float theta);
RageVector4 RageQuatFromR(float theta);
void RageQuatMultiply( RageVector4* pOut, const RageVector4 &pA, const RageVector4 &pB );
RageMatrix RageLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz );
void RageMatrixAngles( RageMatrix* pOut, const RageVector3 &angles );
void RageMatrixTranspose( RageMatrix* pOut, const RageMatrix* pIn );

float RageSquare( float x) CONST_FUNCTION;
float RageTriangle( float x) CONST_FUNCTION;

class RageQuadratic
{
public:
	void SetFromBezier( float fC1, float fC2, float fC3, float fC4 );
	void GetBezier( float &fC1, float &fC2, float &fC3, float &fC4 ) const;

	void SetFromCubic( float fX1, float fX2, float fX3, float fX4 );

	float Evaluate( float fT ) const;
	float GetSlope( float fT ) const;

	/* Equivalent to Evaluate(0.0f) and Evaluate(1.0f), but faster: */
	float GetBezierStart() const { return m_fD; }
	float GetBezierEnd() const { return m_fA + m_fB + m_fC + m_fD; }

	void PushSelf(lua_State* L);
private:
	float m_fA, m_fB, m_fC, m_fD;
};

class RageBezier2D
{
public:
	void SetFromBezier( float fC1X, float fC2X, float fC3X, float fC4X,
			    float fC1Y, float fC2Y, float fC3Y, float fC4Y );

	void Evaluate( float fT, float *pX, float *pY ) const;
	float EvaluateYFromX( float fX ) const;

	RageQuadratic& get_x() { return m_X; }
	RageQuadratic& get_y() { return m_Y; }
	void PushSelf(lua_State* L);
private:
	RageQuadratic m_X;
	RageQuadratic m_Y;
};

#endif

/*
 * Copyright (c) 2001-2006 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
