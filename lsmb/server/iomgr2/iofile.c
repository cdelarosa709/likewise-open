/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software
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

#include "iop.h"

NTSTATUS
IopFileObjectAllocate(
    OUT PIO_FILE_OBJECT* ppFileObject,
    IN PIO_DEVICE_OBJECT pDevice
    )
{
    NTSTATUS status = 0;
    int EE = 0;
    PIO_FILE_OBJECT pFileObject = NULL;

    status = IO_ALLOCATE(&pFileObject, IO_FILE_OBJECT, sizeof(*pFileObject));
    GOTO_CLEANUP_ON_STATUS_EE(status, EE);

    pFileObject->ReferenceCount = 1;
    pFileObject->pDevice = pDevice;

    LwListInit(&pFileObject->IrpList);

    LwListInsertTail(&pDevice->FileObjectsList, &pFileObject->DeviceLinks);

cleanup:
    if (status)
    {
        IopFileObjectFree(&pFileObject);
    }

    *ppFileObject = pFileObject;

    IO_LOG_LEAVE_ON_STATUS_EE(status, EE);
    return status;
}

VOID
IopFileObjectFree(
    IN OUT PIO_FILE_OBJECT* ppFileObject
    )
{
    PIO_FILE_OBJECT pFileObject = *ppFileObject;

    if (pFileObject)
    {
        // TODO -- add proper drain support...
        assert(LwListIsEmpty(&pFileObject->IrpList));
        LwListRemove(&pFileObject->DeviceLinks);
        IoMemoryFree(pFileObject);
        *ppFileObject = NULL;
    }
}

NTSTATUS
IoFileSetContext(
    IN IO_FILE_HANDLE FileHandle,
    IN PVOID FileContext
    )
{
    NTSTATUS status = 0;
    int EE = 0;

    if (FileHandle->pContext)
    {
        assert(FALSE);
        status = STATUS_UNSUCCESSFUL;
        GOTO_CLEANUP_ON_STATUS_EE(status, EE);
    }

    FileHandle->pContext = FileContext;

cleanup:
    IO_LOG_LEAVE_ON_STATUS_EE(status, EE);
    return status;
}

PVOID
IoFileGetContext(
    IN IO_FILE_HANDLE FileHandle
    )
{
    return FileHandle->pContext;
}

