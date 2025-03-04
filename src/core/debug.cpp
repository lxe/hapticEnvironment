#include "debug.h"
#include <windows.h>
#include <dbghelp.h>
#include <sstream>
#include <iomanip>
#include <iostream>

// Debug logging function implementation
void debug_log(const char* file, int line, const char* func, const char* msg) {
    std::cout << "[DEBUG] " << file << ":" << line << " in " << func << ": " << msg << std::endl;
    std::cout.flush();
}

// Stack trace handler for Windows
void print_stack_trace() {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT context;
    context.ContextFlags = CONTEXT_FULL;
    GetThreadContext(thread, &context);

    STACKFRAME64 frame;
    frame.AddrPC.Offset = context.Rip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Rbp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Rsp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrBStore.Offset = context.Rbx;
    frame.AddrBStore.Mode = AddrModeFlat;

    std::cout << "\nStack trace:" << std::endl;
    for (int i = 0; i < 25; i++) {
        if (!StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &frame,
                        &context, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) {
            break;
        }

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;

        if (SymFromAddr(process, frame.AddrPC.Offset, &displacement, symbol)) {
            std::cout << std::hex << frame.AddrPC.Offset << ": " << symbol->Name << std::endl;
        }
    }
    std::cout.flush();
} 