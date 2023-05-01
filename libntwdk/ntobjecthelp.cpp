//
//  Create 2012.04.05
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
extern "C" {
#include <ntifs.h>
#include "ntnativeapi.h"
#include "ntobjecthelp.h"
#include "ntstrsafe.h"
#include "ntnativehelp.h"
};

//---------------------------------------------------------------------------
//
//  LookupVolumeGuidName()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
NTSTATUS
NTAPI
LookupVolumeGuidName(
    PCWSTR DevicePath,
    PWSTR VolumeSymbolicLink,
    ULONG cchVolumeSymbolicLink
    )
{
    HANDLE hObjDir;
    LONG Status;
    ULONG Index = 0;
    WCHAR VolumeSymName[260];
    WCHAR DeviceName[260];
    UNICODE_STRING usDeviceName;
    UNICODE_STRING usDevicePath;

    RtlInitUnicodeString(&usDevicePath,DevicePath);
    if( !GetVolumeName_U( &usDevicePath ) )
        return STATUS_OBJECT_PATH_INVALID;

    Status = OpenObjectDirectory( L"\\GLOBAL??", &hObjDir );

    if( Status == STATUS_SUCCESS )
    {
        Status = RtlNtStatusToDosError( STATUS_OBJECT_NAME_NOT_FOUND );

        while( QueryObjectDirectory(hObjDir,&Index,VolumeSymName,260,NULL,0) == 0)
        {
            // Looking for a string which have prefix of "Volume{".
            // If find matched prefix, check a trail of the matched string to 
            // verify that is a GUID style.
            //
            //  0     0                                    4
            //  0     6                                    3 
            // "Volume{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
            if( _wcsnicmp(VolumeSymName,L"Volume{",7) == 0 && 
                (VolumeSymName[43] == L'}' && VolumeSymName[44] == '\0') )
            {
                usDeviceName.Length = 0;
                usDeviceName.MaximumLength = sizeof(DeviceName);
                usDeviceName.Buffer = DeviceName;

                if( QuerySymbolicLinkObject_U(hObjDir,VolumeSymName,&usDeviceName,FALSE) == 0 )
                {
                    if( RtlCompareUnicodeString(&usDevicePath,&usDeviceName,TRUE) == 0 )
                    {
                        // Symbolic link name
                        if( VolumeSymbolicLink != NULL )
                        {
                            wcsncpy_s(VolumeSymbolicLink,cchVolumeSymbolicLink,VolumeSymName,cchVolumeSymbolicLink-1);
                        }

                        Status = STATUS_SUCCESS;

                        break;
                    }
                }
            }
        }

        CloseObjectDirectory( hObjDir );
    }

    return Status;
}

