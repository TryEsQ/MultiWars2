#include "GameProfileListModel.h"
#include <QDir>
#include <QFileInfoList>
#include <QSettings>
#include <QMessageBox>

GameProfileListModel::GameProfileListModel(QObject *parent) : QAbstractListModel(parent)
{
}

GameProfileListModel::~GameProfileListModel()
{
}

bool GameProfileListModel::loadFromFolder(const QString & path)
{
    profiles.clear();

    QDir folder(path);
    if (!folder.exists())
        return false;

    QFileInfoList profile_files = folder.entryInfoList(QStringList("*.ini"));
    if (profile_files.size() == 0)
        return true;

    int profile_count = profile_files.count();
    for (int i = 0; i < profile_count; i++)
    {
        QString profile_name, profile_type, game_folder, command_line, avatar_path;
        {
            QSettings configuration(profile_files.at(i).absoluteFilePath(), QSettings::IniFormat);
            profile_name = configuration.value("ProfileName").toString();
            profile_type = configuration.value("StandaloneMode").toString();
            game_folder = configuration.value("GameFolder").toString();
            command_line = configuration.value("CommandLine").toString();
            avatar_path = configuration.value("Avatar").toString();
        }

        for (int k = game_folder.length() - 1; k > 0; k--)
        {
            if (game_folder[k] != '\\')
                break;

            game_folder.resize(k);
        }

        GameProfile profile;
        profile.setProfileName(profile_name);
        profile.setGameFolder(game_folder);
        profile.setCommandLine(command_line);
        profile.setAvatarPath(avatar_path);

        if (QString::compare(profile_type, "true", Qt::CaseInsensitive) == 0)
            profile.setProfileType(GameProfile::Standalone);
        else if (QString::compare(profile_type, "false", Qt::CaseInsensitive) == 0)
            profile.setProfileType(GameProfile::Shared);

        QString error_description;
        if (!insertGameProfile(profile, error_description))
        {
            QString error_message = tr("The following profile could not be loaded: ");
            error_message += QString("\"") + profile_files.at(i).absoluteFilePath() + QString("\"");

            error_message += QString("\n\n");
            error_message += tr("Error description:") + QString("\n");
            error_message += error_description;

            emit gameProfileError(error_message);
        }
    }

    if (profiles.size() == 0)
        return false;

    return true;
}

int GameProfileListModel::rowCount(const QModelIndex & parent) const
{
    (void) parent;
    return profiles.count();
}

int GameProfileListModel::columnCount(const QModelIndex & parent) const
{
    (void) parent;
    return 5;
}

Qt::ItemFlags GameProfileListModel::flags(const QModelIndex & index) const
{
    (void) index;
    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVariant GameProfileListModel::data(const QModelIndex & index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant(QVariant::Invalid);

    if (!index.isValid())
        return QVariant(QVariant::Invalid);

    int row = index.row();
    if (row < 0 || row >= profiles.count())
        return QVariant(QVariant::Invalid);

    const GameProfile & profile = profiles.at(row);

    int column = index.column();
    switch (column)
    {
        case 0:
            return QVariant(profile.getProfileName());

        case 1:
            return QVariant(profile.getProfileType() == GameProfile::Standalone);

        case 2:
            return QVariant(profile.getGameFolder());

        case 3:
            return QVariant(profile.getCommandLine());

        case 4:
            return QVariant(profile.getAvatarImage());

        default:
            return QVariant(QVariant::Invalid);
    }
}

bool GameProfileListModel::insertGameProfile(const GameProfile & profile, QString & error_description)
{
    error_description.clear();

    if (!profile.isValid(error_description))
        return false;

    int profile_count = profiles.count();
    for (int i = 0; i < profile_count; i++)
    {
        GameProfile current_profile = profiles.at(i);
        if (profile.getProfileName().compare(current_profile.getProfileName(), Qt::CaseInsensitive) == 0)
        {
            error_description = tr("A profile with the same name already exists");
            return false;
        }

        if (profile.getGameFolder().compare(current_profile.getGameFolder(), Qt::CaseInsensitive) == 0)
        {
            if (profile.getProfileType() == current_profile.getProfileType() && profile.getProfileType() != GameProfile::Shared)
            {
                error_description = tr("A standalone profile with the same game folder already exists");
                return false;
            }
        }
    }

    profiles.append(profile);
    return true;
}
