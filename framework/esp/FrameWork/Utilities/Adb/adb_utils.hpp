#pragma once
#include <Windows.h>
#include <FrameWork/Utilities/Misc/ToolhelpAnsi.hpp>
#include <psapi.h>
#include <string>
#include <iostream>
#pragma comment(lib, "Psapi.lib")

// Obtiene el ID de un proceso por su nombre.
inline DWORD GetProcZ(const std::string& processName) {
    DWORD processID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32A pe32 = { sizeof(PROCESSENTRY32A) };
        if (Process32FirstA(hSnapshot, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    processID = pe32.th32ProcessID;
                    break;
                }
            } while (Process32NextA(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return processID;
}

// Verifica si un proceso est� ejecut�ndose y cuenta las instancias.
inline bool CheckProcessInstancesZ(const std::string& processName, int& count) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    count = 0;
    PROCESSENTRY32A pe32 = { sizeof(PROCESSENTRY32A) };
    if (Process32FirstA(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                count++;
            }
        } while (Process32NextA(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return true;
}

// Mata un proceso dado su ID.
inline bool KillProcZ(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);
    if (!hProcess) {
        return false;
    }
    bool result = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    return result;
}

// Verifica si un proceso est� corriendo y devuelve su ID.
inline bool IsProcRunZ(const std::string& processName, DWORD& processID) {
    processID = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool isRunning = false;
    PROCESSENTRY32A pe32 = { sizeof(PROCESSENTRY32A) };
    if (Process32FirstA(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                processID = pe32.th32ProcessID;
                isRunning = true;
                break;
            }
        } while (Process32NextA(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return isRunning;
}

// Env�a un mensaje para cerrar la ventana asociada a un proceso.
inline bool ForgeKillProcZ(DWORD processID) {
    HWND hwnd = NULL;
    DWORD dwPID = 0;
    while ((hwnd = FindWindowEx(NULL, hwnd, NULL, NULL)) != NULL) {
        GetWindowThreadProcessId(hwnd, &dwPID);
        if (dwPID == processID) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return true;
        }
    }
    return false;
}
inline void ForceKillByName(const char* processName) {
    DWORD pid = GetProcZ(processName);
    if (pid != 0) KillProcZ(pid);

    DWORD pid2 = 0;
    if (IsProcRunZ(processName, pid2)) ForgeKillProcZ(pid2);
}

inline void KillAdbZ() {
    ForceKillByName("adb.exe");
    ForceKillByName("HD-Adb.exe");
}

inline void KillBst() {
    ForceKillByName("BstkSVC.exe");
    ForceKillByName("BstkSVC.exe");
}

inline void KillBstEmu() {
    ForceKillByName("HD-Player.exe");
    ForceKillByName("HD-Player.exe");
}



inline std::string GetExeDirectoryZ() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);

    std::string fullPath(path);
    size_t lastSlashIndex = fullPath.find_last_of("\\/");
    if (lastSlashIndex != std::string::npos) {
        return fullPath.substr(0, lastSlashIndex);
    }
    return "";
}

inline std::string GetExeDirectoryPIDZ(DWORD processID) {
    char path[MAX_PATH];
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (hProcess) {
        if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
            std::string fullPath(path);
            size_t lastSlashIndex = fullPath.find_last_of("\\/");
            CloseHandle(hProcess);
            if (lastSlashIndex != std::string::npos) {
                return fullPath.substr(0, lastSlashIndex);
            }
        }
        CloseHandle(hProcess);
    }

    return "";
}

inline bool CambiarDirectoryZ(const std::string& path) {
    return SetCurrentDirectoryA(path.c_str());
}

inline std::string ExtraerLibAddressZ(const std::string& input) {
    std::size_t pos = input.find('-');
    if (pos != std::string::npos && pos >= 8) {
        return input.substr(pos - 8, 8);
    }
    return "";
}

inline uintptr_t uIntExtrZ(std::string c, int base = 16) {
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long)
        || sizeof(uintptr_t) == sizeof(unsigned long long),
        "Please add string to handle conversion for this architecture.");
    if (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return strtoul(c.c_str(), nullptr, base);
    }
    return strtoull(c.c_str(), nullptr, base);
}

inline std::string ShellGetAddressZ(const std::string& firstCommand, const std::string& secondCommand) {
    HANDLE hStdOutRead, hStdOutWrite;
    HANDLE hStdInRead, hStdInWrite;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
        return 0;
    }
    if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
        return 0;
    }

    if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
        std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
        return 0;
    }
    if (!SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
        return 0;
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hStdOutWrite;
    si.hStdOutput = hStdOutWrite;
    si.hStdInput = hStdInRead;
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(NULL, (LPSTR)".\\HD-Adb shell", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
        return 0;
    }

    CloseHandle(hStdOutWrite);
    CloseHandle(hStdInRead);

    DWORD written;
    std::string commands = firstCommand + "\n" + secondCommand + "\n";
    if (!WriteFile(hStdInWrite, commands.c_str(), commands.length(), &written, NULL)) {
        std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
        return 0;
    }
    CloseHandle(hStdInWrite);

    CHAR buffer[128];
    DWORD read;
    std::string output;
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output += buffer;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdOutRead);

    return ExtraerLibAddressZ(output);
}

inline std::string ShellGetAddressNoSuZ(const std::string& comand) {
    HANDLE hStdOutRead, hStdOutWrite;
    HANDLE hStdInRead, hStdInWrite;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
        return 0;
    }
    if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
        return 0;
    }

    if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
        std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
        return 0;
    }
    if (!SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
        return 0;
    }

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hStdOutWrite;
    si.hStdOutput = hStdOutWrite;
    si.hStdInput = hStdInRead;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::string fullCmd = ".\\HD-Adb.exe shell \"/system/xbin/bstk/su 0 cat /proc/*/maps | grep libil2cpp.so\"";
    if (!CreateProcessA(NULL, const_cast<LPSTR>(fullCmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
        return 0;
    }

    CloseHandle(hStdOutWrite);
    CloseHandle(hStdInRead);

    DWORD written;
    std::string commands = comand + "\n";
    if (!WriteFile(hStdInWrite, commands.c_str(), commands.length(), &written, NULL)) {
        std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
        return 0;
    }
    CloseHandle(hStdInWrite);

    CHAR buffer[128];
    DWORD read;
    std::string output;
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
        buffer[read] = '\0';
        output += buffer;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdOutRead);

    return ExtraerLibAddressZ(output);
}

inline void ComdADBZ(const std::string& command) {
    std::string fullCommand = ".\\HD-Adb shell " + command;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_HIDE;
    ZeroMemory(&pi, sizeof(pi));
    if (!CreateProcessA(NULL, const_cast<LPSTR>(fullCommand.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start process. Error: " << GetLastError() << std::endl;
        return;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

inline void KillProcessByName(const std::string& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32A pe;
    pe.dwSize = sizeof(PROCESSENTRY32A);

    if (Process32FirstA(hSnap, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName.c_str()) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess) {
                    TerminateProcess(hProcess, 0);
                    CloseHandle(hProcess);
                }
                break;
            }
        } while (Process32NextA(hSnap, &pe));
    }

    CloseHandle(hSnap);
}
inline void KillHDAdbProcesses() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (std::wstring(processEntry.szExeFile) == L"HD-Adb.exe") {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }
}


