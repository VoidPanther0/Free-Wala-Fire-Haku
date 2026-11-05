#pragma once
// In-process emulator connection (port of Grej Labs' MainThread).
// Must run inside the BlueStacks process: hooks BstkVMM!PGMPhysRead to capture the VM pointer.

#include <Windows.h>
#include <string>
#include <atomic>
#include <MinHook.h>

#include <Cheat/Options.hpp>
#include <Cheat/Offsets/Offsets.hpp>
#include <FrameWork/Utilities/Memory/Memory.hpp>
#include <FrameWork/Utilities/Adb/adb_utils.hpp>

namespace emu_connect {

    inline std::atomic<const char*> status{ "Idle" };

    inline DWORD WINAPI thread_main( LPVOID )
    {
        using FrameWork::MemoryUtils;

        status = "Initializing...";
        Sleep( 1000 );

        HMODULE vmm = GetModuleHandleA( "BstkVMM.dll" );
        if ( !vmm ) { status = "BstkVMM.dll not found (inject into HD-Player)"; return 0; }

        auto read_fn = ( MemoryUtils::PGMPhysReadFunc )GetProcAddress( vmm, "PGMPhysRead" );
        if ( !read_fn ) { status = "PGMPhysRead not found"; return 0; }

        const MH_STATUS init = MH_Initialize( );
        if ( init != MH_OK && init != MH_ERROR_ALREADY_INITIALIZED ) { status = "MinHook init failed"; return 0; }

        if ( MH_CreateHook( ( LPVOID )read_fn, ( LPVOID )MemoryUtils::HookedPGMPhysRead, ( LPVOID* )&MemoryUtils::ogPhysRead ) != MH_OK )
        { status = "Hook creation failed"; return 0; }
        if ( MH_EnableHook( ( LPVOID )read_fn ) != MH_OK ) { status = "Hook enable failed"; return 0; }

        status = "Waiting for VM pointer...";
        for ( int waited = 0; MemoryUtils::vmPtr == nullptr; ++waited )
        {
            Sleep( 10 );
            if ( waited > 1500 ) { status = "VM pointer timeout"; return 0; }
        }

        MemoryUtils::ogCPU   = ( MemoryUtils::VMMGetCpuByIdFunc )GetProcAddress( vmm, "VMMGetCpuById" );
        MemoryUtils::ogCast  = ( MemoryUtils::PGMPhysGCPtr2GCPhysFunc )GetProcAddress( vmm, "PGMPhysGCPtr2GCPhys" );
        MemoryUtils::ogWrite = ( MemoryUtils::PGMPhysSimpleWriteGCPhysFunc )GetProcAddress( vmm, "PGMPhysSimpleWriteGCPhys" );
        if ( !MemoryUtils::ogCPU || !MemoryUtils::ogCast || !MemoryUtils::ogWrite ) { status = "VMM functions missing"; return 0; }

        MemoryUtils::Initialize( MemoryUtils::vmPtr );

        if ( !CambiarDirectoryZ( GetExeDirectoryZ( ) ) ) { status = "Emulator directory error"; return 0; }

        KillProcessByName( "HD-Adb.exe" );
        KillProcessByName( "HD-Adb.exe" );
        KillProcessByName( "HD-Adb.exe" );
        ComdADBZ( "kill-server" );
        ComdADBZ( "devices" );

        status = "Scanning game libraries...";
        Sleep( 400 );

        std::string il2cpp;
        for ( int tries = 0; ; ++tries )
        {
            il2cpp = ShellGetAddressNoSuZ( "cat /proc/*/maps | grep libil2cpp.so" );
            if ( !il2cpp.empty( ) ) break;
            if ( tries > 10 ) { status = "libil2cpp.so not found - start the game, then re-inject"; return 0; }
            status = "Waiting for game (libil2cpp.so)...";
            Sleep( 1500 );
        }

        Offsets::Il2Cpp = uIntExtrZ( il2cpp );
        if ( Offsets::Il2Cpp == 0 ) { status = "Fetching libil2cpp failed"; return 0; }

        g_Options.Hook.IsInjected = true;
        Context::IsReady = true;   // ← ESP draw gate open ho jata hai
        status = "Connection established";
        return 0;
    }

    inline void start( )
    {
        CreateThread( nullptr, 0, thread_main, nullptr, 0, nullptr );
    }

} // namespace emu_connect
