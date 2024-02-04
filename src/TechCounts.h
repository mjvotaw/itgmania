#ifndef TECH_COUNTS_H
#define TECH_COUNTS_H

#include "GameConstantsAndTypes.h"
class NoteData;

enum Foot
{
	Foot_None = -1,
	Foot_Left = 0,
	Foot_Right
};

/** @brief Unknown radar values are given a default value. */
#define TECHCOUNTS_VAL_UNKNOWN -1

enum TechCountsCategory
{
	TechCountsCategory_Crossovers = 0,
	TechCountsCategory_Footswitches,
	TechCountsCategory_Sideswitches,
	TechCountsCategory_Jacks,
	TechCountsCategory_Brackets,
	TechCountsCategory_Doublesteps,
	NUM_TechCountsCategory,
	TechCountsCategory_Invalid
};

const RString& TechCountsCategoryToString( TechCountsCategory cat );
/**
 * @brief Turn the radar category into a proper localized string.
 * @param cat the radar category.
 * @return the localized string version of the radar category.
 */
const RString& TechCountsCategoryToLocalizedString( TechCountsCategory cat );
LuaDeclareType( TechCountsCategory );


// This could probably be an enum?
typedef int StepDirection;
const StepDirection StepDirection_None = 0;
const StepDirection StepDirection_Left = 1;
const StepDirection StepDirection_Down = 2;
const StepDirection StepDirection_Up = 4;
const StepDirection StepDirection_Right = 8;

const StepDirection StepDirection_LR = StepDirection_Left | StepDirection_Right;
const StepDirection StepDirection_UD = StepDirection_Up | StepDirection_Down;
const StepDirection StepDirection_LD = StepDirection_Left | StepDirection_Down;
const StepDirection StepDirection_LU = StepDirection_Left | StepDirection_Up;
const StepDirection StepDirection_RD = StepDirection_Right | StepDirection_Down;
const StepDirection StepDirection_RU = StepDirection_Right | StepDirection_Up;

/** @brief Converts a track index into a StepDirection. Note that it only supports 4-panel play at the moment. */
inline int TrackIntToStepDirection(int track)
{
	return ((track > 3 || track < 0) ? -1 : pow(2, track) );
}

/** @brief convenience method for checking if the given StepDirection is a single arrow */
inline bool IsSingleStep(StepDirection s)
{
	return ((s == StepDirection_Left) || (s == StepDirection_Down) || (s == StepDirection_Up) || (s == StepDirection_Right));
}

/** @brief convenience method for checking if the given StepDirection is a jump */
inline bool IsJump(StepDirection s)
{
	return ( (s == StepDirection_LR) || (s == StepDirection_UD) || (s == StepDirection_LD) || (s == StepDirection_LU) || (s == StepDirection_RD) || (s == StepDirection_RU) );
}

/** @brief convenience method for checking if `instep` contains `step`. 
 * Examples:
 * instep = StepDirection_LR, step = StepDirection_Left => true
 * instep = StepDirection_Left, step = StepDirection_LR => false
 * instep = StepDirectio_LR, step = StepDirection_LU => false
*/
inline bool StepContainsStep(StepDirection instep, StepDirection step)
{
	return (instep & step) == step;
}

/** @brief convenience method for "switching" the value for the given Foot. If f == Foot_None, this will return Foot_Right. */
inline Foot SwitchFeet(Foot f) 
{
	return ((f == Foot_None || f == Foot_Left) ? Foot_Right : Foot_Left);
}

struct lua_State;

/** @brief Technical statistics */
struct TechCounts
{
private:
	float m_Values[NUM_TechCountsCategory];
public:

	float operator[](TechCountsCategory cat) const { return m_Values[cat]; }
	float& operator[](TechCountsCategory cat) { return m_Values[cat]; }
	float operator[](int cat) const { return m_Values[cat]; }
	float& operator[](int cat) { return m_Values[cat]; }
	TechCounts();
	void MakeUnknown();
	void Zero();

	TechCounts& operator+=( const TechCounts& other )
	{
		FOREACH_ENUM( TechCountsCategory, tc )
		{
			(*this)[tc] += other[tc];
		}
		return *this;
	}

	bool operator==( const TechCounts& other ) const
	{
		FOREACH_ENUM( TechCountsCategory, tc )
		{
			if((*this)[tc] != other[tc])
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=( const TechCounts& other ) const
	{
		return !operator==( other );
	}

	RString ToString( int iMaxValues = -1 ) const; // default = all
	void FromString( RString sValues );

	void PushSelf( lua_State *L );
};

/** @brief a counter used to keep track of state while parsing data in TechCountsCalculator::CalculateTechCounts*/
struct TechCountsCounter
{

	bool wasLastStreamFlipped;
	bool anyStepsSinceLastCommitStream;
	bool justBracketed;
	bool lastFlip;

	std::vector<bool> stepsLr;

	Foot lastFoot;
	Foot trueLastFoot;

	StepDirection lastStep;
	StepDirection lastRepeatedFoot;
	StepDirection lastArrowL;
	StepDirection lastArrowR;

	StepDirection trueLastArrowL;
	StepDirection trueLastArrowR;

	int recursionCount;
	TechCountsCounter()
	{
		lastFoot = Foot_Left;
		wasLastStreamFlipped = false;
		lastStep = StepDirection_None;
		lastRepeatedFoot = StepDirection_None;

		anyStepsSinceLastCommitStream = false;
		
		lastArrowL = StepDirection_None;
		lastArrowR = StepDirection_None;
		
		trueLastArrowL = StepDirection_None;
		trueLastArrowR = StepDirection_None;

		trueLastFoot = Foot_None;
		justBracketed = false;
		lastFlip = false;

		recursionCount = 0;
	}
};


namespace TechCountsCalculator
{
	void CalculateTechCounts(const NoteData &in, TechCounts &out);
	void UpdateTechCounts(TechCounts &stats, TechCountsCounter &counter, StepDirection currentStep);
	void CommitStream(TechCounts &stats, TechCountsCounter &counter, Foot tieBreaker);
};

#endif

/**
 * @file
 * @author Michael Votaw (c) 2023
 * @section LICENSE
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
