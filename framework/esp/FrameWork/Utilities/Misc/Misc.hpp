#pragma once
#include <string>
#include <Windows.h>
#include <iostream>

namespace FrameWork {
	namespace Misc {
		int RandomInt(int Lower = -100, int Max = 100);
		std::string RandomString(size_t Length = 12);



		
		std::string Wstring2String(std::wstring Input);
		std::string convert_u8string(std::string u8str);
		std::wstring String2WString(std::string Input);
		void InitializeConsole();

		template<typename T>
		void Log(const T& message) {
			std::cout << message << std::endl;
		}

		template<typename T, typename... Args>
		void Log(const T& first, const Args&... args) {
			std::cout << first;
			Log(args...);
		}
		void ShutDownConsole();
	}
}
