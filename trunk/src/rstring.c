/***************************************************************************
 *   Copyright (C) 2007 by Rui Maciel   *
 *   rui.maciel@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "rstring.h"

#include <stdlib.h>
#include <stdio.h>		// printf
#include <string.h>
#include <assert.h>
#include <wchar.h>


rwstring *
rws_create (size_t length)
{
	rwstring *rws;
	rws = malloc (sizeof (rwstring));	// allocates memory for a struct rwstring
	if (rws == NULL)
		return NULL;

	rws->length = 0;
	rws->max = length;

	rws->text = calloc (rws->max + 1, sizeof (wchar_t));
	if (rws->text == NULL)
		return NULL;

	return rws;
}


void
rws_free (rwstring ** rws)
{
	assert (rws != NULL);
	if (*rws != NULL)
	{
		if ((*rws)->text != NULL)
		{
			free ((*rws)->text);
			(*rws)->text = NULL;
		}
		free (*rws);
		*rws = NULL;
	}

}

rwstring *
rws_duplicate (rwstring * copied)
{
	assert (copied != NULL);
	rwstring *copy = malloc (sizeof (rwstring));
	if (copy == NULL)
		return NULL;

	copy->text = calloc (1, sizeof (wchar_t));
	if (copy->text == NULL)
		return NULL;
	copy->text[0] = 0;
	copy->length = copy->max = 0;

	if (rws_copyrws (copy, copied) == RS_OK)
		return copy;
	else
		return NULL;
}


size_t
rws_length (rwstring * rws)
{
	assert (rws != NULL);
	return rws->length;
}


int
rws_copyrws (rwstring * to, const rwstring * from)
{
	assert (to != NULL);

	if (from == NULL)
		return RS_OK;	// nothing to copy

	//TODO implement intelligent memory allocation
	if (to->max < from->length)
	{
		to->text = realloc (to->text, (from->length + 1) * sizeof (wchar_t));
		if (to->text == NULL)
		{
			return RS_MEMORY;
		}

		to->max = from->length;
	}
	wcsncpy (to->text, from->text, from->length);
	to->text[from->length] = 0;
	to->length = from->length;
	return RS_OK;
}


int
rws_copywcs (rwstring * to, const wchar_t * from, const size_t length)
{
	assert (to != NULL);

	if (from == NULL)
		return RS_OK;

	//TODO implement intelligent memory allocation
	if (to->max < length)
	{
		to->text = realloc (to->text, (length + 1) * sizeof (wchar_t));
		if (to->text == NULL)
		{
			return RS_MEMORY;
		}

		to->max = length;
	}
	wcsncpy (to->text, from, length);
	to->text[length] = 0;
	to->length = length;
	return RS_OK;
}

int
rws_catrws (rwstring * pre, const rwstring * pos)
{
	assert (pre != NULL);
	if (pos == NULL)
		return RS_OK;

	if (pre->max < pre->length + pos->length + 1)
	{
		pre->text = realloc (pre->text, (pre->length + pos->length + 1) * sizeof (wchar_t));
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + pos->length;
	}
	wcsncpy (pre->text + pre->length, pos->text, pos->length);
	pre->text[pre->length + pos->length] = 0;
	pre->length = pre->length + pos->length;
	return RS_OK;
}

int
rws_catrcs (rwstring * pre, const rcstring * pos)
{
	size_t utf8pos;
	wchar_t wc;
	char i;			/* static loop counter */

	assert (pre != NULL);
	assert (pos != NULL);
	assert (strlen (pos->text) == pos->length);
	assert (strlen (pos->text) == pos->length);

	/* starting the conversion */
	utf8pos = 0;

	while (utf8pos < pos->length)
	{
		if ((pos->text[utf8pos] & 0x80) == 0)
		{
			if (rws_catc (pre, pos->text[utf8pos++]) != RS_OK)
			{
				return RS_MEMORY;
			}
		}
		else if ((pos->text[utf8pos] & 0xE0) == 0xC0)
		{
			wc = (pos->text[utf8pos++] & 0x1F) << 6;
			if ((pos->text[utf8pos] & 0xC0) == 0x80)
			{
				wc |= (pos->text[utf8pos++] & 63);

			}
			else
			{	/* Invalid utf8 string */
				return RS_UNKNOWN;	/* malformed UTF8 string */
			}
			if (rws_catwc (pre, wc) != RS_OK)
			{
				return RS_MEMORY;
			}
		}
		else if ((pos->text[utf8pos] & 0xF0) == 0xE0)
		{
			wc = pos->text[utf8pos++] & 0xF << 12;
			for (i = 1; i >= 0; i--)
			{
				if ((pos->text[utf8pos] & 0xC0) == 0x80)
				{
					wc &= pos->text[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					return RS_UNKNOWN;
				}
			}
			if (rws_catwc (pre, wc) != RS_OK)
			{
				return RS_MEMORY;
			}

		}
		else if ((pos->text[utf8pos] & 0xF8) == 0xF0)
		{
			wc = pos->text[utf8pos++] & 0xF << 12;
			for (i = 2; i >= 0; i--)
			{
				if ((pos->text[utf8pos] & 0xC0) == 0x80)
				{
					wc &= pos->text[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					return RS_UNKNOWN;

				}
			}
			if (rws_catwc (pre, wc) != RS_OK)
			{
				return RS_MEMORY;
			}

		}
		else if ((pos->text[utf8pos] & 0xFC) == 0xF8)
		{
			for (i = 3; i >= 0; i--)
			{
				if ((pos->text[utf8pos] & 0xC0) == 0x80)
				{
					wc &= pos->text[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					return RS_UNKNOWN;

				}
			}
			if (rws_catwc (pre, wc) != RS_OK)
			{
				return RS_MEMORY;
			}
		}
		else if ((pos->text[utf8pos] & 0xFE) == 0xFC)
		{
			for (i = 4; i >= 0; i--)
			{
				if ((pos->text[utf8pos] & 0xC0) == 0x80)
				{
					wc &= pos->text[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					return RS_UNKNOWN;
				}
			}
			if (rws_catwc (pre, wc) != RS_OK)
			{
				return RS_MEMORY;
			}
		}
		else		/* Invalid utf8 string */
		{
			return RS_UNKNOWN;
		}
	}

	return RS_OK;
}


int
rws_catwcs (rwstring * pre, const wchar_t * pos, const size_t length)
{
	assert (pre != NULL);
	if (pos == NULL)
		return RS_OK;

	if (pre->max < pre->length + length)
	{
		pre->text = realloc (pre->text, (pre->length + length + 1) * sizeof (wchar_t));
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + length;
	}
	wcsncpy (pre->text + pre->length, pos, length);
	pre->text[pre->length + length] = 0;
	pre->length = pre->length + length;	//is this the correct value?
	return RS_OK;
}


int
rws_catwc (rwstring * pre, const wchar_t c)
{
	assert (pre != NULL);
	if (pre->max <= pre->length)
	{
		pre->text = realloc (pre->text, (pre->length + 2) * sizeof (wchar_t));	// 2 = new character + null character
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + 1;
	}
	pre->text[pre->length] = c;
	pre->text[pre->length + 1] = L'\0';
	pre->length++;
	return RS_OK;
}


int
rws_catc (rwstring * pre, const char c)
{
//      assert(pre != NULL);
	wchar_t newc;
	mbtowc (&newc, &c, 1);
	return rws_catwc (pre, newc);
}


rwstring *
rws_wrap (wchar_t * wcs)
{
	if (wcs == NULL)
		return NULL;
	rwstring *wrapper = malloc (sizeof (rwstring));
	if (wrapper == NULL)
		return NULL;
	wrapper->max = wrapper->length = wcslen (wcs);
	wrapper->text = wcs;
	return wrapper;
}


wchar_t *
rws_unwrap (rwstring * rws)
{
	if (rws == NULL)
		return NULL;
	wchar_t *out = rws->text;

	free (rws);
	return out;
}


rcstring *
rcs_create (size_t length)
{
	rcstring *rcs = malloc (sizeof (rcstring));	// allocates memory for a struct rcstring
	if (rcs == NULL)
		return NULL;

	rcs->length = 0;
	rcs->max = length;

	rcs->text = calloc (rcs->max + 1, sizeof (char));
	if (rcs->text == NULL)
		return NULL;

	return rcs;
}


void
rcs_free (rcstring ** rcs)
{
	assert (rcs != NULL);
	if (*rcs != NULL)
	{
		if ((*rcs)->text != NULL)
		{
			free ((*rcs)->text);
			(*rcs)->text = NULL;
		}
		free (*rcs);
		*rcs = NULL;
	}

}

rcstring *
rcs_duplicate (rcstring * copied)
{
	assert (copied != NULL);
	rcstring *copy = malloc (sizeof (rcstring));
	if (copy == NULL)
		return NULL;

	/*TODO check if this makes any sense */
	copy->text = calloc (1, sizeof (char));
	if (copy->text == NULL)
		return NULL;
	copy->text[0] = 0;
	copy->length = copy->max = 0;

	if (rcs_copyrcs (copy, copied) == RS_OK)
		return copy;
	else
		return NULL;
}


size_t
rcs_length (rcstring * rcs)
{
	assert (rcs != NULL);
	return rcs->length;
}


int
rcs_copyrcs (rcstring * to, const rcstring * from)
{
	assert (to != NULL);

	if (from == NULL)
		return RS_OK;	// nothing to copy

	//TODO implement intelligent memory allocation
	if (to->max < from->length)
	{
		to->text = realloc (to->text, (from->length + 1) * sizeof (char));
		if (to->text == NULL)
		{
			return RS_MEMORY;
		}

		to->max = from->length;
	}
	strncpy (to->text, from->text, from->length);
	to->text[from->length] = '\0';
	to->length = from->length;
	return RS_OK;
}


int
rcs_copycs (rcstring * to, const char *from, const size_t length)
{
	assert (to != NULL);

	if (from == NULL)
		return RS_OK;

	//TODO implement intelligent memory allocation
	if (to->max < length)
	{
		to->text = realloc (to->text, (length + 1) * sizeof (char));
		if (to->text == NULL)
		{
			return RS_MEMORY;
		}

		to->max = length;
	}
	strncpy (to->text, from, length);
	to->text[length] = '\0';
	to->length = length;
	return RS_OK;
}

int
rcs_catrcs (rcstring * pre, const rcstring * pos)
{
	assert (pre != NULL);
	if (pos == NULL)
		return RS_OK;

	if (pre->max < pre->length + pos->length + 1)
	{
		pre->text = realloc (pre->text, (pre->length + pos->length + 1) * sizeof (char));
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + pos->length;
	}
	strncpy (pre->text + pre->length, pos->text, pos->length);
	pre->text[pre->length + pos->length] = 0;
	pre->length = pre->length + pos->length;
	return RS_OK;
}

int
rcs_catcs (rcstring * pre, const char *pos, const size_t length)
{
	assert (pre != NULL);
	if (pos == NULL)
		return RS_OK;

	if (pre->max < pre->length + length)
	{
		pre->text = realloc (pre->text, (pre->length + length + 1) * sizeof (char));
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + length;
	}
	strncpy (pre->text + pre->length, pos, length);
	pre->text[pre->length + length] = 0;
	pre->length = pre->length + length;	//is this the correct value?
	return RS_OK;
}


int
rcs_catwc (rcstring * pre, const wchar_t wc)
{
	assert (pre != NULL);
	/*TODO convert wc to multi-byte UTF8 string and append */

	if (wc <= 0x7F)
	{
		rcs_catc (pre, wc);
	}
	else if (wc <= 0x7FF)
	{
		rcs_catc (pre, (wc >> 6) | 192);
		rcs_catc (pre, (wc & 63) | 128);
	}
	else if (wc <= 0xFFFF)
	{
		rcs_catc (pre, wc >> 12 | 224);
		rcs_catc (pre, (wc >> 6 & 63) | 128);
		rcs_catc (pre, (wc & 63) | 128);
	}
	else if (wc <= 0x1FFFFF)
	{
		rcs_catc (pre, wc >> 18 | 240);
		rcs_catc (pre, (wc >> 12 & 63) | 128);
		rcs_catc (pre, (wc >> 6 & 63) | 128);
		rcs_catc (pre, (wc & 63) | 128);
	}
	else if (wc <= 0x3FFFFFF)
	{
		rcs_catc (pre, wc >> 24 | 248);
		rcs_catc (pre, (wc >> 18 & 63) | 128);
		rcs_catc (pre, (wc >> 12 & 63) | 128);
		rcs_catc (pre, (wc >> 6 & 63) | 128);
		rcs_catc (pre, (wc & 63) | 128);
	}
	else if (wc <= 0x7FFFFFFF)
	{
		rcs_catc (pre, wc >> 30 | 252);
		rcs_catc (pre, (wc >> 24 & 63) | 128);
		rcs_catc (pre, (wc >> 18 & 63) | 128);
		rcs_catc (pre, (wc >> 12 & 63) | 128);
		rcs_catc (pre, (wc >> 6 & 63) | 128);
		rcs_catc (pre, (wc & 63) | 128);
	}
	return RS_OK;
}


int
rcs_catc (rcstring * pre, const char c)
{
	assert (pre != NULL);
	if (pre->max <= pre->length)
	{
		pre->text = realloc (pre->text, (pre->length + 2) * sizeof (char));	// 2 = new character + null character
		if (pre->text == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + 1;
	}
	pre->text[pre->length] = c;
	pre->text[pre->length + 1] = '\0';
	pre->length++;
	return RS_OK;
}


rcstring *
rcs_wrap (char *cs)
{
	assert (cs != NULL);

	rcstring *wrapper;
	wrapper = malloc (sizeof (rcstring));
	if (wrapper == NULL)
		return NULL;
	wrapper->length = strlen (cs);
	wrapper->max = wrapper->length;
	wrapper->text = cs;
	return wrapper;
}


char *
rcs_unwrap (rcstring * rcs)
{
	if (rcs == NULL)
		return NULL;
	char *out = rcs->text;

	free (rcs);
	return out;
}
