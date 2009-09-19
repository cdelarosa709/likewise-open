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

#include "domainjoin.h"
#include "djdaemonmgr.h"
#include <libxml/xpath.h>
#include <libxml/parser.h>

#define GCE(x) GOTO_CLEANUP_ON_CENTERROR((x))

// aka: CENTERROR_LICENSE_INCORRECT
// static DWORD GPAGENT_LICENSE_ERROR = 0x00002001;

// CENTERROR_LICENSE_EXPIRED
// static DWORD GPAGENT_LICENSE_EXPIRED_ERROR = 0x00002002;

// static PSTR pszAuthdStartPriority = "90";
// static PSTR pszAuthdStopPriority = "10";
// static PSTR pszGPAgentdStartPriority = "91";
// static PSTR pszGPAgentdStopPriority = "9";

// Runs an xpath expression on an xml file. If the resultant nodeset contains
// exactly one text node, it is returned through result, otherwise
// CENTERROR_CFG_VALUE_NOT_FOUND or CENTERROR_INVALID_VALUE is returned.
static CENTERROR GetXPathString(PCSTR file, PSTR *result, PCSTR expression)
{
    xmlDocPtr xmlDoc = NULL;
    xmlXPathContextPtr xpathCtx = NULL; 
    xmlXPathObjectPtr xpathObj = NULL; 
    CENTERROR ceError = CENTERROR_SUCCESS;
    BOOLEAN bExists = FALSE;

    *result = NULL;

    GCE(ceError = CTCheckFileExists(file, &bExists));
    if (bExists == FALSE)
        GCE(ceError = CENTERROR_DOMAINJOIN_INVALID_FORMAT);

    xmlDoc = xmlReadFile(file, NULL, XML_PARSE_NONET | XML_PARSE_NOERROR);
    if(xmlDoc == NULL)
        GCE(ceError = CENTERROR_DOMAINJOIN_INVALID_FORMAT);

    xpathCtx = xmlXPathNewContext(xmlDoc);
    if(xpathCtx == NULL)
        GCE(ceError = CENTERROR_OUT_OF_MEMORY);

    xpathObj = xmlXPathEvalExpression((xmlChar*)expression, xpathCtx);

    if(xpathObj == NULL)
        GCE(ceError = CENTERROR_DOMAINJOIN_INVALID_FORMAT);

    if(xpathObj->type != XPATH_NODESET)
        GCE(ceError = CENTERROR_INVALID_VALUE);
    if(xpathObj->nodesetval == NULL)
        GCE(ceError = CENTERROR_CFG_VALUE_NOT_FOUND);
    if(xpathObj->nodesetval->nodeNr < 1)
        GCE(ceError = CENTERROR_CFG_VALUE_NOT_FOUND);
    if(xpathObj->nodesetval->nodeNr > 1 ||
            xpathObj->nodesetval->nodeTab[0]->type != XML_TEXT_NODE)
    {
        GCE(ceError = CENTERROR_INVALID_VALUE);
    }
    GCE(ceError = CTStrdup((PCSTR)xpathObj->nodesetval->nodeTab[0]->content, result));

cleanup:
    if(xpathObj != NULL)
        xmlXPathFreeObject(xpathObj);
    if(xpathCtx != NULL)
        xmlXPathFreeContext(xpathCtx);
    if(xmlDoc != NULL)
        xmlFreeDoc(xmlDoc);
    return ceError;
}

static CENTERROR DJDaemonLabelToConfigFile(PSTR *configFile, PCSTR dirName, PCSTR label)
{
    DIR *dir = NULL;
    PSTR filePath = NULL;
    struct dirent *dirEntry = NULL;
    PSTR fileLabel = NULL;
    CENTERROR ceError = CENTERROR_SUCCESS;

    *configFile = NULL;

    if ((dir = opendir(dirName)) == NULL) {
        GCE(ceError = CTMapSystemError(errno));
    }

    while(1)
    {
        errno = 0;
        dirEntry = readdir(dir);
        if(dirEntry == NULL)
        {
            if(errno != 0)
                GCE(ceError = CTMapSystemError(errno));
            else
            {
                //No error here. We simply read the last entry
                break;
            }
        }
        if(dirEntry->d_name[0] == '.')
            continue;

        CT_SAFE_FREE_STRING(filePath);
        GCE(ceError = CTAllocateStringPrintf(&filePath, "%s/%s",
                    dirName, dirEntry->d_name));

        CT_SAFE_FREE_STRING(fileLabel);
        ceError = GetXPathString(filePath, &fileLabel,
            "/plist/dict/key[text()='Label']/following-sibling::string[position()=1]/text()");
        if(!CENTERROR_IS_OK(ceError))
        {
            DJ_LOG_INFO("Cannot read daemon label from '%s' file. Error code %X. Ignoring it.", filePath, ceError);
            ceError = CENTERROR_SUCCESS;
            continue;
        }
        if(!strcmp(fileLabel, label))
        {
            //This is a match
            *configFile = filePath;
            filePath = NULL;
            goto cleanup;
        }
    }

    GCE(ceError = CENTERROR_DOMAINJOIN_MISSING_DAEMON);

cleanup:
    CT_SAFE_FREE_STRING(fileLabel);
    CT_SAFE_FREE_STRING(filePath);
    if(dir != NULL)
    {
        closedir(dir);
    }
    return ceError;
}

