#define INITGUID  // Include this #define to use SystemTraceControlGuid in Evntrace.h.

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <evntcons.h>
#include <evntrace.h>

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

static void WINAPI
EventRecordCallback(EVENT_RECORD *EventRecord)
{
	EVENT_HEADER &Header = EventRecord->EventHeader;

	UCHAR ProcessorNumber = EventRecord->BufferContext.ProcessorNumber;
	ULONG ThreadID = Header.ThreadId;

	// Process event here.
}

TRACEHANDLE ConsumerHandle;

static DWORD WINAPI
Win32TracingThread(LPVOID Parameter)
{
	ProcessTrace(&ConsumerHandle, 1, 0, 0);
	return(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ULONG status = ERROR_SUCCESS;
	TRACEHANDLE SessionHandle = 0;
	EVENT_TRACE_PROPERTIES* pSessionProperties = NULL;
	ULONG BufferSize = 0;

	BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
	pSessionProperties = (EVENT_TRACE_PROPERTIES*)malloc(BufferSize);

	ZeroMemory(pSessionProperties, BufferSize);
	pSessionProperties->Wnode.BufferSize = BufferSize;
	pSessionProperties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	pSessionProperties->Wnode.ClientContext = 3;
	pSessionProperties->Wnode.Guid = SystemTraceControlGuid;
	pSessionProperties->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;
	pSessionProperties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
	pSessionProperties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
	strcpy((LPSTR)((char*)pSessionProperties + pSessionProperties->LoggerNameOffset),
    KERNEL_LOGGER_NAME);

ControlTrace(0, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_STOP);

	status = StartTrace((PTRACEHANDLE)&SessionHandle, KERNEL_LOGGER_NAME, pSessionProperties);
	if (ERROR_SUCCESS != status)
	{
			ErrorExit("EnableTrace");
	}

	EVENT_TRACE_LOGFILE LogFile = { 0 };
	LogFile.LoggerName = KERNEL_LOGGER_NAME;
	LogFile.ProcessTraceMode = (PROCESS_TRACE_MODE_REAL_TIME |
		PROCESS_TRACE_MODE_EVENT_RECORD |
		PROCESS_TRACE_MODE_RAW_TIMESTAMP);
	LogFile.EventRecordCallback = EventRecordCallback;
	ConsumerHandle = OpenTrace(&LogFile);
	DWORD ThreadID;
	HANDLE ThreadHandle = CreateThread(0, 0, Win32TracingThread, 0, 0, &ThreadID);
	CloseHandle(ThreadHandle);

	wprintf(L"Press any key to end trace session ");
	getchar();

ControlTrace(0, KERNEL_LOGGER_NAME, pSessionProperties, EVENT_TRACE_CONTROL_STOP);
	return 0;
}

