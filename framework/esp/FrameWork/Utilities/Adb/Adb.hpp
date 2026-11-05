#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <cstdint>
#include <cctype>

static bool juegoAbierto = false;

typedef unsigned char bimno1;

struct PatternData {
    std::vector<bimno1> pattern;
    std::vector<bimno1> mask;
};

std::vector<bimno1> StringToBytes1(const std::string& pattern) {
    std::vector<bimno1> bytes;
    std::istringstream stream(pattern);
    std::string byteStr;

    while (stream >> byteStr) {
        bytes.push_back(static_cast<bimno1>(std::stoul(byteStr, nullptr, 16)));
    }
    return bytes;
}

class Adb {
private:
    std::wstring path;
    PROCESS_INFORMATION processInfo;

public:
    Adb(const std::wstring& adbPath) : path(adbPath) {
        ZeroMemory(&processInfo, sizeof(processInfo));
    }

    void Kill() {
        KillHDAdbProcesses();
    }

    bool Start() {
        ExecuteAdbCommand(L"kill-server");
        ExecuteAdbCommand(L"devices");

        STARTUPINFOW startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);

        std::wstring command = L"shell \"getprop ro.secure ; /boot/android/android/system/xbin/bstk/su\"";
        if (!CreateProcessW(path.c_str(), const_cast<wchar_t*>(command.c_str()), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo)) {
            std::wcerr << L"Error al iniciar ADB: " << GetLastError() << std::endl;
            return false;
        }
        WaitForInputIdle(processInfo.hProcess, INFINITE);
        return true;
    }

    void EndGame() {
        ExecuteAdbCommand(L"shell am force-stop com.dts.freefireth");
    }

    void OpenGame() {
        ExecuteAdbCommand(L"shell am start -n com.dts.freefireth/.FFMainActivity");
        juegoAbierto = true;
    }

    uint32_t FindModule(const std::wstring& processName, const std::wstring& moduleName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error al crear el snapshot del proceso." << std::endl;
            return 0;
        }

        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32W);
        DWORD pid = 0;

        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                if (std::wstring(processEntry.szExeFile) == processName) {
                    pid = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);

        if (pid == 0) {
            std::wcerr << L"No se encontr� el proceso: " << processName << std::endl;
            return 0;
        }

        snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (snapshot == INVALID_HANDLE_VALUE) {
            std::wcerr << L"Error al crear el snapshot del m�dulo." << std::endl;
            return 0;
        }

        MODULEENTRY32W moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32W);
        uint32_t baseAddress = 0;

        if (Module32FirstW(snapshot, &moduleEntry)) {
            do {
                if (std::wstring(moduleEntry.szModule) == moduleName) {
                    baseAddress = reinterpret_cast<uint32_t>(moduleEntry.modBaseAddr);
                    break;
                }
            } while (Module32NextW(snapshot, &moduleEntry));
        }
        CloseHandle(snapshot);

        if (baseAddress == 0) {
            std::wcerr << L"No se encontr� el m�dulo: " << moduleName << std::endl;
        }
        return baseAddress;
    }

private:
    void KillHDAdbProcesses() {
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

    void ExecuteAdbCommand(const std::wstring& command) {
        STARTUPINFOW startupInfo;
        PROCESS_INFORMATION processInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        ZeroMemory(&processInfo, sizeof(processInfo));
        startupInfo.cb = sizeof(startupInfo);

        std::wstring fullCommand = path + L" " + command;

        if (!CreateProcessW(nullptr, const_cast<wchar_t*>(fullCommand.c_str()), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo)) {
            std::wcerr << L"Error al ejecutar el comando ADB: " << GetLastError() << std::endl;
        }

        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }
};



std::string GetProcessPath(const std::string& processName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot == INVALID_HANDLE_VALUE) return "";

    std::string processPath = "";
    if (Process32First(snapshot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
                if (hProcess) {
                    char buffer[MAX_PATH];
                    if (GetModuleFileNameExA(hProcess, NULL, buffer, MAX_PATH)) {
                        processPath = buffer;
                    }
                    CloseHandle(hProcess);
                }
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return processPath;
}

std::vector<std::string> GetAvailableDrives() {
    std::vector<std::string> drives;
    DWORD driveMask = GetLogicalDrives();

    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (driveMask & (1 << (letter - 'A'))) {
            std::string drive = { letter, ':', '\\' };
            drives.push_back(drive);
        }
    }
    return drives;
}

std::string FindAdbPath(const std::string& emulatorFolder) {
    std::vector<std::string> drives = GetAvailableDrives();

    for (const auto& drive : drives) {
        std::string adbPath = drive + ("Program Files\\") + emulatorFolder + ("\\HD-Adb.exe");
        if (std::filesystem::exists(adbPath)) {
            return adbPath;
        }
    }
    return "";
}

std::string GetAdbPath() {
    std::string hdPlayerPath = GetProcessPath(("HD-Player.exe"));
    std::string blueStacksPath = GetProcessPath(("Bluestacks.exe"));

    if (!hdPlayerPath.empty()) {
        if (hdPlayerPath.find(("BlueStacks_msi5")) != std::string::npos) {
            return FindAdbPath(("BlueStacks_msi5"));
        }
        else if (hdPlayerPath.find(("BlueStacks_msi2")) != std::string::npos) {
            return FindAdbPath(("BlueStacks_msi2"));
        }
        else if (hdPlayerPath.find(("BlueStacks_nxt")) != std::string::npos) {
            return FindAdbPath(("BlueStacks_nxt"));
        }
        else if (hdPlayerPath.find(("BlueStacks")) != std::string::npos) {
            return FindAdbPath(("BlueStacks"));
        }
    }

    if (!blueStacksPath.empty() && blueStacksPath.find(("BlueStacks")) != std::string::npos) {
        return FindAdbPath(("BlueStacks"));
    }
    std::string adbPathMSI5 = FindAdbPath(("BlueStacks_msi5"));
    std::string adbPathMSI2 = FindAdbPath(("BlueStacks_msi2"));
    std::string adbPathNXT = FindAdbPath(("BlueStacks_nxt"));
    std::string adbPathBlueStacks = FindAdbPath(("BlueStacks"));

    if (!adbPathMSI5.empty()) {
        return adbPathMSI5;
    }
    if (!adbPathMSI2.empty()) {
        return adbPathMSI2;
    }
    if (!adbPathNXT.empty()) {
        return adbPathNXT;
    }
    return adbPathBlueStacks;
}


std::string adbPath = GetAdbPath();
std::string adbPath1 = adbPath;