void
DJWaitDaemonStatus(
    PCSTR pszDaemonPath,
    BOOLEAN bStatus,
    LWException **exc
    )
{
    int count;
    int retry = 5;
    BOOLEAN bStarted = FALSE;

    for (count = 0; count < retry; count++) {

        LW_TRY(exc, DJGetDaemonStatus(pszDaemonPath, &bStarted, &LW_EXC));

        if (bStarted == bStatus) {
            break;
        }

        /* The Mac daemons may take a little longer to startup as they need to
           sequence a few conditions :
           1) netlogond needs to verify that the hostname is not set to localhost, not likely by
              by the time the user is logged on and running Domain Join tools.
           2) netlogon needs to wait about 5 seconds after testing hostname to wait for the resolver
              libraries to become usable. This is a fixed startup cost to the netlogon daemon, and it
              affects non-boot up time start operations also.
           4) netlogond then creates the PID file for itself.
           5) lsassd needs to stagger its start time to wait for netlogond to be online, it does this by
              similarly testing the hostname, and then waiting about 10 seconds before commencing.
           6) lsassd will then create its PID file and load the configured provider plugins. */

        sleep(5);
    }

    if (bStarted != bStatus) {

        if(bStatus)
        {
            LW_RAISE_EX(exc, CENTERROR_DOMAINJOIN_INCORRECT_STATUS, "Unable to start daemon", "An attempt was made to start the '%s' daemon, but querying its status revealed that it did not start.", pszDaemonPath);
        }
        else
        {
            LW_RAISE_EX(exc, CENTERROR_DOMAINJOIN_INCORRECT_STATUS, "Unable to stop daemon", "An attempt was made to stop the '%s' daemon, but querying its status revealed that it did not stop.", pszDaemonPath);
        }
        goto cleanup;
    }
cleanup:
    // NO-OP
    ;
}

