#include "GameProfile.h"
#include "IdenticonBuilder.h"

#include <QObject>
#include <QDir>
#include <QImage>

GameProfile::GameProfile()
{
    profile_type = GameProfile::Uninitialized;
}

bool GameProfile::isValid(QString & error_description) const
{
    if (profile_type == GameProfile::Uninitialized)
    {
        error_description = QObject::tr("The game profile is not initialized");
        return false;
    }

    if (profile_name.isEmpty())
    {
        error_description = QObject::tr("The profile name parameter is missing");
        return false;
    }

    if (game_folder.isEmpty())
    {
        error_description = QObject::tr("The game folder parameter is missing");
        return false;
    }

    QDir folder(game_folder);
    if (!folder.exists())
    {
        error_description = QObject::tr("The game folder does not exists");
        return false;
    }

    if (getAvatarImage().isNull())
    {
        error_description = QObject::tr("The profile avatar is not valid");
        return false;
    }

    return true;
}

QString GameProfile::getProfileName() const
{
    return profile_name;
}

void GameProfile::setProfileName(const QString & profile_name)
{
    this->profile_name = profile_name;
}

GameProfile::ProfileType GameProfile::getProfileType() const
{
    return profile_type;
}

void GameProfile::setProfileType(const GameProfile::ProfileType & profile_type)
{
    this->profile_type = profile_type;
}

QString GameProfile::getGameFolder() const
{
    return game_folder;
}

void GameProfile::setGameFolder(const QString & game_folder)
{
    this->game_folder = game_folder;
}

QString GameProfile::getCommandLine() const
{
    return command_line;
}

void GameProfile::setCommandLine(const QString & command_line)
{
    this->command_line = command_line;
}

QString GameProfile::getAvatarPath() const
{
    return avatar_path;
}

void GameProfile::setAvatarPath(const QString & avatar_path)
{
    this->avatar_path = avatar_path;
}

QPixmap GameProfile::getAvatarImage() const
{
    if (avatar_image.isNull())
    {
        if (avatar_path.length() != 0)
            loadAvatar();
        else
            createIdenticon();
    }

    return avatar_image;
}

void GameProfile::loadAvatar() const
{
    QImage image_file(avatar_path);
    avatar_image = avatar_image.fromImage(image_file);
}

void GameProfile::createIdenticon() const
{
    QImage identicon = GetIdenticon(getProfileName());
    avatar_image = avatar_image.fromImage(identicon);
}
