// Include this #define to use SystemTraceControlGuid in Evntrace.h.
#define INITGUID

#include <windows.h>

#include <evntcons.h>
#include <evntrace.h>
#include <stdio.h>

static void err(char *s) {
  // Retrieve the system error message for the last-error code
  auto e = GetLastError();
  char *msg;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                0, e, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *)&msg,
                0, 0);

  // Display the error message and exit the process
  printf("%s failed with error %d: %s\n", s, e, msg);
  ExitProcess(e);
}

static void WINAPI EventRecordCallback(EVENT_RECORD *EventRecord) {
  EVENT_HEADER &Header = EventRecord->EventHeader;

  UCHAR ProcessorNumber = EventRecord->BufferContext.ProcessorNumber;
  ULONG ThreadID = Header.ThreadId;

  // Process event here.
}

static TRACEHANDLE trace;

static DWORD WINAPI processThread(void *) {
  ProcessTrace(&trace, 1, 0, 0);
  return 0;
}

int main(int argc, char **argv) {
  auto BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
  auto properties = (EVENT_TRACE_PROPERTIES *)malloc(BufferSize);

  ZeroMemory(properties, BufferSize);
  properties->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;
  properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
  properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
  properties->Wnode.BufferSize = BufferSize;
  properties->Wnode.ClientContext = 3;
  properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
  properties->Wnode.Guid = SystemTraceControlGuid;
  strcpy((char *)properties + properties->LoggerNameOffset, KERNEL_LOGGER_NAME);

  ControlTrace(0, KERNEL_LOGGER_NAME, properties, EVENT_TRACE_CONTROL_STOP);

  auto status =
      StartTrace((PTRACEHANDLE)&trace, KERNEL_LOGGER_NAME, properties);
  if (ERROR_SUCCESS != status)
    err("StartTrace");

  EVENT_TRACE_LOGFILE log = {0};
  log.EventRecordCallback = EventRecordCallback;
  log.LoggerName = KERNEL_LOGGER_NAME;
  log.ProcessTraceMode =
      (PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD |
       PROCESS_TRACE_MODE_RAW_TIMESTAMP);
  trace = OpenTrace(&log);
  CreateThread(0, 0, processThread, 0, 0, 0);

  wprintf(L"Press any key to end trace session ");
  getchar();

  ControlTrace(0, KERNEL_LOGGER_NAME, properties, EVENT_TRACE_CONTROL_STOP);
  return 0;
}
