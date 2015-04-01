#ifndef GAMEPROFILE_H
#define GAMEPROFILE_H

#include <QString>
#include <QList>
#include <QPixmap>

class GameProfile
{
public:
    enum ProfileType
    {
        Standalone,
        Shared,
        Uninitialized
    };

    explicit GameProfile();

    bool isValid(QString & error_description) const;

    QString getProfileName() const;
    void setProfileName(const QString & profile_name);

    ProfileType getProfileType() const;
    void setProfileType(const ProfileType & profile_type);

    QString getGameFolder() const;
    void setGameFolder(const QString & game_folder);

    QString getCommandLine() const;
    void setCommandLine(const QString & command_line);

    QString getAvatarPath() const;
    void setAvatarPath(const QString & avatar_path);
    QPixmap getAvatarImage() const;

private:
    void loadAvatar() const;
    void createIdenticon() const;

private:
    QString profile_name;
    QString game_folder;
    QString command_line;
    ProfileType profile_type;
    QString avatar_path;
    mutable QPixmap avatar_image;
};

typedef QList<GameProfile> GameProfileList;

#endif // GAMEPROFILE_H