void
DJGetDaemonStatus(
    PCSTR pszDaemonPath,
    PBOOLEAN pbStarted,
    LWException **exc
    )
{
    PSTR* ppszArgs = NULL;
    DWORD nArgs = 7;
    long status = 0;
    PPROCINFO pProcInfo = NULL;
    CHAR  szBuf[1024+1];
    FILE* fp = NULL;
    CENTERROR ceError;
    PSTR configFile = NULL;
    PSTR command = NULL;
    PSTR command2 = NULL;
    int argNum = 0;
    PSTR whitePos = NULL;
    PSTR wrapPos = NULL;

    /* Translate the Unix daemon names into the mac daemon names */
    if(!strcmp(pszDaemonPath, "lsassd"))
        pszDaemonPath = "com.likewisesoftware.lsassd";
    else if(!strcmp(pszDaemonPath, "gpagentd"))
        pszDaemonPath = "com.likewisesoftware.gpagentd";
    else if (!strcmp(pszDaemonPath, "lwmgmtd"))
        pszDaemonPath = "com.likewisesoftware.lwmgmtd";
    else if (!strcmp(pszDaemonPath, "eventfwdd"))
        pszDaemonPath = "com.likewisesoftware.eventfwdd";
    else if (!strcmp(pszDaemonPath, "reapsysld"))
        pszDaemonPath = "com.likewisesoftware.reapsysld";

    /* Find the .plist file for the daemon */
    ceError = DJDaemonLabelToConfigFile(&configFile, "/Library/LaunchDaemons", pszDaemonPath);
    if(ceError == CENTERROR_DOMAINJOIN_MISSING_DAEMON)
        ceError = DJDaemonLabelToConfigFile(&configFile, SYSCONFDIR "/LaunchDaemons", pszDaemonPath);
    if(ceError == CENTERROR_DOMAINJOIN_MISSING_DAEMON)
    {
        DJ_LOG_ERROR("Checking status of daemon [%s] failed: Missing", pszDaemonPath);
        LW_RAISE_EX(exc, CENTERROR_DOMAINJOIN_MISSING_DAEMON, "Unable to find daemon plist file", "The plist file for the '%s' daemon could not be found in /Library/LaunchDaemons or " SYSCONFDIR "/LaunchDaemons .", pszDaemonPath);
        goto cleanup;
    }
    LW_CLEANUP_CTERR(exc, ceError);

    DJ_LOG_INFO("Found config file [%s] for daemon [%s]", configFile, pszDaemonPath);

    /* Figure out the daemon binary by reading the command from the plist file
     */
    LW_CLEANUP_CTERR(exc, GetXPathString(configFile, &command,
        "/plist/dict/key[text()='ProgramArguments']/following-sibling::array[position()=1]/string[position()=1]/text()"));

    DJ_LOG_INFO("Found daemon binary [%s] for daemon [%s]", command, pszDaemonPath);

    /* Need to special case those daemons that are running in a wrapper script. The ProgramArguments
       will say the daemon is /opt/likewise/sbin/<daemon>-wrap, but the actual daemon will be
       exec'd from the wrapper as the name without -wrap. So we need to check for the name in this
       form also. */
    LW_CLEANUP_CTERR(exc, CTAllocateString(command, &command2));
    wrapPos = strstr(command2, "-wrap");
    if (wrapPos)
    {
        *wrapPos = '\0';
        DJ_LOG_INFO("Found daemon binary (alt) [%s] for daemon [%s]", command2, pszDaemonPath);
    }

    DJ_LOG_INFO("Checking status of daemon [%s]", pszDaemonPath);

    LW_CLEANUP_CTERR(exc, CTAllocateMemory(sizeof(PSTR)*nArgs, (PVOID*)&ppszArgs));

    LW_CLEANUP_CTERR(exc, CTAllocateString("/bin/ps", ppszArgs + argNum++));

    LW_CLEANUP_CTERR(exc, CTAllocateString("-U", ppszArgs + argNum++));

    LW_CLEANUP_CTERR(exc, CTAllocateString("root", ppszArgs + argNum++));

    LW_CLEANUP_CTERR(exc, CTAllocateString("-o", ppszArgs + argNum++));

    LW_CLEANUP_CTERR(exc, CTAllocateString("command=", ppszArgs + argNum++));

    LW_CLEANUP_CTERR(exc, DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo));

    LW_CLEANUP_CTERR(exc, DJGetProcessStatus(pProcInfo, &status));

    *pbStarted = FALSE;
    if (!status) {

        fp = fdopen(pProcInfo->fdout, "r");
        if (!fp) {
            LW_CLEANUP_CTERR(exc, CTMapSystemError(errno));
        }

        while (1) {

            if (fgets(szBuf, 1024, fp) == NULL) {
                if (!feof(fp)) {
                    LW_CLEANUP_CTERR(exc, CTMapSystemError(errno));
                }
                else
                    break;
            }

            CTStripWhitespace(szBuf);

            if (IsNullOrEmptyString(szBuf))
                continue;

            whitePos = strchr(szBuf, ' ');
            if(whitePos != NULL)
                *whitePos = '\0';

            if (!strcmp(szBuf, command)) {
                *pbStarted = TRUE;
                break;
            }

            if (!strcmp(szBuf, command2)) {
                *pbStarted = TRUE;
                break;
            }
        }
    }

cleanup:

    if (fp)
        fclose(fp);

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);

    CT_SAFE_FREE_STRING(configFile);
    CT_SAFE_FREE_STRING(command);
    CT_SAFE_FREE_STRING(command2);
}

