/* RageSoundMixBuffer - Simple audio mixing. */

#ifndef RAGE_SOUND_MIX_BUFFER_H
#define RAGE_SOUND_MIX_BUFFER_H

#include <cstdint>

class RageSoundMixBuffer
{
public:
	RageSoundMixBuffer();
	~RageSoundMixBuffer();

	// See how many samples we can stuff into 2MB.
	static constexpr size_t BUF_SIZE = 2 * 1024 * 1024 / sizeof(float);

	// Mix the given buffer of samples.
	void write( const float *pBuf, unsigned iSize, int iSourceStride = 1, int iDestStride = 1 );

	// Extend the buffer as if write() was called with a buffer of silence.
	void Extend( unsigned iSamples );

	void read( std::int16_t *pBuf );
	void read( float *pBuf );
	void read_deinterlace( float **pBufs, int channels );
	float *read() { return m_pMixbuf; }
	unsigned size() const { return m_iBufUsed; }
	void SetWriteOffset( int iOffset );

private:
	float *m_pMixbuf;
	std::uint64_t m_iBufSize; // actual allocated samples
	std::uint64_t m_iBufUsed; // used samples
	int m_iOffset;
};

#endif

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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
