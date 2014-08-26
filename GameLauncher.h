#ifndef GAMELAUNCHER_H
#define GAMELAUNCHER_H

#include <QThread>
#include <windows.h>

#include "data_types.h"

class GameLauncher : public QThread
{
private:
    Q_OBJECT

    QString game_folder;
    QString command_line;
    QString profile_name;
    bool standalone_mode;
    bool game_started;

    int mutex_count;
    QString error_messages;

    PROCESS_INFORMATION process_information;
    QString profile_base_path;
    QString working_directory;
    QString executable_path;

public:
    explicit GameLauncher(QObject *parent = NULL);

    void setGameFolder(const QString & game_folder);
    void setCommandLine(const QString & command_line);
    void setProfileName(const QString & profile_name);
    void enableStandaloneMode(bool enable);

    const QString & getErrorMessages();

    virtual void run();
    bool hasSucceeded();

private:
    bool setSoftwareBreakpoint(UInt64 address, UInt8 & overwritten_byte);
    bool removeSoftwareBreakpoint(UInt64 address, UInt8 original_byte);
    void setSingleStepFlag(CONTEXT & context);
    bool queryAllocationLength(UInt64 address, UInt64 & length);
    bool isInstanceRunning(bool & running);
    bool processDebugEvents();

    bool onCreateMutexABreakpoint(CONTEXT & context, bool & remove_breakpoint);
    bool onCreateFileWBreakpoint(CONTEXT & context, bool & remove_breakpoint);
};

#endif // GAMELAUNCHER_H
