#include "global.h"
#include "TechCounts.h"
#include "NoteData.h"
#include "RageLog.h"
#include "LocalizedString.h"
#include "LuaBinding.h"
#include "TimingData.h"
#include "GameState.h"
#include "RageTimer.h"


static const char *TechCountsCategoryNames[] = {
	"Crossovers",
	"Footswitches",
	"Sideswitches",
	"Jacks",
	"Brackets"
};

XToString( TechCountsCategory );
XToLocalizedString( TechCountsCategory );
LuaFunction(TechCountsCategoryToLocalizedString, TechCountsCategoryToLocalizedString(Enum::Check<TechCountsCategory>(L, 1)) );
LuaXType( TechCountsCategory );

// TechCounts methods

TechCounts::TechCounts()
{
	MakeUnknown();
}

void TechCounts::MakeUnknown()
{
	FOREACH_ENUM( TechCountsCategory, rc )
	{
		(*this)[rc] = TECHCOUNTS_VAL_UNKNOWN;
	}
}

void TechCounts::Zero()
{
	FOREACH_ENUM( TechCountsCategory, rc )
	{
		(*this)[rc] = 0;
	}
}

RString TechCounts::ToString( int iMaxValues ) const
{
	if( iMaxValues == -1 )
		iMaxValues = NUM_TechCountsCategory;
	iMaxValues = std::min( iMaxValues, (int)NUM_TechCountsCategory );

	std::vector<RString> asTechCounts;
	for( int r=0; r < iMaxValues; r++ )
	{
		asTechCounts.push_back(ssprintf("%.3f", (*this)[r]));
	}

	return join( ",",asTechCounts );
}

void TechCounts::FromString( RString sTechCounts )
{
	std::vector<RString> saValues;
	split( sTechCounts, ",", saValues, true );

	if( saValues.size() != NUM_TechCountsCategory )
	{
		MakeUnknown();
		return;
	}

	FOREACH_ENUM( RadarCategory, rc )
	{
		(*this)[rc] = StringToFloat(saValues[rc]);
	}

}


// This is currently a more or less direct port of GetTechniques() from SL-ChartParser.lua
// It seems to match the results pretty closely, but not exactly.
// Instead of using null/false/true for foot mapping, an enum Foot has been introduced.
// And instead of using "L"/"D"/"U"/"R" for arrow mapping, a typedef StepDirection has been introduced 
// (it could probably also be an enum).
void TechCountsCalculator::CalculateTechCounts(const NoteData &in, TechCounts &out)
{
	TechCountsCounter statsCounter = TechCountsCounter();
	int curr_row = -1;

	// This is a bit of a misnomer, because we only process the "current" step
	// once curr_note.Row() has moved on to a new row.
	// But, I don't want to confuse this with lastStep
	StepDirection curr_step = StepDirection_None;
	NoteData::all_tracks_const_iterator curr_note = in.GetTapNoteRangeAllTracks(0, MAX_NOTE_ROW);

	// The notes aren't grouped, so we have to iterate through them all and figure out 
	// which ones go together
	TechCounts statsForRow = TechCounts();
	while(!curr_note.IsAtEnd())
	{
		if(curr_note.Row() != curr_row)
		{
			statsForRow.Zero();
			TechCountsCalculator::UpdateTechCounts(statsForRow, statsCounter, curr_step);

			FOREACH_ENUM( TechCountsCategory, tc )
			{
				out[tc] += statsForRow[tc];
			}
			curr_row = curr_note.Row();
			curr_step = StepDirection_None;
		}
		if (curr_note->type == TapNoteType_Tap || curr_note->type == TapNoteType_HoldHead)
		{
			curr_step = curr_step | TrackIntToStepDirection(curr_note.Track());
		}

		++curr_note;
	}
	TechCountsCalculator::UpdateTechCounts(statsForRow, statsCounter, curr_step);
}

// The main loop from GetTechniques().

