/// \file
/// \brief Softbank emoji access. This file goes with the emoji glyph image
///			generated by another application. It maps the glyphs to a grid
///			where a direct relationship between character and x,y location of
///			the associated glyph is established.
#ifndef _EMOJI_H_
#define _EMOJI_H_

/// Size of the graphics cells
#define EMOJI_CELL_SIZE			20

/// The width of each group in cells
#define EMOJI_GROUP_X			16
/// The height of each group in cells
#define EMOJI_GROUP_Y			6

/// Emoji glyphs are grouped into blocks of 256, but only the first (6*16) are used.
/// So we group them and chop out the whitespace in the glyph image. The rc parameter
/// is a rectangle struct with x1, y1, x2, y2 parameters. If using Lgi then GRect is
/// suitable, otherwise create your own class or struct with those members.
/*
#define EMOJI_GROUP(i)			(((i)-EMOJI_START) / 256)
#define EMOJI_CH2LOC(i, rc)		{ int g = EMOJI_GROUP(i);							\
								int idx = (i) - EMOJI_START;						\
								int y = g * EMOJI_GROUP_Y;							\
								rc.x1 = (idx % 16) * EMOJI_CELL_SIZE;				\
								rc.x2 = rc.x1 + EMOJI_CELL_SIZE - 1;				\
								rc.y1 = (y + ((idx % 256) / 16)) * EMOJI_CELL_SIZE;	\
								rc.y2 = rc.y1 + EMOJI_CELL_SIZE - 1;				\
								}
*/

#endif