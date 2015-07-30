/*******************************************************************************
 * Copyright 2015 Panagiotis Anastasiadis
 * This file is part of SYLoader.
 *
 * SYLoader is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SYLoader is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#include "utility.h"
#include <QRegExp>


#ifdef _WIN32
#include <windows.h>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

LPFN_ISWOW64PROCESS fnIsWow64Process;





bool
Utility::is64Bit()
{
    BOOL bIsWow64 = FALSE;

    /* IsWow64Process is not available on all supported versions of Windows.
     * Use GetModuleHandle to get a handle to the DLL that contains the function
     * and GetProcAddress to get a pointer to the function if available.
     */
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
            return false;
        }
    }

    return bIsWow64;
}





QString
Utility::cleanFilename(QString desiredFilename)
{
    return desiredFilename.replace(QRegExp("[<>:\"/\\|?*]"), QString("-"));
}

#else

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/personality.h>





bool
Utility::is64Bit()
{
    utsname unameStruct;

    personality(PER_LINUX);
    uname(&unameStruct);

    if (strcmp(unameStruct.machine, "x86_64") == 0) {
        return true;
    }
    else {
        return false;
    }
}





QString
Utility::cleanFilename(QString desiredFilename)
{
    return desiredFilename.replace(QRegExp("[/]"), QString("-"));
}

#endif

