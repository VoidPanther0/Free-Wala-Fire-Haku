#pragma warning(disable: 4996)  // freopen / _wfopen safe variants
#include <FrameWork/Utilities/Misc/Misc.hpp>
#include <FrameWork/FrameWork.hpp>
#include <algorithm>

namespace FrameWork {
    namespace Misc {
        int RandomInt(int Lower, int Max) {
            return (rand() % (Max - Lower + 1)) + Lower;
        }

        std::string RandomString(size_t Length) {
            auto Randchar = []() -> char {
                const char* Charset = ("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
                const size_t MaxIndex = (sizeof(Charset) - 1);
                return Charset[rand() % MaxIndex];
                };

            std::string Str(Length, 0);
            std::generate_n(Str.begin(), Length, Randchar);
            return Str;
        }

        std::string Wstring2String(std::wstring Input) {
            if (Input.empty()) return {};

            int size_needed = WideCharToMultiByte(CP_ACP, 0, Input.c_str(), (int)Input.size(), NULL, 0, NULL, NULL);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_ACP, 0, Input.c_str(), (int)Input.size(), &strTo[0], size_needed, NULL, NULL);
            return strTo;
        }

        std::string convert_u8string(std::string u8str) {
            return std::string(u8str.begin(), u8str.end());
        }

        std::wstring String2WString(std::string Input) {
            return std::wstring(Input.begin(), Input.end());
        }

        void InitializeConsole() {
            (AllocConsole)();
            SetConsoleOutputCP(CP_UTF8);
            SetConsoleCP(CP_UTF8);
            freopen(("CONIN$"), "r", stdin);
            freopen(("CONOUT$"), "w", stdout);
            freopen(("CONOUT$"), "w", stderr);
            (SetConsoleTitleA)(("Debug Console"));
        }

        void ShutDownConsole() {
            (FreeConsole)();
            (fclose)(stdout);
        }
    }
}