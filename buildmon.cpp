#include "stdafx.h"

#define LOGFILE_PATH "log.etl"

void ErrorExit(char* lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	printf(
		"%s failed with error %d: %s\n",
		lpszFunction, dw, lpMsgBuf);
	ExitProcess(dw);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ULONG status = ERROR_SUCCESS;
	TRACEHANDLE SessionHandle = 0;
	EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
	ULONG BufferSize = 0;

	// Allocate memory for the session properties. The memory must
	// be large enough to include the log file name and session name,
	// which get appended to the end of the session properties structure.

	BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(LOGFILE_PATH) + sizeof(KERNEL_LOGGER_NAME);
	pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);
	if (NULL == pSessionProperties)
	{
		wprintf(L"Unable to allocate %d bytes for properties structure.\n", BufferSize);
		goto cleanup;
	}

	// Set the session properties. You only append the log file name
	// to the properties structure; the StartTrace function appends
	// the session name for you.

	ZeroMemory(pSessionProperties, BufferSize);
	pSessionProperties->Wnode.BufferSize = BufferSize;
	pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	pSessionProperties->Wnode.ClientContext = 1; //QPC clock resolution
	pSessionProperties->Wnode.Guid = SystemTraceControlGuid;
	pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;
	pSessionProperties->LogFileMode = EVENT_TRACE_FILE_MODE_CIRCULAR;
	pSessionProperties->MaximumFileSize = 5;  // 5 MB
	pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	pSessionProperties->LogFileNameOffset = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
	strcpy(((char*)pSessionProperties + pSessionProperties->LogFileNameOffset), LOGFILE_PATH);

	// Create the trace session.

	status = StartTrace((PTRACEHANDLE)&SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties);

	if (ERROR_SUCCESS != status)
	{
		if (ERROR_ALREADY_EXISTS == status)
		{
			wprintf(L"The NT Kernel Logger session is already in use.\n");
		}
		else
		{
			ErrorExit("EnableTrace");
		}

		goto cleanup;
	}

	wprintf(L"Press any key to end trace session ");
	getchar();

cleanup:

	if (SessionHandle)
	{
		status = ControlTrace(SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_STOP);

		if (ERROR_SUCCESS != status)
		{
			ErrorExit("ControlTrace");
		}
	}

	if (pSessionProperties)
		free(pSessionProperties);
	return 0;
}