void TechCountsCalculator::UpdateTechCounts(TechCounts &stats, TechCountsCounter &counter, StepDirection currentStep)
{
	
	if (currentStep == StepDirection_None)
	{
		return;
	}
	else if(IsSingleStep(currentStep))
	{
		if(counter.lastStep != StepDirection_None && currentStep == counter.lastStep)
		{
			CommitStream(stats, counter, Foot_None);
			counter.lastRepeatedFoot = currentStep;
		}

		counter.lastStep = currentStep;
		counter.lastFoot = SwitchFeet(counter.lastFoot);
		// Record whether we stepped on a matching or crossed-over L/R arrow
		if(currentStep == StepDirection_Left)
		{
			counter.stepsLr.push_back(SwitchFeet(counter.lastFoot));
		}
		else if(currentStep == StepDirection_Right)
		{
			counter.stepsLr.push_back(counter.lastFoot);
		}
		counter.anyStepsSinceLastCommitStream = true;
		// Regardless, record what arrow the foot stepped on (for brackets l8r)
		if(counter.lastFoot == Foot_Right)
		{
			counter.lastArrowR = currentStep;
		}
		else
		{
			counter.lastArrowL = currentStep;
		}
	}
	else if(IsJump(currentStep))
	{
		bool isBracketLeft = (currentStep == StepDirection_LD || currentStep == StepDirection_LU);
		bool isBracketRight = (currentStep == StepDirection_RD || currentStep == StepDirection_RU);
		
		Foot tieBreakFoot = Foot_None;
		
		if(isBracketLeft)
		{
			tieBreakFoot = Foot_Left;
		}
		else if(isBracketRight)
		{
			tieBreakFoot = Foot_Right;
		}

		CommitStream(stats, counter, tieBreakFoot);
		
		counter.lastStep = Foot_None;
		counter.lastRepeatedFoot = StepDirection_None;

		if(isBracketLeft || isBracketRight)
		{
			// possibly bracketable
			if(isBracketLeft && counter.trueLastFoot != Foot_Left)
			{
				// Check for interference from the right foot
				if( counter.trueLastArrowR == StepDirection_None ||
				!StepContainsStep(currentStep, counter.trueLastArrowR) )
				{
					stats[TechCountsCategory_Brackets] += 1;
					// allow subsequent brackets to stream
					counter.trueLastFoot = Foot_Left;
					counter.lastFoot = Foot_Left;
					// this prevents eg "LD bracket, DR also bracket"
					// NB: Take only the U or D arrow (cf above NB)
					counter.trueLastArrowL = currentStep - StepDirection_Left;
					counter.justBracketed = true;
				}
				else
				{
					// right foot is in the way; we have to step with both feet
					counter.trueLastFoot = Foot_None;
					counter.trueLastArrowL = StepDirection_Left;
					counter.trueLastArrowR = currentStep - StepDirection_Left;
					counter.lastArrowR = counter.trueLastArrowR;
				}
				counter.lastArrowL = counter.trueLastArrowL;
			}
			else if(isBracketRight && counter.trueLastFoot != Foot_Right)
			{
				// Check for interference from the left foot
				// Symmetric logic; see comments above
				if( counter.trueLastArrowL == StepDirection_None || 
				!StepContainsStep(currentStep, counter.trueLastArrowL) )
				{
					stats[TechCountsCategory_Brackets] += 1;
					counter.trueLastFoot = Foot_Right;
					counter.lastFoot = Foot_Right;
					counter.trueLastArrowR = currentStep - StepDirection_Right;
					counter.justBracketed = true;
				}
				else
				{
					counter.trueLastFoot = Foot_None;
					counter.trueLastArrowL = currentStep - StepDirection_Right;
					counter.trueLastArrowR = StepDirection_Right;
					counter.lastArrowL = counter.trueLastArrowL;
				}
				counter.lastArrowR = counter.trueLastArrowR;
			}
		}
		else
		{
			// LR or UD
			if(currentStep == StepDirection_UD)
			{
				// past footing influences which way the player can
				// comfortably face while jumping DU
				bool leftD = StepContainsStep(counter.trueLastArrowL, StepDirection_Down);
				bool leftU = StepContainsStep(counter.trueLastArrowL, StepDirection_Up);
				bool rightD = StepContainsStep(counter.trueLastArrowR, StepDirection_Down);
				bool rightU = StepContainsStep(counter.trueLastArrowR, StepDirection_Up);
				// The haskell version of this (decideDUFacing) is a
				// little more strict, and asserts each foot can't be
				// be on both D and U at once, but whatever.
				if ((leftD && !rightD) || (rightU && !leftU))
				{
					counter.trueLastArrowL = StepDirection_Down;
					counter.trueLastArrowR = StepDirection_Up;
				}
				else if((leftU && !rightU) || (rightD && !leftD))
				{
					counter.trueLastArrowL = StepDirection_Up;
					counter.trueLastArrowR = StepDirection_Down;
				}
				else
				{
					counter.trueLastArrowL = StepDirection_None;
					counter.trueLastArrowR = StepDirection_None;
				}
			}
			else
			{
				// not going to bother thinking about spin-jumps
				counter.trueLastArrowL = StepDirection_None;
				counter.trueLastArrowR = StepDirection_None;
			}
			counter.trueLastFoot = Foot_None;
		}
	}
	else
	{
		CommitStream(stats, counter, Foot_None);
		counter.lastStep = StepDirection_None;
		counter.lastRepeatedFoot = StepDirection_None;
		// triple/quad - always gotta bracket these
		stats[TechCountsCategory_Brackets] += 1;
		counter.trueLastFoot = Foot_None;
	}
}

