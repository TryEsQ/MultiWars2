#ifndef GAMEPROFILELISTMODEL_H
#define GAMEPROFILELISTMODEL_H

#include <QAbstractListModel>
#include <QModelIndex>
#include <QVariant>

#include "GameProfile.h"

class GameProfileListModel : public QAbstractListModel
{
private:
    Q_OBJECT

    GameProfileList profiles;

public:
    explicit GameProfileListModel(QObject *parent = NULL);
    virtual ~GameProfileListModel();

    bool loadFromFolder(const QString & path);

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex & parent) const;

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

private:
    bool insertGameProfile(const GameProfile & profile, QString & error_description);

signals:
    void gameProfileError(const QString & error_description);
};

#endif // GAMEPROFILELISTMODEL_H
