#include "GameLauncher.h"

#include <windows.h>
#include <tlhelp32.h>

static bool GetProcessExecutablePath_P(QString & path, HANDLE process)
{
    path.clear();

    DWORD length = MAX_PATH;
    QByteArray temp_buffer;

    bool success = false;
    do
    {
        temp_buffer.resize(length);
        if (temp_buffer.size() != length)
            return false;

        success = QueryFullProcessImageName(process, 0, temp_buffer.data(), &length) != 0;
        if (!success)
        {
            if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                return false;

            length *= 2;
        }
    } while (!success);

    path = QString(temp_buffer);
    return true;
}

static bool getCurrentDirectory_P(QString & path)
{
    path.clear();

    int length = GetCurrentDirectory(0, NULL);

    QByteArray temp_buffer;
    temp_buffer.resize(length);
    if (temp_buffer.size() != length)
        return false;

    if (!GetCurrentDirectory(length, temp_buffer.data()))
        return false;

    path = QString(temp_buffer);
    return true;
}

GameLauncher::GameLauncher(QObject *parent) : QThread(parent)
{
}

void GameLauncher::setGameFolder(const QString & game_folder)
{
    this->game_folder = game_folder;
}

void GameLauncher::setCommandLine(const QString & command_line)
{
    this->command_line = command_line;
}

void GameLauncher::setProfileName(const QString & profile_name)
{
    this->profile_name = profile_name;
}

void GameLauncher::enableStandaloneMode(bool enable)
{
    this->standalone_mode = enable;
}

const QString & GameLauncher::getErrorMessages()
{
    return error_messages;
}

void GameLauncher::run()
{
    memset(&process_information, 0, sizeof (PROCESS_INFORMATION));
    executable_path = game_folder + "\\Gw2.exe";
    error_messages.clear();
    mutex_count = 0;

    bool running = false;
    if (!isInstanceRunning(running))
    {
        error_messages = tr("Failed to determine if the selected profile is already running");
        return;
    }

    if (running && standalone_mode)
    {
        error_messages = tr("The selected profile is already running");
        return;
    }

    if (!getCurrentDirectory_P(working_directory))
    {
        error_messages = tr("Could not determine the working directory");
        return;
    }
    profile_base_path = working_directory + "\\profile_data\\" + profile_name;

    STARTUPINFO startup_info;
    startup_info.cb = sizeof (STARTUPINFO);
    GetStartupInfo(&startup_info);

    if (!CreateProcess(executable_path.toLatin1().constData(), command_line.toLatin1().data(), NULL, NULL, FALSE, DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &startup_info, &process_information))
    {
        error_messages = tr("Could not create the process");
        return;
    }

    game_started = processDebugEvents();

    CloseHandle(process_information.hProcess);
    CloseHandle(process_information.hThread);

    // this is a lame check to inform the user that the game might have crashed because
    // more than one instance has been started with the same shared profile.
    //
    // it is required to create a unique profile for each (running) shared client since we're now
    // saving the configuration files inside the profiles folder (and we can't have more than one
    // client using the same local.dat/gw2.tmp/gw2local.tmp)
    if (!game_started && !standalone_mode)
    {
        error_messages += QString("\n\n") + tr("Since this is a shared profile, you're allowed to run this game folder more than once, but you are required to create one (shared) profile for each instance!");
        return;
    }
}

bool GameLauncher::hasSucceeded()
{
    return game_started;
}

bool GameLauncher::setSoftwareBreakpoint(UInt64 address, UInt8 & overwritten_byte)
{
    UInt8 breakpoint_opcode = (UInt8) 0xCC;

    DWORD original_mem_protection;
    if (!VirtualProtectEx(process_information.hProcess, (LPVOID) address, sizeof (UInt8), PAGE_EXECUTE_READWRITE, &original_mem_protection))
        return false;

    SIZE_T processed_bytes;
    if (!ReadProcessMemory(process_information.hProcess, (LPCVOID) address, &overwritten_byte, sizeof (overwritten_byte), &processed_bytes))
        return false;

    if (!WriteProcessMemory(process_information.hProcess, (LPVOID) address, &breakpoint_opcode, sizeof (breakpoint_opcode), &processed_bytes))
        return false;

    if (!VirtualProtectEx(process_information.hProcess, (LPVOID) address, sizeof (UInt8), original_mem_protection, &original_mem_protection))
        return false;

    if (!FlushInstructionCache(process_information.hProcess, (LPCVOID) address, sizeof (UInt8)))
        return false;

    return true;
}

