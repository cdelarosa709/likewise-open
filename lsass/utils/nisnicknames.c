/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        nisnicknames.c
 *
 * Abstract:
 *
 *        Likewise Security and Authentication Subsystem (LSASS)
 *
 *        NIS Nicknames
 *
 * Authors: Manny Vellon (mvellon@likewisesoftware.com)
 *          Sriram Nambakam (snambakam@likewisesoftware.com)
 */
#include "includes.h"

static
VOID
LsaNISFreeNicknameInList(
    PVOID pItem,
    PVOID pUserData
    );

static
VOID
LsaNISFreeNickname(
    PLSA_NIS_NICKNAME pNickname
    );

DWORD
LsaNISGetNicknames(
    PDLINKEDLIST* ppNicknameList
    )
{
    typedef enum
    {
        NIS_NICKNAME_ALIAS = 0,
        NIS_NICKNAME_NAME
    } NISNicknameTokenType;
    DWORD dwError = 0;
    PDLINKEDLIST pNicknameList = NULL;
    PCSTR pszNicknameFilePath = "/var/yp/nicknames";
    BOOLEAN bFileExists = FALSE;
    PLSA_NIS_NICKNAME pNickname = NULL;
    FILE* fp = NULL;

    dwError = LsaCheckFileExists(
                    pszNicknameFilePath,
                    &bFileExists);
    BAIL_ON_LSA_ERROR(dwError);

    if (!bFileExists)
    {
        dwError = ENOENT;
        BAIL_ON_LSA_ERROR(dwError);
    }

    fp = fopen(pszNicknameFilePath, "r");
    if (!fp)
    {
        dwError = errno;
        BAIL_ON_LSA_ERROR(dwError);
    }

    while (1)
    {
        CHAR szBuf[1024+1];
        PSTR pszToken = NULL;
        PSTR pszSavePtr = NULL;
        PCSTR pszDelim = " \t\r\n";
        NISNicknameTokenType nextTokenType = NIS_NICKNAME_ALIAS;

        szBuf[0] = '\0';

        if (fgets(szBuf, 1024, fp) == NULL)
        {
            if (feof(fp))
            {
                break;
            }
            else
            {
                dwError = errno;
                BAIL_ON_LSA_ERROR(dwError);
            }
        }

        LsaStripWhitespace(szBuf, TRUE, TRUE);

        if (!szBuf[0] || (szBuf[0] == '#'))
        {
            // skip comments
            continue;
        }

        // The alias and map name can occur on the same line or
        // on consecutive lines
        pszToken = strtok_r(szBuf, pszDelim, &pszSavePtr);

        if (nextTokenType == NIS_NICKNAME_ALIAS)
        {
            dwError = LsaAllocateMemory(
                            sizeof(LSA_NIS_NICKNAME),
                            (PVOID*)&pNickname);
            BAIL_ON_LSA_ERROR(dwError);

            dwError = LsaAllocateString(
                            pszToken,
                            &pNickname->pszMapAlias);
            BAIL_ON_LSA_ERROR(dwError);

            pszToken = NULL;

            nextTokenType = NIS_NICKNAME_NAME;
        }

        if (nextTokenType == NIS_NICKNAME_NAME)
        {
            if (!pszToken)
            {
                pszToken = strtok_r(NULL, pszDelim, &pszSavePtr);

                if (IsNullOrEmptyString(pszToken))
                {
                    continue;
                }
            }

            dwError = LsaAllocateString(
                            pszToken,
                            &pNickname->pszMapName);
            BAIL_ON_LSA_ERROR(dwError);

            dwError = LsaDLinkedListAppend(
                            &pNicknameList,
                            pNickname);
            BAIL_ON_LSA_ERROR(dwError);

            pNickname = NULL;

            nextTokenType = NIS_NICKNAME_ALIAS;
        }
    }

    *ppNicknameList = pNicknameList;

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    if (pNickname)
    {
        LsaNISFreeNickname(pNickname);
    }

    return dwError;

error:

    *ppNicknameList = NULL;

    if (pNicknameList)
    {
        LsaNISFreeNicknameList(pNicknameList);
    }

    goto cleanup;
}

VOID
LsaNISFreeNicknameList(
    PDLINKEDLIST pNicknameList
    )
{
    LsaDLinkedListForEach(
                pNicknameList,
                &LsaNISFreeNicknameInList,
                NULL);
    LsaDLinkedListFree(pNicknameList);
}

static
VOID
LsaNISFreeNicknameInList(
    PVOID pItem,
    PVOID pUserData
    )
{
    if (pItem)
    {
        LsaNISFreeNickname((PLSA_NIS_NICKNAME)pItem);
    }
}

static
VOID
LsaNISFreeNickname(
    PLSA_NIS_NICKNAME pNickname
    )
{
    LSA_SAFE_FREE_STRING(pNickname->pszMapAlias);
    LSA_SAFE_FREE_STRING(pNickname->pszMapName);
    LsaFreeMemory(pNickname);
}

