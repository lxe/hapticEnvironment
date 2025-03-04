#pragma once

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <string>

// Debug logging function declaration
void debug_log(const char* file, int line, const char* func, const char* msg);

// Stack trace function declaration
void print_stack_trace();

#endif 