bool GameLauncher::removeSoftwareBreakpoint(UInt64 address, UInt8 original_byte)
{
    DWORD original_mem_protection;
    if (!VirtualProtectEx(process_information.hProcess, (LPVOID) address, sizeof (UInt8), PAGE_EXECUTE_READWRITE, &original_mem_protection))
        return false;

    SIZE_T written_bytes;
    if (!WriteProcessMemory(process_information.hProcess, (LPVOID) address, &original_byte, sizeof (original_byte), &written_bytes))
        return false;

    if (!VirtualProtectEx(process_information.hProcess, (LPVOID) address, sizeof (UInt8), original_mem_protection, &original_mem_protection))
        return false;

    if (!FlushInstructionCache(process_information.hProcess, (LPCVOID) address, sizeof (UInt8)))
        return false;

    return true;
}

void GameLauncher::setSingleStepFlag(CONTEXT & context)
{
    context.EFlags |= 0x100;
}

bool GameLauncher::queryAllocationLength(UInt64 address, UInt64 & length)
{
    MEMORY_BASIC_INFORMATION mem_info;
    if (!VirtualQueryEx(process_information.hProcess, (LPCVOID) address, &mem_info, sizeof (mem_info)))
        return false;

    UInt64 highest_address = ((UInt64) mem_info.BaseAddress) + ((UInt64) mem_info.RegionSize);
    length = highest_address - address;
    return true;
}

bool GameLauncher::isInstanceRunning(bool & running)
{
    running = false;

    HANDLE system_snapshot = NULL;
    HANDLE process = NULL;

    try
    {
        system_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (system_snapshot == INVALID_HANDLE_VALUE)
            throw false;

        PROCESSENTRY32 current_process;
        current_process.dwSize = sizeof (PROCESSENTRY32);

        bool initialize = true;
        while (true)
        {
            bool error;
            if (initialize)
            {
                error = Process32First(system_snapshot, &current_process) == FALSE;
                initialize = false;
            }
            else
                error = Process32Next(system_snapshot, &current_process) == FALSE;

            if (error)
            {
                if (GetLastError() != ERROR_NO_MORE_FILES)
                    throw false;

                break;
            }

            process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, current_process.th32ProcessID);
            if (process == NULL)
                continue;

            QString process_path;
            if (!GetProcessExecutablePath_P(process_path, process))
                throw false;

            if (!CloseHandle(process))
                throw false;
            process = NULL;

            if (running = process_path.contains(executable_path, Qt::CaseInsensitive))
                break;
        }

        throw true;
    }

    catch (bool exit_code)
    {
        if (process != NULL)
        {
            if (!CloseHandle(process))
                exit_code = false;
        }

        if (system_snapshot != INVALID_HANDLE_VALUE)
        {
            if (!CloseHandle(system_snapshot))
                exit_code = false;
        }

        return exit_code;
    }
}