void
DJStartStopDaemon(
    PCSTR pszDaemonPath,
    BOOLEAN bStatus,
    LWException **exc
    )
{
    PSTR* ppszArgs = NULL;
    DWORD nArgs = 4;
    long status = 0;
    PPROCINFO pProcInfo = NULL;

    if (bStatus) {
        DJ_LOG_INFO("Starting daemon [%s]", pszDaemonPath);
    } else {
        DJ_LOG_INFO("Stopping daemon [%s]", pszDaemonPath);
    }

    LW_CLEANUP_CTERR(exc, CTAllocateMemory(sizeof(PSTR)*nArgs, (PVOID*)&ppszArgs));
    LW_CLEANUP_CTERR(exc, CTAllocateString("/bin/launchctl", ppszArgs));

    if (bStatus)
    {
       LW_CLEANUP_CTERR(exc, CTAllocateString("start", ppszArgs+1));
    }
    else
    {
       LW_CLEANUP_CTERR(exc, CTAllocateString("stop", ppszArgs+1));
    }

    LW_CLEANUP_CTERR(exc, CTAllocateString(pszDaemonPath, ppszArgs+2));
    LW_CLEANUP_CTERR(exc, DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo));
    LW_CLEANUP_CTERR(exc, DJGetProcessStatus(pProcInfo, &status));

    LW_TRY(exc, DJWaitDaemonStatus(pszDaemonPath, bStatus, &LW_EXC));

cleanup:

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);
}

static
CENTERROR
DJExistsInLaunchCTL(
    PCSTR pszName,
    PBOOLEAN pbExists
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    PPROCINFO pProcInfo = NULL;
    PSTR* ppszArgs = NULL;
    LONG  status = 0;
    DWORD nArgs = 3;
    FILE* fp = NULL;
    CHAR szBuf[1024+1];
    BOOLEAN bExists = FALSE;

    ceError = CTAllocateMemory(nArgs*sizeof(PSTR), (PVOID*)&ppszArgs);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTAllocateString("/bin/launchctl", ppszArgs);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTAllocateString("list", ppszArgs+1);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = DJGetProcessStatus(pProcInfo, &status);
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (status != 0) {
        ceError = CENTERROR_DOMAINJOIN_LISTSVCS_FAILED;
        BAIL_ON_CENTERIS_ERROR(ceError);
    }

    fp = fdopen(pProcInfo->fdout, "r");
    if (fp == NULL) {
        ceError = CTMapSystemError(errno);
        BAIL_ON_CENTERIS_ERROR(ceError);
    }

    for(;;)
    {
        if (fgets(szBuf, 1024, fp) == NULL)
        {
            if (feof(fp))
            {
                break;
            }
            else
            {
                ceError = CTMapSystemError(errno);
                BAIL_ON_CENTERIS_ERROR(ceError);
            }
        }

        CTStripWhitespace(szBuf);

        if (!strcmp(szBuf, pszName))
        {
            bExists = TRUE;
            break;
        }
    }

error:

    *pbExists = bExists;

    if (fp)
        fclose(fp);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    return ceError;
}

static
CENTERROR
DJPrepareServiceLaunchScript(
    PCSTR pszName
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    PPROCINFO pProcInfo = NULL;
    PSTR* ppszArgs = NULL;
    LONG  status = 0;
    DWORD nArgs = 5;
    CHAR szBuf[1024+1];
    BOOLEAN bFileExists = FALSE;

    ceError = CTAllocateMemory(nArgs*sizeof(PSTR), (PVOID*)&ppszArgs);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTAllocateString("/bin/cp", ppszArgs);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = CTAllocateString("-f", ppszArgs+1);
    BAIL_ON_CENTERIS_ERROR(ceError);

    sprintf(szBuf, SYSCONFDIR "/LaunchDaemons/%s.plist", pszName);
    ceError = CTCheckFileExists(szBuf, &bFileExists);  
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (!bFileExists)
	BAIL_ON_CENTERIS_ERROR(ceError = CENTERROR_DOMAINJOIN_MISSING_DAEMON);

    ceError = CTAllocateString(szBuf, ppszArgs+2);
    BAIL_ON_CENTERIS_ERROR(ceError);

    sprintf(szBuf, "/Library/LaunchDaemons/%s.plist", pszName);
    ceError = CTAllocateString(szBuf, ppszArgs+3);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo);
    BAIL_ON_CENTERIS_ERROR(ceError);

    ceError = DJGetProcessStatus(pProcInfo, &status);
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (status != 0) {
        ceError = CENTERROR_DOMAINJOIN_PREPSVC_FAILED;
        BAIL_ON_CENTERIS_ERROR(ceError);
    }

error:

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);

    return ceError;
}

