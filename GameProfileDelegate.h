#ifndef GAMEPROFILEDELEGATE_H
#define GAMEPROFILEDELEGATE_H

#include "GameProfileListModel.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QSize>
#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QImage>

#define IDENTICON_SIZE 64

class GameProfileDelegate : public QStyledItemDelegate
{
private:
    Q_OBJECT

    QFont normal_font, title_font;
    QFontMetrics normal_font_metrics, title_font_metrics;
    int margin, width_padding;
    int height;

public:
    explicit GameProfileDelegate(QObject *parent = NULL);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

private:
    QString getProfileLabel(const QModelIndex & index) const;
    QString getProfileModeLabel(const QModelIndex & index) const;
    QString getGameFolderLabel(const QModelIndex & index) const;
    QString getCommandLineLabel(const QModelIndex & index) const;
    QPixmap getAvatarImage(const QModelIndex & index) const;
};

#endif // GAMEPROFILEDELEGATE_H