bool GameLauncher::processDebugEvents()
{
    UInt64 createmutexa_address = (UInt64) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateMutexA");
    UInt8 createmutexa_backup;
    bool restore_createmutexa_breakpoint = false;

    UInt64 createfilew_address = (UInt64) GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileW");
    UInt8 createfilew_backup;
    bool restore_createfilew_breakpoint = false;

    bool system_breakpoint = true;
    bool terminate = false;

    while (true)
    {
        DEBUG_EVENT debug_event;
        if (!WaitForDebugEvent(&debug_event, 0))
        {
            if (GetLastError() != ERROR_SEM_TIMEOUT)
            {
                error_messages = tr("Failed to fetch the debug event");
                return false;
            }

            if (terminate)
            {
                if (!DebugSetProcessKillOnExit(FALSE))
                {
                    error_messages = tr("Failed to detach from the process");
                    return false;
                }

                DebugActiveProcessStop(process_information.dwProcessId);
                return true;
            }

            Sleep(1);
            continue;
        }

        switch (debug_event.dwDebugEventCode)
        {
            // exceptions
            case EXCEPTION_DEBUG_EVENT:
            {
                bool exception_handled = false;
                bool enable_single_step = false;
                UInt32 exception_code = debug_event.u.Exception.ExceptionRecord.ExceptionCode;
                UInt64 exception_address = (UInt64) debug_event.u.Exception.ExceptionRecord.ExceptionAddress;

                if (exception_code == EXCEPTION_BREAKPOINT)
                {
                    HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, debug_event.dwThreadId);
                    if (thread == NULL)
                    {
                        error_messages = tr("Failed to open the thread");
                        return false;
                    }

                    CONTEXT context;
                    context.ContextFlags = CONTEXT_ALL;
                    if (!GetThreadContext(thread, &context))
                    {
                        error_messages = tr("Failed to obtain the thread context");
                        return false;
                    }

                    context.Eip = (DWORD) exception_address;

                    if (system_breakpoint)
                    {
                        if (!setSoftwareBreakpoint(createmutexa_address, createmutexa_backup) || !setSoftwareBreakpoint(createfilew_address, createfilew_backup))
                        {
                            error_messages = tr("Failed to set the breakpoints");
                            return false;
                        }

                        exception_handled = true;
                        system_breakpoint = false;
                    }

                    else if (exception_address == createmutexa_address)
                    {
                        if (!onCreateMutexABreakpoint(context, restore_createmutexa_breakpoint))
                        {
                            error_messages = tr("The breakpoint handling procedure has failed");
                            return false;
                        }
                        restore_createmutexa_breakpoint = !restore_createmutexa_breakpoint;

                        if (!removeSoftwareBreakpoint(exception_address, createmutexa_backup))
                        {
                            error_messages = tr("Failed to remove the breakpoint");
                            return false;
                        }

                        enable_single_step = restore_createmutexa_breakpoint;
                        exception_handled = true;
                    }

                    else if (exception_address == createfilew_address)
                    {
                        if (!onCreateFileWBreakpoint(context, restore_createfilew_breakpoint))
                        {
                            error_messages = tr("The breakpoint handling procedure has failed");
                            return false;
                        }
                        restore_createfilew_breakpoint = !restore_createfilew_breakpoint;
                        terminate = !restore_createfilew_breakpoint;

                        if (!removeSoftwareBreakpoint(exception_address, createfilew_backup))
                        {
                            error_messages = tr("Failed to remove the breakpoint");
                            return false;
                        }

                        enable_single_step = restore_createfilew_breakpoint;
                        exception_handled = true;
                    }

                    if (enable_single_step)
                        setSingleStepFlag(context);

                    if (!SetThreadContext(thread, &context))
                    {
                        error_messages = tr("Failed to set the thread context");
                        return false;
                    }

                    CloseHandle(thread);
                }

                else if (exception_code == EXCEPTION_SINGLE_STEP)
                {
                    bool error = false;
                    if (restore_createmutexa_breakpoint)
                    {
                        error = !setSoftwareBreakpoint(createmutexa_address, createmutexa_backup);
                        restore_createmutexa_breakpoint = false;
                        exception_handled = true;
                    }
                    else if (restore_createfilew_breakpoint)
                    {
                        error = !setSoftwareBreakpoint(createfilew_address, createfilew_backup);
                        restore_createfilew_breakpoint = false;
                        exception_handled = true;
                    }

                    if (exception_handled && error)
                    {
                        error_messages = tr("Failed to restore the breakpoint");
                        return false;
                    }
                }

                if (!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, exception_handled ? DBG_CONTINUE : DBG_EXCEPTION_NOT_HANDLED))
                {
                    error_messages = tr("Failed to resume the thread");
                    return false;
                }

                break;
            }

            // process termination
            case EXIT_PROCESS_DEBUG_EVENT:
            case RIP_EVENT:
            {
                error_messages = tr("The game has unexpectedly terminated");
                return false;
            }

            default:
            {
                if (!ContinueDebugEvent(debug_event.dwProcessId, debug_event.dwThreadId, DBG_CONTINUE))
                {
                    error_messages = tr("Failed to resume the thread");
                    return false;
                }

                break;
            }
        }
    }
}