static
CENTERROR
DJRemoveFromLaunchCTL(
    PCSTR pszName
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    BOOLEAN bExistsInLaunchCTL = FALSE;
    CHAR    szBuf[1024+1];
    PPROCINFO pProcInfo = NULL;
    PSTR* ppszArgs = NULL;
    DWORD nArgs = 4;
    LONG  status = 0;
    LWException* innerException = NULL;

    ceError = DJExistsInLaunchCTL(pszName, &bExistsInLaunchCTL);
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (bExistsInLaunchCTL) {

        sprintf(szBuf, "/Library/LaunchDaemons/%s.plist", pszName);

        ceError = CTAllocateMemory(sizeof(PSTR)*nArgs, (PVOID*)&ppszArgs);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString("/bin/launchctl", ppszArgs);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString("unload", ppszArgs+1);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString(szBuf, ppszArgs+2);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = DJGetProcessStatus(pProcInfo, &status);
        BAIL_ON_CENTERIS_ERROR(ceError);

        if (status != 0) {
            ceError = CENTERROR_DOMAINJOIN_SVC_UNLOAD_FAILED;
            BAIL_ON_CENTERIS_ERROR(ceError);
        }

        DJWaitDaemonStatus(pszName, FALSE, &innerException);
        if (innerException)
        {
            ceError = innerException->code;
            BAIL_ON_CENTERIS_ERROR(ceError);
        }

        sprintf(szBuf, "/Library/LaunchDaemons/%s.plist", pszName);
        ceError = CTRemoveFile(szBuf);
        if (!CENTERROR_IS_OK(ceError)) {
            DJ_LOG_WARNING("Failed to remove file [%s]", szBuf);
            ceError = CENTERROR_SUCCESS;
        }
    }

error:
    LW_HANDLE(&innerException);

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);

    return ceError;
}

static
CENTERROR
DJAddToLaunchCTL(
    PCSTR pszName
    )
{
    CENTERROR ceError = CENTERROR_SUCCESS;
    BOOLEAN bExistsInLaunchCTL = FALSE;
    BOOLEAN bFileExists = FALSE;
    CHAR    szBuf[1024+1];
    PPROCINFO pProcInfo = NULL;
    PSTR* ppszArgs = NULL;
    DWORD nArgs = 4;
    LONG  status = 0;

    memset(szBuf, 0, sizeof(szBuf));
    sprintf(szBuf, "/Library/LaunchDaemons/%s.plist", pszName);

    ceError = CTCheckFileExists(szBuf, &bFileExists);
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (!bFileExists) {

        ceError = DJPrepareServiceLaunchScript(pszName);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTCheckFileExists(szBuf, &bFileExists);
        BAIL_ON_CENTERIS_ERROR(ceError);

        if (!bFileExists) {
            ceError = CENTERROR_DOMAINJOIN_MISSING_DAEMON;
            BAIL_ON_CENTERIS_ERROR(ceError);
        }
    }

    ceError = DJExistsInLaunchCTL(pszName, &bExistsInLaunchCTL);
    BAIL_ON_CENTERIS_ERROR(ceError);

    if (!bExistsInLaunchCTL)
    {
        sprintf(szBuf, "/Library/LaunchDaemons/%s.plist", pszName);

        ceError = CTAllocateMemory(sizeof(PSTR)*nArgs, (PVOID*)&ppszArgs);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString("/bin/launchctl", ppszArgs);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString("load", ppszArgs+1);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = CTAllocateString(szBuf, ppszArgs+2);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = DJSpawnProcess(ppszArgs[0], ppszArgs, &pProcInfo);
        BAIL_ON_CENTERIS_ERROR(ceError);

        ceError = DJGetProcessStatus(pProcInfo, &status);
        BAIL_ON_CENTERIS_ERROR(ceError);

        if (status != 0) {
            ceError = CENTERROR_DOMAINJOIN_SVC_LOAD_FAILED;
            BAIL_ON_CENTERIS_ERROR(ceError);
        }
    }

error:

    if (ppszArgs)
        CTFreeStringArray(ppszArgs, nArgs);

    if (pProcInfo)
        FreeProcInfo(pProcInfo);

    return ceError;
}

