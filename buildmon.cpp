// Include this #define to use SystemTraceControlGuid in Evntrace.h.
#define INITGUID

#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <tdh.h>

static void help() {
  printf("Usage: buildmon [options]\n"
         "\n"
         "-h  Show help\n"
         "-v  Show version\n");
  exit(0);
}

struct Process_TypeGroup1 {
  int UniqueProcessKey;
  int ProcessId;
  int ParentId;
  int SessionId;
  int ExitStatus;
  int DirectoryTableBase;
  int UserSID[13];
  char ImageFileName[1];
  // string CommandLine;
};

static FILE *file;
static PEVENT_TRACE_PROPERTIES properties;
static TRACEHANDLE trace;

static void end(int status) {
  if (trace)
    ControlTrace(0, KERNEL_LOGGER_NAME, properties, EVENT_TRACE_CONTROL_STOP);
  if (file)
    fclose(file);
  ExitProcess(status);
}

static void err(char *s) {
  // Retrieve the system error message for the last-error code
  auto error = GetLastError();
  char *msg;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                0, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (char *)&msg, 0, 0);

  // Display the error message and exit the process
  printf("%s failed with error %d: %s\n", s, error, msg);
  end(error);
}

template <class C> static void output(C *s) {
  fputc('"', file);
  for (; *s; s++) {
    if (*s == '"')
      fputc('"', file);
    fputc(*s, file);
  }
  fputc('"', file);
}

static void WINAPI EventRecordCallback(PEVENT_RECORD event) {
  if (event->EventHeader.EventDescriptor.Opcode != 1)
    return;
  auto p = (Process_TypeGroup1 *)event->UserData;
  auto n = strlen(p->ImageFileName) + 1;
  auto CommandLine = (wchar_t *)(p->ImageFileName + n);
  output(p->ImageFileName);
  fputc(',', file);
  output(CommandLine);
  fputc('\n', file);
}

static DWORD WINAPI processThread(void *) {
  ProcessTrace(&trace, 1, 0, 0);
  return 0;
}

int main(int argc, char **argv) {
  for (auto i = argv + 1; i != argv + argc; ++i) {
    auto s = *i;
    if (*s != '-')
      help();
    while (*s == '-')
      ++s;
    switch (*s) {
    case '?':
    case 'h':
      help();
    case 'V':
    case 'v':
      puts("buildmon version 1");
      return 0;
    default:
      printf("%s: unknown option\n", *i);
      return 1;
    }
  }

  auto BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(KERNEL_LOGGER_NAME);
  properties = (PEVENT_TRACE_PROPERTIES)malloc(BufferSize);

  ZeroMemory(properties, BufferSize);
  properties->EnableFlags = EVENT_TRACE_FLAG_PROCESS;
  properties->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
  properties->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
  properties->Wnode.BufferSize = BufferSize;
  properties->Wnode.ClientContext = 3;
  properties->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
  properties->Wnode.Guid = SystemTraceControlGuid;
  strcpy((char *)properties + properties->LoggerNameOffset, KERNEL_LOGGER_NAME);

  ControlTrace(0, KERNEL_LOGGER_NAME, properties, EVENT_TRACE_CONTROL_STOP);

  auto status = StartTrace(&trace, KERNEL_LOGGER_NAME, properties);
  if (status != ERROR_SUCCESS)
    err("StartTrace");

  EVENT_TRACE_LOGFILE log = {0};
  log.EventRecordCallback = EventRecordCallback;
  log.LoggerName = KERNEL_LOGGER_NAME;
  log.ProcessTraceMode =
      (PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD |
       PROCESS_TRACE_MODE_RAW_TIMESTAMP);
  trace = OpenTrace(&log);
  file = fopen("log.csv", "wb");
  CreateThread(0, 0, processThread, 0, 0, 0);

  puts("Press any key to exit");
  getch();

  end(0);
}
