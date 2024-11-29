/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

unsigned char	*r_turb_pbase, *r_turb_pdest;
fixed16_t		r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int				*r_turb_turb;
int				r_turb_spancount;

/*
=============
D_WarpScreen

// this performs a slight compression of the screen at the same time as
// the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen (void)
{
	int		w, h, src_w, src_h;
	int		u,v;
	byte	*dest;
	int		*turb;
	int		*col;
	byte	**row;
	byte	*rowptr[MAXHEIGHT+(AMP2*2)];
	int		column[MAXWIDTH+(AMP2*2)];
	float	wratio, hratio;
	unsigned int	_vid_rowbytes;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;
	src_w = scr_vrect.width;
	src_h = scr_vrect.height;

	wratio = w / (float)src_w;
	hratio = h / (float)src_h;

	for (v=0 ; v<src_h+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}

	for (u=0 ; u<src_w+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	_vid_rowbytes = vid.rowbytes;
	turb = intsintable + ((int)(cl.time*SPEED)&(CYCLE-1));
	dest = vid.buffer + scr_vrect.y * _vid_rowbytes + scr_vrect.x;

	for (v=0 ; v<src_h ; v++, dest += _vid_rowbytes)
	{
		col = &column[turb[v]];
		row = &rowptr[v];

		for (u=0 ; u<src_w ; u+=4)
		{
			dest[u+0] = row[turb[u+0]][col[u+0]];
			dest[u+1] = row[turb[u+1]][col[u+1]];
			dest[u+2] = row[turb[u+2]][col[u+2]];
			dest[u+3] = row[turb[u+3]][col[u+3]];
		}
	}
}

/*
=============
D_DrawTurbulent8Span
=============
*/
static inline void D_DrawTurbulent8Span (void)
{
	int		sturb, tturb;

	// Preload global variables.
	fixed16_t _r_turb_s = r_turb_s;
	fixed16_t _r_turb_t = r_turb_t;
	const fixed16_t _r_turb_sstep = r_turb_sstep;
	fixed16_t _r_turb_tstep = r_turb_tstep;
	int _r_turb_spancount = r_turb_spancount;
	int* _r_turb_turb = r_turb_turb;
	unsigned char* _r_turb_pbase = r_turb_pbase;
	unsigned char* _r_turb_pdest = r_turb_pdest;

	do
	{
		sturb = ((_r_turb_s + _r_turb_turb[(_r_turb_t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((_r_turb_t + _r_turb_turb[(_r_turb_s>>16)&(CYCLE-1)])>>16)&63;
		*_r_turb_pdest++ = *(_r_turb_pbase + (tturb<<6) + sturb);
		_r_turb_s += _r_turb_sstep;
		_r_turb_t += _r_turb_tstep;
	} while (--_r_turb_spancount > 0);

	// Update global variables.
	r_turb_pbase = _r_turb_pbase;
	r_turb_pdest = _r_turb_pdest;
	r_turb_s = _r_turb_s;
	r_turb_t = _r_turb_t;
	r_turb_spancount = _r_turb_spancount;
}

/*
=============
Turbulent8
=============
*/
void Turbulent8 (espan_t *pspan)
{
	int				count;
	fixed16_t		snext, tnext;
	float			sdivz, tdivz, zi, z, du, dv, spancountminus1;
	float			sdivz16stepu, tdivz16stepu, zi16stepu;
	
	r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));

	r_turb_sstep = 0;	// keep compiler happy
	r_turb_tstep = 0;	// ditto

	r_turb_pbase = (unsigned char *)cacheblock;

	sdivz16stepu = d_sdivzstepu * 16;
	tdivz16stepu = d_tdivzstepu * 16;
	zi16stepu = d_zistepu * 16;

	do
	{
		r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		r_turb_s = (int)(sdivz * z) + sadjust;
		if (r_turb_s > bbextents)
			r_turb_s = bbextents;
		else if (r_turb_s < 0)
			r_turb_s = 0;

		r_turb_t = (int)(tdivz * z) + tadjust;
		if (r_turb_t > bbextentt)
			r_turb_t = bbextentt;
		else if (r_turb_t < 0)
			r_turb_t = 0;

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 16)
				r_turb_spancount = 16;
			else
				r_turb_spancount = count;

			count -= r_turb_spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
				sdivz += sdivz16stepu;
				tdivz += tdivz16stepu;
				zi += zi16stepu;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				r_turb_sstep = (snext - r_turb_s) >> 4;
				r_turb_tstep = (tnext - r_turb_t) >> 4;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
				spancountminus1 = (float)(r_turb_spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps

				if (r_turb_spancount > 1)
				{
					r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
					r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
				}
			}

			r_turb_s = r_turb_s & ((CYCLE<<16)-1);
			r_turb_t = r_turb_t & ((CYCLE<<16)-1);

			D_DrawTurbulent8Span ();

			r_turb_s = snext;
			r_turb_t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

/*
=============
D_DrawSpans8
Note: This function is the top CPU consumer. Optimize it as far as possible!
=============
*/
void D_DrawSpans8 (espan_t *pspan)
{
	int		count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t	s, t, snext, tnext, sstep = 0, tstep = 0;
	float		sdivz, tdivz, zi, z, du, dv, spancountminus1;

	pbase = (unsigned char *)cacheblock;

	byte *viewbuffer = (byte *)d_viewbuffer;
	int _screenwidth = screenwidth;
	float _d_sdivzorigin = d_sdivzorigin;
	float _d_sdivzstepv = d_sdivzstepv;
	float _d_sdivzstepu = d_sdivzstepu;
	float _d_tdivzorigin = d_tdivzorigin;
	float _d_tdivzstepv = d_tdivzstepv;
	float _d_tdivzstepu = d_tdivzstepu;
	float _d_ziorigin = d_ziorigin;
	float _d_zistepv = d_zistepv;
	float _d_zistepu = d_zistepu;
	fixed16_t _sadjust = sadjust;
	fixed16_t _tadjust = tadjust;
	fixed16_t _bbextents = bbextents;
	fixed16_t _bbextentt = bbextentt;
	int _cachewidth = cachewidth;

	float sdivzstepu = _d_sdivzstepu * 16;
	float tdivzstepu = _d_tdivzstepu * 16;
	float zistepu = _d_zistepu * 16;

	do
	{
		pdest = (unsigned char *)&viewbuffer[(_screenwidth * pspan->v) + pspan->u];
		count = pspan->count >> 4;
		spancount = pspan->count % 16;

		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		du = (float)pspan->u;
		dv = (float)pspan->v;

		sdivz = _d_sdivzorigin + dv*_d_sdivzstepv + du*_d_sdivzstepu;
		tdivz = _d_tdivzorigin + dv*_d_tdivzstepv + du*_d_tdivzstepu;
		zi = _d_ziorigin + dv*_d_zistepv + du*_d_zistepu;
		z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		// prevent round-off error on <0 steps from causing overstepping
		// and running off the edge of the texture.
		s = bound(0, (int) (sdivz * z) + _sadjust, _bbextents);
		t = bound(0, (int) (tdivz * z) + _tadjust, _bbextentt);

		while (count-- > 0)
		{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
			sdivz += sdivzstepu;
			tdivz += tdivzstepu;
			zi += zistepu;
			z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

			snext = bound(16, (int) (sdivz * z) + _sadjust, _bbextents);
			tnext = bound(16, (int) (tdivz * z) + _tadjust, _bbextentt);

			sstep = (snext - s) >> 4;
			tstep = (tnext - t) >> 4;
			pdest += 16;

#define WRITEPDEST(i) \
	{ pdest[i] = *(pbase + (s >> 16) + (t >> 16) * _cachewidth); s += sstep; t += tstep; }

			WRITEPDEST(-16);
			WRITEPDEST(-15);
			WRITEPDEST(-14);
			WRITEPDEST(-13);
			WRITEPDEST(-12);
			WRITEPDEST(-11);
			WRITEPDEST(-10);
			WRITEPDEST(-9);
			WRITEPDEST(-8);
			WRITEPDEST(-7);
			WRITEPDEST(-6);
			WRITEPDEST(-5);
			WRITEPDEST(-4);
			WRITEPDEST(-3);
			WRITEPDEST(-2);
			WRITEPDEST(-1);

			s = snext;
			t = tnext;
		}

		// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
		// can't step off polygon), clamp, calculate s and t steps across
		// span by division, biasing steps low so we don't run off the
		// texture
		if (spancount > 0)
		{
			spancountminus1 = (float)(spancount - 1);
			sdivz += d_sdivzstepu * spancountminus1;
			tdivz += d_tdivzstepu * spancountminus1;
			zi += d_zistepu * spancountminus1;
			z = (float)0x10000 / zi;   // prescale to 16.16 fixed-point

			snext = bound(16, (int)(sdivz * z) + sadjust, bbextents);
			tnext = bound(16, (int)(tdivz * z) + tadjust, bbextentt);

			if (spancount > 1)
			{
				sstep = (snext - s) / (spancount - 1);
				tstep = (tnext - t) / (spancount - 1);
			}

			pdest += spancount;

			switch (spancount)
			{
			case 16:
				WRITEPDEST(-16);
			case 15:
				WRITEPDEST(-15);
			case 14:
				WRITEPDEST(-14);
			case 13:
				WRITEPDEST(-13);
			case 12:
				WRITEPDEST(-12);
			case 11:
				WRITEPDEST(-11);
			case 10:
				WRITEPDEST(-10);
			case  9:
				WRITEPDEST(-9);
			case  8:
				WRITEPDEST(-8);
			case  7:
				WRITEPDEST(-7);
			case  6:
				WRITEPDEST(-6);
			case  5:
				WRITEPDEST(-5);
			case  4:
				WRITEPDEST(-4);
			case  3:
				WRITEPDEST(-3);
			case  2:
				WRITEPDEST(-2);
			case  1:
				WRITEPDEST(-1);
				break;
			}
		}
	}
	while ((pspan = pspan->pnext));
}

/*
=============
D_DrawZSpans
=============
*/
void D_DrawZSpans (espan_t *pspan)
{
	int				count, doublecount, izistep;
	int				izi;
	short			*pdest;
	unsigned		ltemp;
	float			zi;
	float			du, dv;

// FIXME: check for clamping/range problems
// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	short *_d_pzbuffer = d_pzbuffer;
	unsigned int _d_zwidth= d_zwidth;
	float _d_ziorigin = d_ziorigin;
	float _d_zistepu = d_zistepu;
	float _d_zistepv = d_zistepv;

	do
	{
		pdest = _d_pzbuffer + (_d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

	// calculate the initial 1/z
		du = (float)pspan->u;
		dv = (float)pspan->v;

		zi = _d_ziorigin + dv*_d_zistepv + du*_d_zistepu;
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		if ((uintptr_t)pdest & 0x02)
		{
			*pdest++ = (short)(izi >> 16);
			izi += izistep;
			count--;
		}

		if ((doublecount = count >> 1) > 0)
		{
			do
			{
				ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(int *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}

		if (count & 1)
			*pdest = (short)(izi >> 16);

	} while ((pspan = pspan->pnext) != NULL);
}