//---------------------------------------------------------------------------
//
//  OpenObjectDirectory()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG
NTAPI
OpenObjectDirectory(
    PCWSTR DirectoryName,
    HANDLE *pHandle
    )
{
    NTSTATUS Status;
    HANDLE Handle;
    UNICODE_STRING Name;
    OBJECT_ATTRIBUTES ObjectAttributes;

    RtlInitUnicodeString(&Name,(PWSTR)DirectoryName);

    InitializeObjectAttributes (
                &ObjectAttributes,
                &Name,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

    Status = NtOpenDirectoryObject(
                &Handle, 
                STANDARD_RIGHTS_READ|DIRECTORY_QUERY|DIRECTORY_TRAVERSE, 
                &ObjectAttributes
                );
   
    if( NT_SUCCESS(Status) )
    {
        *pHandle = Handle;
    }

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  CloseObjectDirectory()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG
NTAPI
CloseObjectDirectory(
    HANDLE Handle
    )
{
    NTSTATUS Status;

    Status = NtClose(Handle);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  QueryObjectDirectory()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG 
NTAPI
QueryObjectDirectory(
    HANDLE Handle,
    PULONG pulIndex,
    PWSTR ObjectName,
    ULONG ObjectNameLength,
    PWSTR TypeName,
    ULONG TypeNameLength
    )
{
    NTSTATUS Status;
    ULONG cb = 0;
    LONG hr = 0;
    
    OBJECT_DIRECTORY_INFORMATION *odi = NULL;
    cb = 0;

    Status = NtQueryDirectoryObject(Handle,NULL,0,TRUE,FALSE,pulIndex,&cb);
    while( STATUS_BUFFER_TOO_SMALL == Status )
    {
        if( odi != NULL )
            FreeMemory(odi);

        odi = (OBJECT_DIRECTORY_INFORMATION*)AllocMemory(cb);
        if( odi == NULL )
        {
            Status = STATUS_NO_MEMORY;
            break;
        }

        Status = NtQueryDirectoryObject(Handle,odi,cb,TRUE,FALSE,pulIndex,&cb);
    }

    if( Status == STATUS_SUCCESS )
    {
        if( ObjectName )
            RtlStringCchCopyW(ObjectName,ObjectNameLength,odi->Name.Buffer);
        if( TypeName )
            RtlStringCchCopyW(TypeName,TypeNameLength,odi->TypeName.Buffer);
    }

    if( odi != NULL )
        FreeMemory(odi);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  QueryObjectDirectory_U()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG 
NTAPI
QueryObjectDirectory_U(
    HANDLE Handle,
    PULONG pulIndex,
    UNICODE_STRING *ObjectName,
    UNICODE_STRING *TypeName,
    BOOLEAN AllocateString
    )
{
    NTSTATUS Status;
    ULONG cb = 0;
    ULONG hr = 0;
    
    OBJECT_DIRECTORY_INFORMATION *odi = NULL;
    cb = 0;

    Status = NtQueryDirectoryObject(Handle,NULL,0,TRUE,FALSE,pulIndex,&cb);
    while( STATUS_BUFFER_TOO_SMALL == Status )
    {
        if( odi != NULL )
            FreeMemory(odi);

        odi = (OBJECT_DIRECTORY_INFORMATION*)AllocMemory(cb);
        if( odi == NULL )
        {
            Status = STATUS_NO_MEMORY;
            break;
        }

        Status = NtQueryDirectoryObject(Handle,odi,cb,TRUE,FALSE,pulIndex,&cb);
    }

    if( Status == STATUS_SUCCESS )
    {
        if( AllocateString )
        {
            if( ObjectName )
                Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,ObjectName,&odi->Name);
                    
            if( Status == STATUS_SUCCESS )
            {
                if( TypeName )
                    Status = RtlDuplicateUnicodeString(RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,TypeName,&odi->TypeName);
            }
        }
        else
        {
            if( ObjectName )
            {
                RtlCopyUnicodeString(ObjectName,&odi->Name);
                if( ObjectName->Length != odi->Name.Length )
                    Status = STATUS_BUFFER_TOO_SMALL;
            }
            if( TypeName )
            {
                RtlCopyUnicodeString(TypeName,&odi->TypeName);
                if( TypeName->Length != odi->TypeName.Length )
                    Status = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

    if( odi != NULL )
        FreeMemory(odi);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  QuerySymbolicLinkObject_U()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObject_U(
    HANDLE hObjectDirectory,
    PCWSTR pszSymbolicLinkName,
    UNICODE_STRING *ObjectName,
    BOOLEAN AllocateNameString
    )
{
    HANDLE hObject;
    LONG Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Name;

    RtlInitUnicodeString(&Name,pszSymbolicLinkName);

    InitializeObjectAttributes(
                &ObjectAttributes,
                &Name,
                OBJ_CASE_INSENSITIVE,
                hObjectDirectory,
                NULL);

    Status = NtOpenSymbolicLinkObject(
                            &hObject, 
                            SYMBOLIC_LINK_QUERY, 
                            &ObjectAttributes);

    if( Status != STATUS_SUCCESS )
        return RtlNtStatusToDosError(Status);

    ULONG cb = 0;

    if( !AllocateNameString )
    {
        Status = NtQuerySymbolicLinkObject(hObject,ObjectName,&cb);
    }
    else
    {
        UNICODE_STRING usTemp = {0,0,NULL};
        
        Status = NtQuerySymbolicLinkObject(hObject,&usTemp,&cb);

        while( STATUS_BUFFER_TOO_SMALL == Status )
        {
            if( usTemp.Buffer != NULL )
                FreeMemory(usTemp.Buffer);

            usTemp.MaximumLength = (USHORT)cb;
            usTemp.Buffer = (PWSTR)AllocMemory(cb);

            if( usTemp.Buffer == NULL )
            {
                Status = STATUS_NO_MEMORY;
                break;
            }

            Status = NtQuerySymbolicLinkObject(hObject,&usTemp,&cb);
        }

        if( Status == STATUS_SUCCESS )
        {
            *ObjectName = usTemp;
        }
        else
        {
            if( usTemp.Buffer != NULL )
                FreeMemory(usTemp.Buffer);
        }
    }

    NtClose(hObject);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  QuerySymbolicLinkObject()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObject(
    HANDLE hRootDirectory,
    PCWSTR pszSymbolicName,
	UNICODE_STRING *pusSymbolicName OPTIONAL,
    PWSTR ObjectName,
    int cchObjectName
    )
{
    HANDLE hObject;
    LONG Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Name;

	if( pusSymbolicName )
		Name = *pusSymbolicName;
	else
		RtlInitUnicodeString(&Name,pszSymbolicName);

    InitializeObjectAttributes(
                &ObjectAttributes,
                &Name,
                OBJ_CASE_INSENSITIVE,
                hRootDirectory,
                NULL);

    Status = NtOpenSymbolicLinkObject(
                            &hObject, 
                            SYMBOLIC_LINK_QUERY, 
                            &ObjectAttributes);

    if( Status != 0 )
        return RtlNtStatusToDosError(Status);

    UNICODE_STRING SymObj = {0,0,NULL};
    ULONG cb = 0;
    Status = NtQuerySymbolicLinkObject(hObject,&SymObj,&cb);

    while( STATUS_BUFFER_TOO_SMALL == Status )
    {
        if( SymObj.Buffer != NULL )
            FreeMemory(SymObj.Buffer);

        SymObj.MaximumLength = (USHORT)cb;
        SymObj.Buffer = (PWSTR)AllocMemory(cb);
        if( SymObj.Buffer == NULL )
        {
            Status = STATUS_NO_MEMORY;
            break;
        }

        Status = NtQuerySymbolicLinkObject(hObject,&SymObj,&cb);
    }

    if( Status == STATUS_SUCCESS )
    {
        int cbCopy = min(SymObj.Length,cchObjectName*sizeof(WCHAR));

        memcpy(ObjectName,SymObj.Buffer,cbCopy);

        ObjectName[cbCopy/sizeof(WCHAR)] = L'\0';
    }

    if( SymObj.Buffer )
        FreeMemory(SymObj.Buffer);

    NtClose(hObject);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}

//---------------------------------------------------------------------------
//
//  QuerySymbolicLinkObjectName()
//
//  PURPOSE:
//
//---------------------------------------------------------------------------
EXTERN_C
ULONG
NTAPI
QuerySymbolicLinkObjectName(
    PCWSTR SymbolicLinkPath,
    PCWSTR SymbolicLinkName,
    PWSTR ObjectNameBuffer,      // name buffer
    LONG ObjectNameBufferLength, // buffer length in character
    PULONG ReturnedLength        // returns length of character including terminate null 
    )
{
    NTSTATUS Status = 0;
    OBJECT_ATTRIBUTES  ObjectAttributes;
    UNICODE_STRING usSymLinkPath;
    HANDLE hSymLink;

    const USHORT cbBufferBytes = 65535;

    usSymLinkPath.Length        = 0;
    usSymLinkPath.MaximumLength = cbBufferBytes;
    usSymLinkPath.Buffer        = (PWCH)AllocMemory( cbBufferBytes );

    if( usSymLinkPath.Buffer == NULL )
    {
        _SetLastStatusDos( STATUS_NO_MEMORY );
        return 0;
    }

    RtlAppendUnicodeToString(&usSymLinkPath,SymbolicLinkPath);

    RtlAppendUnicodeToString(&usSymLinkPath,SymbolicLinkName);

    InitializeObjectAttributes(
            &ObjectAttributes,
            &usSymLinkPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

    Status = NtOpenSymbolicLinkObject(&hSymLink,SYMBOLIC_LINK_QUERY,&ObjectAttributes);

    if(NT_SUCCESS(Status))
    {
        UNICODE_STRING usObjectName;

        usObjectName.Length        = 0;
        usObjectName.MaximumLength = (USHORT)(ObjectNameBufferLength * sizeof(WCHAR));
        usObjectName.Buffer        = ObjectNameBuffer;

        Status = NtQuerySymbolicLinkObject(hSymLink,&usObjectName,ReturnedLength);

        if(NT_SUCCESS(Status))
        {
            ObjectNameBuffer[ (usObjectName.Length/sizeof(WCHAR)) ] = UNICODE_NULL;
            if( ReturnedLength )
                *ReturnedLength = (*ReturnedLength/sizeof(WCHAR));
        }

        NtClose(hSymLink);
    }

    FreeMemory(usSymLinkPath.Buffer);

    _SetLastNtStatus(Status);

    return RtlNtStatusToDosError(Status);
}