bool GameLauncher::onCreateMutexABreakpoint(CONTEXT & context, bool & remove_breakpoint)
{
    remove_breakpoint = false;

    SIZE_T processed_bytes;
    UInt32 mutex_name_ptr;
    if (!ReadProcessMemory(process_information.hProcess, (LPCVOID) (context.Esp + 0x0C), &mutex_name_ptr, sizeof (UInt32), &processed_bytes))
        return false;

    if (mutex_name_ptr == 0)
        return true;

    UInt64 buffer_length;
    if (!queryAllocationLength(mutex_name_ptr, buffer_length))
        return false;

    QString mutex_prefix = "AN-Mutex-";
    if (buffer_length < mutex_prefix.length())
        return true;

    QByteArray temp_buffer;
    int temp_buffer_length = mutex_prefix.length();

    temp_buffer.resize(temp_buffer_length + 1);
    if (temp_buffer.size() != temp_buffer_length + 1)
        return false;
    temp_buffer.fill(0);

    if (!ReadProcessMemory(process_information.hProcess, (LPCVOID) mutex_name_ptr, temp_buffer.data(), temp_buffer_length, &processed_bytes))
        return false;

    if (mutex_prefix.compare(temp_buffer.constData(), Qt::CaseInsensitive) != 0)
        return true;

    DWORD pid = GetProcessId(process_information.hProcess);
    QString new_mutex_name = QString::number((ulong) pid);
    if (!WriteProcessMemory(process_information.hProcess, (LPVOID) mutex_name_ptr, (LPCVOID) new_mutex_name.toLatin1().constData(), new_mutex_name.size(), &processed_bytes))
        return false;

    mutex_count++;
    if (mutex_count == 3)
        remove_breakpoint = true;

    return true;
}

bool GameLauncher::onCreateFileWBreakpoint(CONTEXT & context, bool & remove_breakpoint)
{
    remove_breakpoint = false;

    SIZE_T processed_bytes;
    UInt32 file_name_ptr;
    if (!ReadProcessMemory(process_information.hProcess, (LPCVOID) (context.Esp + 0x04), &file_name_ptr, sizeof (UInt32), &processed_bytes))
        return false;

    if (file_name_ptr == 0)
        return true;

    UInt64 buffer_length;
    if (!queryAllocationLength(file_name_ptr, buffer_length))
        return false;
    buffer_length++;

    QByteArray temp_buffer;
    temp_buffer.resize(buffer_length);
    if (temp_buffer.size() != buffer_length)
        return false;
    temp_buffer.fill(0);

    if (!ReadProcessMemory(process_information.hProcess, (LPCVOID) file_name_ptr, temp_buffer.data(), buffer_length - 1, &processed_bytes))
        return false;

    QString new_remote_file_name;
    QString remote_file_name = QString::fromWCharArray((const wchar_t *) temp_buffer.constData());
    bool data_file;

    if (remote_file_name.contains("Gw2.tmp", Qt::CaseInsensitive))
    {
        new_remote_file_name = profile_base_path + "_Gw2.tmp";
        data_file = false;
    }
    else if (remote_file_name.contains("Local.tmp", Qt::CaseInsensitive))
    {
        new_remote_file_name = profile_base_path + "_Local.tmp";
        data_file = false;
    }
    else if (remote_file_name.contains("Local.dat", Qt::CaseInsensitive))
    {
        new_remote_file_name = profile_base_path + "_Local.dat";
        data_file = false;
    }
    else if (remote_file_name.contains("Gw2.dat", Qt::CaseInsensitive))
    {
        remove_breakpoint = true;
        data_file = true;
    }
    else
        return true;

    if (data_file)
    {
        if (standalone_mode)
            return true;

        // desired access
        // setting GENERIC_READ will protect the game file from any modification; this also means that if you try to update this game instance
        // you will crash
        UInt32 parameter = GENERIC_READ;
        if (!WriteProcessMemory(process_information.hProcess, (LPVOID)(context.Esp + 0x08), &parameter, sizeof (UInt32), &processed_bytes))
            return false;

        // share mode
        // allow other game instances to *READ* the Gw2.dat file
        if (!ReadProcessMemory(process_information.hProcess, (LPCVOID)(context.Esp + 0x0C), &parameter, sizeof (UInt32), &processed_bytes))
            return false;

        parameter |= FILE_SHARE_READ;
        if (!WriteProcessMemory(process_information.hProcess, (LPVOID)(context.Esp + 0x0C), &parameter, sizeof (UInt32), &processed_bytes))
            return false;
    }
    else
    {
        int file_path_size = (new_remote_file_name.size() + 1) * sizeof (WCHAR);

        void *remote_buffer = VirtualAllocEx(process_information.hProcess, NULL, file_path_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (remote_buffer == NULL)
            return false;

        if (!WriteProcessMemory(process_information.hProcess, remote_buffer, new_remote_file_name.utf16(), file_path_size, &processed_bytes))
            return false;

        if (!WriteProcessMemory(process_information.hProcess, (LPVOID) (context.Esp + 0x04), &remote_buffer, sizeof (UInt32), &processed_bytes))
            return false;
    }

    return true;
}
