/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.  You should have received a copy of the GNU General
 * Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        includes
 *
 * Abstract:
 *
 *        Likewise Posix File System (SMBSS)
 *
 *        Service Entry API
 *
 * Authors: Gerald Carter <gcarter@likewise.com>
 */

#ifndef __PVFS_H__
#define __PVFS_H__

#include "config.h"

#if defined(HAVE_ATTR_XATTR_H) && \
    defined(HAVE_FSETXATTR) && \
    defined(HAVE_FGETXATTR) && \
    defined(HAVE_GETXATTR)
#  ifdef __LWI_LINUX__
#    define HAVE_EA_SUPPORT
#  endif
#endif


#include "lwiosys.h"

#include <lw/base.h>
#include <lw/security-types.h>

#include "iodriver.h"
#include "lwioutils.h"

#include "structs.h"
#include "async_handler.h"
#include "threads.h"
#include "externs.h"
#include "macros.h"
#include "fileinfo_p.h"
#include "security_p.h"
#include "create_p.h"
#include "alloc_p.h"
#include "time_p.h"
#include "syswrap_p.h"
#include "ccb_p.h"
#include "fcb.h"
#include "acl.h"
#include "attrib.h"

/* Unix (POSIX) APIs */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>

#ifdef HAVE_SYS_VFS_H
#  include <sys/vfs.h>
#endif

#ifdef HAVE_ATTR_XATTR_H
#  include <attr/xattr.h>
#endif

/* Driver defines */

#define PVFS_NUMBER_WORKER_THREADS      2


/* Top level APi functions */

NTSTATUS
PvfsCreate(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsDeviceIoControl(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsFsIoControl(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsWrite(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsRead(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsClose(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsQuerySetInformation(
    PVFS_INFO_TYPE RequestType,
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsQueryDirInformation(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsQueryVolumeInformation(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsDispatchLockControl(
    PPVFS_IRP_CONTEXT pIrpContext
    );

NTSTATUS
PvfsLockControl(
    PPVFS_IRP_CONTEXT pIrpContext
    );

NTSTATUS
PvfsFlushBuffers(
    PPVFS_IRP_CONTEXT  pIrpContext
    );

NTSTATUS
PvfsQuerySetSecurityFile(
    PVFS_INFO_TYPE RequestType,
    PPVFS_IRP_CONTEXT pIrpContext
    );


/* From errno.c */

NTSTATUS
PvfsMapUnixErrnoToNtStatus(
    int err
    );


/* From unixpath.c */

NTSTATUS
PvfsCanonicalPathName(
    PSTR *ppszPath,
    IO_FILE_NAME IoPath
    );

NTSTATUS
PvfsWC16CanonicalPathName(
    PSTR *ppszPath,
    PWSTR pwszPathname
    );

NTSTATUS
PvfsValidatePath(
    PPVFS_CCB pCcb
    );

NTSTATUS
PvfsFileBasename(
    PSTR *ppszFilename,
    PCSTR pszPath
    );
NTSTATUS
PvfsFileDirname(
    PSTR *ppszDirname,
    PCSTR pszPath
    );

NTSTATUS
PvfsFileSplitPath(
    PSTR *ppszDirname,
    PSTR *ppszBasename,
    PCSTR pszPath
    );

NTSTATUS
PvfsLookupPath(
    PSTR *ppszDiskPath,
    PCSTR pszPath,
    BOOLEAN bCaseSensitive
    );

NTSTATUS
PvfsLookupFile(
    PSTR *ppszDiskPath,
    PCSTR pszDiskDirname,
    PCSTR pszFilename,
    BOOLEAN bCaseSensitive
    );

/* From pathcache.c */

NTSTATUS
PvfsPathCacheAdd(
    IN PCSTR pszResolvedPath
    );


NTSTATUS
PvfsPathCacheLookup(
    OUT PSTR *ppResolvedPath,
    IN  PCSTR pszOriginalPath
    );


/* From wildcard.c */

BOOLEAN
PvfsWildcardMatch(
    IN PSTR pszPathname,
    IN PSTR pszPattern,
    IN BOOLEAN bCaseSensitive
    );


/* From util_open.c */

NTSTATUS
MapPosixOpenFlags(
    int *unixFlags,
    ACCESS_MASK Access,
    IRP_ARGS_CREATE CreateArgs
    );


/* From string.c */

VOID
PvfsCStringUpper(
	PSTR pszString
	);

/* From sharemode.c */

NTSTATUS
PvfsCheckShareMode(
    IN PSTR pszFilename,
    IN FILE_SHARE_FLAGS ShareAccess,
    IN ACCESS_MASK DesiredAccess,
    OUT PPVFS_FCB *ppFcb
    );

NTSTATUS
PvfsEnforceShareMode(
    IN PPVFS_FCB pFcb,
    IN FILE_SHARE_FLAGS ShareAccess,
    IN ACCESS_MASK DesiredAccess
    );

/* From locking.c */

NTSTATUS
PvfsLockFile(
    PPVFS_CCB pCcb,
    PULONG pKey,
    LONG64 Offset,
    LONG64 Length,
    PVFS_LOCK_FLAGS Flags
    );

NTSTATUS
PvfsUnlockFile(
    PPVFS_CCB pCcb,
    BOOLEAN bUnlockAll,
    PULONG pKey,
    LONG64 Offset,
    LONG64 Length
    );

NTSTATUS
PvfsCanReadWriteFile(
    PPVFS_CCB pCcb,
    PULONG pKey,
    LONG64 Offset,
    LONG64 Length,
    PVFS_LOCK_FLAGS Flags
    );

#endif /* __PVFS_H__ */


/*
local variables:
mode: c
c-basic-offset: 4
indent-tabs-mode: nil
tab-width: 4
end:
*/
