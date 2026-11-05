#pragma once
#include <Windows.h>
#include <TlHelp32.h>

// tlhelp32.h has no *A names; the unsuffixed ones are ANSI and UNICODE remaps them to W.
#ifdef UNICODE
#undef PROCESSENTRY32
#undef Process32First
#undef Process32Next
#endif

#ifndef PROCESSENTRY32A
#define PROCESSENTRY32A PROCESSENTRY32
#define Process32FirstA Process32First
#define Process32NextA  Process32Next
#endif
