/* hacktv - Analogue video transmitter for the HackRF                    */
/*=======================================================================*/
/* Copyright 2018 Philip Heron <phil@sanslogic.co.uk>                    */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* -=== WSS encoder ===- */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "video.h"
#include "vbidata.h"

typedef struct {
	const char *id;
	uint8_t code;
} _wss_modes_t;

static const _wss_modes_t _wss_modes[] = {
	{ "4:3", 0x08 },
	{ "16:9", 0x07 },
	{ "14:9-letterbox", 0x01 },
	{ "16:9-letterbox", 0x04 },
	{ "auto", 0xFF },
	{ NULL, 0 },
};

static size_t _group_bits(uint8_t *vbi, uint8_t code, size_t offset, size_t length)
{
	int i, b;
	
	while(length--)
	{
		for(i = 0; i < 6; i++, offset++)
		{
			if(i == 3) code ^= 1;
			
			b = 7 - (offset % 8);
			
			vbi[offset / 8] &= ~(1 << b);
			vbi[offset / 8] |= (code & 1) << b;
		}
		
		code >>= 1;
	}
	
	return(offset);
}

int wss_init(wss_t *s, vid_t *vid, char *mode)
{
	int16_t level;
	size_t o;
	
	memset(s, 0, sizeof(wss_t));
	
	/* Find the mode settings */
	s->code = 0;
	for(o = 0; _wss_modes[o].id != NULL; o++)
	{
		if(strcasecmp(mode, _wss_modes[o].id) == 0)
		{
			s->code = _wss_modes[o].code;
			break;
		}
	}
	
	if(s->code == 0)
	{
		fprintf(stderr, "wss: Unrecognised mode '%s'.\n", mode);
		return(VID_ERROR);
	}
	
	/* Calculate the high level for the VBI data */
	level = round((vid->white_level - vid->black_level) * (5.0 / 7.0));
	
	s->vid = vid;
	s->lut = vbidata_init(
		320, s->vid->width,
		level,
		VBIDATA_FILTER_RC, 0.7
	);
	
	if(!s->lut)
	{
		return(VID_OUT_OF_MEMORY);
	}
	
	/* Prepare the VBI data. Start with the run-in and start code */
	s->vbi[0] = 0xF8; // 11111000
	s->vbi[1] = 0xE3; // 11100011
	s->vbi[2] = 0x8E; // 10001110
	s->vbi[3] = 0x38; // 00111000
	s->vbi[4] = 0xF1; // 11110001
	s->vbi[5] = 0xE0; // 11100000
	s->vbi[6] = 0xF8; // 11111___
	
	/* Group 1 (Aspect Ratio) */
	o = _group_bits(s->vbi, s->code, 29 + 24, 4);
	
	/* Group 2 (Enhanced Services) */
	o = _group_bits(s->vbi, 0x00, o, 4);
	
	/* Group 3 (Subtitles) */
	o = _group_bits(s->vbi, 0x00, o, 3);
	
	/* Group 4 (Reserved) */
	o = _group_bits(s->vbi, 0x00, o, 3);
	
	/* Calculate width of line to blank */
	s->blank_width = round(s->vid->sample_rate * 42.5e-6);
	
	return(VID_OK);
}

void wss_free(wss_t *s)
{
	if(s == NULL) return;
	
	free(s->lut);
	
	memset(s, 0, sizeof(wss_t));
}

void wss_render_line(wss_t *s)
{
	int x;
	
	/* WSS is rendered on line 23 */
	if(s->vid->line == 23)
	{
		if(s->code == 0xFF)
		{
			/* Auto mode selects between 4:3 and 16:9 based on the
			 * the ratio of the source frame. */
			_group_bits(s->vbi, s->vid->ratio <= (14.0 / 9.0) ? 0x08 : 0x07, 29 + 24, 4);
		}
		
		/* 42.5μs of line 23 needs to be blanked otherwise the WSS bits may
		 * overlap active video */
		for(x = s->vid->half_width; x < s->blank_width; x++)
		{
			s->vid->output[x * 2] = s->vid->black_level;
		}
		
		vbidata_render_nrz(s->lut, s->vbi, -55, 137, VBIDATA_MSB_FIRST, s->vid->output, 2);
	}
}

