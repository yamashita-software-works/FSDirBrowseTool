#pragma once

#ifdef _WINDOWS

typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

#define NTAPI __stdcall
#define NTSTATUS LONG

EXTERN_C
ULONG
NTAPI
RtlNtStatusToDosError(
    IN NTSTATUS  Status
	); 

#endif
