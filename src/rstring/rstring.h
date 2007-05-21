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

#include <wchar.h>

#ifndef RSTRING
#define RSTRING

struct rui_string
{
	wchar_t *s;	///<! wchar_t c-string
	size_t length; ///<! current length of s
	size_t max;	///<! maximum length of s, according to the allocated memory
};

typedef struct rui_string rstring;


enum rui_string_error_codes {RS_MEMORY, RS_OK = 1};

typedef enum rui_string_error_codes rstring_code;

/**
Creates and initializes a string
\param cstring a c-string containing the initial string text
\return a pointer to the newly created rstring
**/
rstring *rs_create(const wchar_t *cstring);

/**
Frees the memory allocated to a rstring
\param rs pointer to a pointer to a rstring
**/
void rs_destroy(rstring **rs);

/**
Creates a duplicate string of copied
\param copied the string that will be duplicated
\return a duplicate of copied, NULL if the memory allocation failed
**/
rstring *rs_duplicate(rstring *copied);

/** Returns the length of a rstring
\param rs rstring to measure
\return the length of the string
**/
size_t rs_length(rstring *rs);

/** Copies a rstring into another rstring
\param to rstring where to copy to
\param from rstring where to copy from
\return result
**/
int rs_copyrs(rstring *to, const rstring *from);

/** Copies a wchar_t string into a rstring
\param to rstring where to copy to
\param from wchar_t string where to copy from
\param length the length to be copied
\return result
**/
int rs_copywcs(rstring *to, const wchar_t *from, const size_t length);

/** Concatenates a rstring onto the end of a rstring
\param pre rstring where to append to
\param pos rtring where to append from
\return result
**/
int rs_catrs(rstring *pre, const rstring *pos);

/** Concatenates a wchar_t string onto the end of a rstring
\param pre rstring where to append to
\param pos wchar_t string where to append from
\param length the length to be copied
\return result
**/
int rs_catwcs(rstring *pre, const wchar_t *pos, const size_t length);

/** Concatenates a single wchar_t onto the end of a rstring
\param pre rstring where to append to
\param c wchar_t to be appended
\return result
**/
int rs_catwc(rstring *pre, const wchar_t c);

/** Concatenates a single char onto the end of a rstring
\param pre rstring where to append to
\param c char to be appended
\return result
**/
int rs_catc(rstring *pre, const char c);

/** Wraps a wchar_t string with a rstring structure, in order to offer a higher level string handling
\param wcs wchar_t structure which will be wrapped
\return a rstring structure with the s pointer pointing towards wcs
**/
rstring *rs_wrap(wchar_t *wcs);

/** Unwraps a rstring from it's wchar_t string. Returns a pointer to the wchar_t string and frees everything else
\param rs the rstring structure to be unwrapped and freed
\return a wchar_t string containing the text
**/
wchar_t *rs_unwrap(rstring *rs);


#endif
