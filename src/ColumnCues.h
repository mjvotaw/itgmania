#ifndef COLUMN_CUES_H
#define COLUMN_CUES_H

#include "GameConstantsAndTypes.h"
class NoteData;
struct lua_State;

struct ColumnCueColumn
{
	int colNum;
	bool isMine;
	ColumnCueColumn()
	{
		colNum = 0;
		isMine = false;
	}
	ColumnCueColumn(int c, bool m)
	{
		colNum = c;
		isMine = m;
	}
	void PushSelf( lua_State *L );
};

struct ColumnCue
{
	float startTime;
	float duration;
	std::vector<ColumnCueColumn> columns;

	ColumnCue()
	{
		startTime = -1;
		duration = -1;
	}

	ColumnCue(float s, float d, std::vector<ColumnCueColumn> c)
	{
		startTime = s;
		duration = d;
		columns.assign(c.begin(), c.end());
	}

	void PushSelf( lua_State *L );
};

struct ColumnCues
{
	std::vector<ColumnCue> columnCues;

	ColumnCues()
	{
		Zero();
	}

	void Zero()
	{
		columnCues.clear();
	}

	static void CalculateColumnCues(const NoteData &in, ColumnCues &out);
	void PushSelf(lua_State *L);
};

#endif