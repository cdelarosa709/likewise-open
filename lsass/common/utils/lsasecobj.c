/*
 * Copyright Likewise Software
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

#include "includes.h"

VOID
LsaUtilFreeSecurityObject(
    PLSA_SECURITY_OBJECT pObject
    )
{
    if (pObject)
    {
        LW_SAFE_FREE_STRING(pObject->pszDN);
        LW_SAFE_FREE_STRING(pObject->pszObjectSid);
        LW_SAFE_FREE_STRING(pObject->pszNetbiosDomainName);
        LW_SAFE_FREE_STRING(pObject->pszSamAccountName);

        switch(pObject->type)
        {
        case AccountType_Group:
            LW_SAFE_FREE_STRING(pObject->groupInfo.pszAliasName);
            LW_SAFE_FREE_STRING(pObject->groupInfo.pszUnixName);
            LW_SAFE_FREE_STRING(pObject->groupInfo.pszPasswd);
            break;
        case AccountType_User:
            LW_SAFE_FREE_STRING(pObject->userInfo.pszPrimaryGroupSid);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszUPN);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszAliasName);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszUnixName);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszPasswd);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszGecos);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszShell);
            LW_SAFE_FREE_STRING(pObject->userInfo.pszHomedir);
            LW_SAFE_FREE_MEMORY(pObject->userInfo.pLmHash);
            LW_SAFE_FREE_MEMORY(pObject->userInfo.pNtHash);
            break;
        }
    }
}

void
LsaUtilFreeSecurityObjectList(
    DWORD dwCount,
    PLSA_SECURITY_OBJECT* ppObjectList
    )
{
    DWORD dwIndex = 0;

    if (ppObjectList)
    {
        for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
            LsaUtilFreeSecurityObject(ppObjectList[dwIndex]);
        }

        LwFreeMemory(ppObjectList);
    }
}