void
DJManageDaemon(
    PCSTR pszName,
    BOOLEAN bStatus,
    int startPriority,
    int stopPriority,
    LWException **exc
    )
{
    BOOLEAN bStarted = FALSE;
    const char* pszDaemonPath = pszName;

    DJ_LOG_INFO("Processing call to DJManageDaemon('%s',%s)", pszDaemonPath, bStatus ? "start" : "stop");

    /* Translate the Unix daemon names into the mac daemon names */
    if(!strcmp(pszName, "likewise-open"))
        pszDaemonPath = "com.likewise.open";
    else if(!strcmp(pszName, "gpagentd"))
        pszDaemonPath = "com.likewisesoftware.gpagentd";
    else if(!strcmp(pszName, "lsassd"))
        pszDaemonPath = "com.likewisesoftware.lsassd";
    else if(!strcmp(pszName, "netlogond"))
        pszDaemonPath = "com.likewisesoftware.netlogond";
    else if(!strcmp(pszName, "lwiod"))
        pszDaemonPath = "com.likewisesoftware.lwiod";
    else if(!strcmp(pszName, "eventlogd"))
        pszDaemonPath = "com.likewisesoftware.eventlogd";
    else if(!strcmp(pszName, "dcerpcd"))
        pszDaemonPath = "com.likewisesoftware.dcerpcd";
    else if (!strcmp(pszName, "lwmgmtd"))
        pszDaemonPath = "com.likewisesoftware.lwmgmtd";
    else if (!strcmp(pszName, "srvsvcd"))
        pszDaemonPath = "com.likewisesoftware.srvsvcd";
    else if (!strcmp(pszName, "eventfwdd"))
        pszDaemonPath = "com.likewisesoftware.eventfwdd";
    else if (!strcmp(pszName, "reapsysld"))
        pszDaemonPath = "com.likewisesoftware.reapsysld";
    else if (!strcmp(pszName, "lwregd"))
        pszDaemonPath = "com.likewisesoftware.lwregd";
    else
        pszDaemonPath = pszName;

    DJ_LOG_INFO("DJManageDaemon('%s',%s) after name fixup", pszDaemonPath, bStatus ? "start" : "stop");

    if (bStatus)
    {
        LW_CLEANUP_CTERR(exc, DJAddToLaunchCTL(pszDaemonPath));
    }

    // check our current state prior to doing anything.  notice that
    // we are using the private version so that if we fail, our inner
    // exception will be the one that was tossed due to the failure.
    LW_TRY(exc, DJGetDaemonStatus(pszDaemonPath, &bStarted, &LW_EXC));

    // if we got this far, we have validated the existence of the
    // daemon and we have figured out if its started or stopped

    // if we are already in the desired state, do nothing.
    if (bStarted != bStatus) {

        LW_TRY(exc, DJStartStopDaemon(pszDaemonPath, bStatus, &LW_EXC));

    }
    else
    {
        DJ_LOG_INFO("daemon '%s' is already %s", pszDaemonPath, bStarted ? "started" : "stopped");
    }

    if (!bStatus)
    {
        // Daemon is no longer needed, we are leaving the domain.
        LW_CLEANUP_CTERR(exc, DJRemoveFromLaunchCTL(pszDaemonPath));
    }

cleanup:
    ;
}

void
DJManageDaemonDescription(
    PCSTR pszName,
    BOOLEAN bStatus,
    int startPriority,
    int stopPriority,
    PSTR *description,
    LWException **exc
    )
{
    BOOLEAN bStarted = FALSE;

    *description = NULL;

    LW_TRY(exc, DJGetDaemonStatus(pszName, &bStarted, &LW_EXC));

    // if we got this far, we have validated the existence of the
    // daemon and we have figured out if its started or stopped

    // if we are already in the desired state, do nothing.
    if (bStarted != bStatus) {

        if(bStatus)
        {
            LW_CLEANUP_CTERR(exc, CTAllocateStringPrintf(description,
                    "Start %s by running '/bin/launchctl start /Library/LaunchDaemons/%s.plist'.\n", pszName, pszName));
        }
        else
        {
            LW_CLEANUP_CTERR(exc, CTAllocateStringPrintf(description,
                    "Stop %s by running '/bin/launchctl unload /Library/LaunchDaemons/%s.plist'.\n", pszName, pszName));
        }
    }

cleanup:
    ;
}

struct _DaemonList daemonList[] = {
    { "com.likewisesoftware.gpagentd", {NULL}, FALSE, 94, 9 },
    { "com.likewisesoftware.lwmgmtd", {NULL}, FALSE, 95, 8 },
    //{ "com.likewisesoftware.eventfwdd", {NULL}, FALSE, 96, 7 },
    //{ "com.likewisesoftware.reapsysld", {NULL}, FALSE, 97, 6 },
    { NULL, {NULL}, FALSE, 0, 0 },
};

