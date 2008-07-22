/* Editor Settings: expandtabs and use 4 spaces for indentation
* ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
* -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright (C) Centeris Corporation 2004-2007
 * Copyright (C) Likewise Software    2007-2008
 * All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 2.1 of 
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this program.  If not, see 
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __CTSTRUTILS_H__
#define __CTSTRUTILS_H__

#include "ctarray.h"

CENTERROR
CTAllocateString(
    PCSTR pszInputString,
    PSTR * ppszOutputString
    );

#define CTStrdup CTAllocateString

CENTERROR
CTStrndup(
    PCSTR pszInputString,
    size_t size,
    PSTR * ppszOutputString
    );

void
CTFreeString(
    PSTR pszString
    );

#define CT_SAFE_FREE_STRING(str) \
    do { if (str) { CTFreeString(str); (str) = NULL; } } while (0)

void
CTFreeStringArray(
    PSTR * ppStringArray,
    DWORD dwCount
    );

void
CTFreeNullTerminatedStringArray(
    PSTR * ppStringArray
    );

void
CTStripLeadingWhitespace(
    PSTR pszString
    );

void
CTStripTrailingWhitespace(
    PSTR pszString
    );

void
CTRemoveLeadingWhitespacesOnly(
    PSTR pszString
    );

void
CTRemoveTrailingWhitespacesOnly(
    PSTR pszString
    );

void
CTStripWhitespace(
    PSTR pszString
    );

void
CTStrToUpper(
    PSTR pszString
    );

void
CTStrToLower(
    PSTR pszString
    );

CENTERROR
CTEscapeString(
    PCSTR pszOrig,
    PSTR * ppszEscapedString
    );

CENTERROR
CTAllocateStringPrintfV(
    PSTR* result,
    PCSTR format,
    va_list args
    );

CENTERROR
CTAllocateStringPrintf(
    PSTR* result,
    PCSTR format,
    ...
    );

/** Returns true if substr(str, 0, strlen(prefix)) == prefix
 */
BOOLEAN
CTStrStartsWith(
    PCSTR str,
    PCSTR prefix
    );

/** Returns true if substr(str, strlen(str) - strlen(suffix)) == suffix
 */
BOOLEAN
CTStrEndsWith(
    PCSTR str,
    PCSTR suffix
    );

typedef DynamicArray StringBuffer;

CENTERROR
CTStringBufferConstruct(StringBuffer* buffer);

//Sets the string to zero length, but does not free the data.
void
CTStringBufferClear(
		StringBuffer* buffer
		);

//Frees the stringbuffer object
void
CTStringBufferDestroy(
		StringBuffer* buffer
		);

//The returned string needs to be freed, but not buffer.
char*
CTStringBufferFreeze(
		StringBuffer* buffer
		);

CENTERROR
CTStringBufferAppend(
		StringBuffer* buffer,
		const char* str
		);

CENTERROR
CTStringBufferAppendLength(
		StringBuffer* buffer,
		const char* str,
		unsigned int length
		);

CENTERROR
CTStringBufferAppendChar(
		StringBuffer* buffer,
		char c
		);

BOOLEAN
CTIsAllDigit(
    PCSTR pszVal
    );

CENTERROR CTDupOrNullStr(const char *src, char **dest);

typedef struct
{
    char *value;
    char *trailingSeparator;
} CTParseToken;

void CTFreeParseTokenContents(CTParseToken *token);
void CTFreeParseToken(CTParseToken **token);
/* Returns the length of the value and trailingSeparator */
size_t CTGetTokenLen(const CTParseToken *token);
/* Does not enlarge pos. Does not null terminate.
 * Make sure pos already has enough space. Use GetTokenLen to calculate
 * necessary length */
void CTAppendTokenString(char **pos, const CTParseToken *token);
/* Read a token from *pos, and update pos to point to the first character not
 * included in the token.
 *
 * Token reading stops when a character inside of includeSeparators or
 * excludeSeparators is found. If the character is inside of
 * includeSeparators, it and any subsequent characters in includeSeparators
 * will be added in the trailingSeparator string of the token.
 *
 * After the first separator is found, the value portion of the token will be
 * trimmed on the right side. Any token inside of trimBack will be trimmed.
 * The trimmed characters will be included in the trailingSeparator.
 */
CENTERROR CTReadToken(const char **pos, CTParseToken *store, const char *includeSeparators, const char *excludeSeparators, const char *trimBack);
CENTERROR CTCopyTokenContents(CTParseToken *dest, const CTParseToken *source);
CENTERROR CTCopyToken(const CTParseToken *source, CTParseToken **dest);

CENTERROR CTWriteToken(FILE *file, CTParseToken *token);

CENTERROR CTGetTerminalWidth(int terminalFid, int *width);
CENTERROR CTWordWrap(PCSTR input, PSTR *output, int tabWidth, int columns);

#endif /* __CTSTRUTILS_H__ */