void TechCountsCalculator::CommitStream(TechCounts &stats, TechCountsCounter &counter, Foot tieBreaker)
{
	int ns = (int)counter.stepsLr.size();
	int nx = 0;
	bool needFlip = false;
	
	for (unsigned i = 0; i < counter.stepsLr.size(); i++)
	{
		// Count crossed-over steps given initial footing
		if(!counter.stepsLr[i])
		{
			nx += 1;
		}
	}

	if(nx * 2 > ns)
	{
		// Easy case - more than half the L/R steps in this stream were crossed over,
		// so we guessed the initial footing wrong and need to flip the stream.
		needFlip = true;
	}
	else if(nx * 2 == ns)
	{
		// Exactly half crossed over. Note that flipping the stream will introduce
		// a jack (i.e. break the alternating-feet assumption) on the first note,
		// whereas leaving it as is will footswitch that note. Break the tie by
		// looking at history to see if the chart is already more jacky or switchy.
		// (OTOH, the reverse applies if the previous stream was also flipped.)

		if(tieBreaker != Foot_None)
		{
			// But first, as a higher priority tiebreaker -- if this stream is followed
			// by a bracketable jump, choose whichever flipness lets us bracket it.
			if(counter.justBracketed)
			{
				// (However, don't get too overzealous -- if also preceded by a
				// bracket jump, that forces the footing, so we can't bracket both)
				needFlip = false;
			}
			else if(counter.lastFoot == Foot_Right)
			{
				needFlip = tieBreaker == Foot_Right;
			}
			else
			{
				needFlip = tieBreaker == Foot_Left;
			}
		}
		else if(stats[TechCountsCategory_Footswitches] > stats[TechCountsCategory_Jacks])
		{
			// Match flipness of last chunk -> footswitch
			needFlip = counter.lastFlip;
		}
		else
		{
			// Don't match -> jack
			needFlip = !counter.lastFlip;
		}
	}

	// Now that we know the correct flip, see if the stream needs split.
	// If "too much" of the stream is *completely* crossed over, force a
	// double-step there by splitting the stream to stay facing forward.
	// Heuristic value (9) chosen by inspection on Subluminal - After Hours.

	int splitIndex = -1;
	int splitFirstUncrossedStepIndex = -1;
	int numConsecutiveCrossed = 0;
	for (unsigned i = 0; i < counter.stepsLr.size(); i++)
	{
		bool stepIsCrossed = counter.stepsLr[i] == needFlip;
		if(splitIndex == -1)
		{
			if(stepIsCrossed)
			{
				numConsecutiveCrossed += 1;
				if(numConsecutiveCrossed == 9)
				{
					splitIndex = i - 9;
				}
			}
			else
			{
				numConsecutiveCrossed = 0;
			}
		}
		else if(splitFirstUncrossedStepIndex == -1)
		{
			// Also search for the first un-crossed step after the fux section,
			// which will be used below in the `splitIndex == 1` case.
			if(!stepIsCrossed)
			{
				splitFirstUncrossedStepIndex = i;
			}
		}
	}

	if(splitIndex > -1)
	{
		// Note that since we take O(n) to compute `needFlip`, and then we might
		// do repeated work scanning already-analyzed ranges of `StepsLR` during
		// the recursive call here, it's technically possible for a worst case
		// performance of O(n^2 / 18) if the whole chart fits this pattern.
		// But this is expected to be pretty rare to happen even once so probably ok.
		// TODO: Optimize the above by using a separate explicit counter for `nx`.

		if(splitIndex == 0)
		{
			// Prevent infinite splittage if the fux section starts immediately.
			// In that case split instead at the first non-crossed step.
			// The next index is guaranteed to be set in this case.
			splitIndex = splitFirstUncrossedStepIndex;
		}

		std::vector<bool> stepsLr1;
		std::vector<bool> stepsLr2;

		for (unsigned i = 0; i < counter.stepsLr.size(); i++)
		{
			if((int)i < splitIndex)
			{
				stepsLr1.push_back(counter.stepsLr[i]);
			}
			else
			{
				stepsLr2.push_back(counter.stepsLr[i]);
			}
		}
		// Recurse for each split half
		counter.stepsLr = stepsLr1;
		CommitStream(stats, counter, Foot_None);
		counter.lastRepeatedFoot = Foot_None;
		counter.stepsLr = stepsLr2;
		CommitStream(stats, counter, tieBreaker);
	}
	else
	{
		// No heuristic doublestep splittage necessary. Update the stats.

		if(needFlip)
		{
			stats[TechCountsCategory_Crossovers] += ns - nx;
		}
		else
		{
			stats[TechCountsCategory_Crossovers] += nx;
		}

		if(counter.lastRepeatedFoot != StepDirection_None)
		{
			if(needFlip == counter.lastFlip)
			{
				stats[TechCountsCategory_Footswitches] += 1;
				if(counter.lastRepeatedFoot == StepDirection_Left || counter.lastRepeatedFoot == StepDirection_Right)
				{
					stats[TechCountsCategory_Sideswitches] += 1;
				}
			}
			else
			{
				stats[TechCountsCategory_Jacks] += 1;
			}
		}
	}

	counter.stepsLr.clear();
	counter.lastFlip = needFlip;

	// Merge the (flip-ambiguous) last-arrow tracking into the source of truth
	// TODO(bracket) - Do we need to check if the `LastArrow`s are empty, like
	// the hs version does? I hypothesize it actually makes no difference.
	// NB: This can't just be `ns > 0` bc we want to update the TrueLastFoot
	// even when there were just U/D steps (whereupon StepsLR would be empty).

	if(counter.anyStepsSinceLastCommitStream)
	{
		if(needFlip)
		{
			if(counter.lastFoot == Foot_Right)
			{
				counter.trueLastFoot = Foot_Left;
			}
			else
			{
				counter.trueLastFoot = Foot_Right;
			}
			counter.trueLastArrowL = counter.lastArrowR;
			counter.trueLastArrowR = counter.lastArrowL;
			// TODO: "If we had to flip a stream right after a bracket jump,
			// that'd make it retroactively unbracketable; if so cancel it"
			// (...do i even believe this anymore? probs not...)
		}
		else
		{
			if(counter.lastFoot == Foot_Right)
			{
				counter.trueLastFoot = Foot_Right;
			}
			else
			{
				counter.trueLastFoot = Foot_Left;
			}
			counter.trueLastArrowL = counter.lastArrowL;
			counter.trueLastArrowR = counter.lastArrowR;
		}
		counter.anyStepsSinceLastCommitStream = false;
		counter.lastArrowL = Foot_None;
		counter.lastArrowR = Foot_None;
		counter.justBracketed = false;
	}
}

// lua start

class LunaTechCounts: public Luna<TechCounts>
{
public:


	static int GetValue( T* p, lua_State *L ) { lua_pushnumber( L, (*p)[Enum::Check<TechCountsCategory>(L, 1)] ); return 1; }

	LunaTechCounts()
	{
		ADD_METHOD( GetValue );
	}
};

LUA_REGISTER_CLASS( TechCounts )

/*
 * (c) 2023 Michael Votaw
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
