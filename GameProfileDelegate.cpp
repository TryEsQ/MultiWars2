#include "GameProfileDelegate.h"
#include "GameProfile.h"
#include <QFont>

#define LABEL_GAME_FOLDER "Game folder: "
#define LABEL_COMMAND_LINE "Command line: "
#define LABEL_SHARED_PROFILE_MODE "Shared"
#define LABEL_STANDALONE_PROFILE_MODE "Standalone"

GameProfileDelegate::GameProfileDelegate(QObject *parent) : QStyledItemDelegate(parent), normal_font_metrics(QFont()), title_font_metrics(QFont())
{
    normal_font = QFont("Consolas");
    normal_font.setPixelSize(12);
    normal_font_metrics = QFontMetrics(normal_font);

    title_font = QFont("Segoe UI");
    title_font.setPixelSize(normal_font.pixelSize() * 2);
    title_font.setBold(true);
    title_font_metrics = QFontMetrics(title_font);

    margin = normal_font_metrics.width(" ");
    width_padding = normal_font_metrics.width(tr(LABEL_STANDALONE_PROFILE_MODE));

    height = (margin * 5) + title_font_metrics.height() + normal_font_metrics.height() * 2;
}

void GameProfileDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QString profile_label = getProfileLabel(index);
    QString profile_mode_label = getProfileModeLabel(index);
    QString game_folder_label = getGameFolderLabel(index);
    QString command_line_label = getCommandLineLabel(index);
    QPixmap avatar = getAvatarImage(index);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    if ((option.state & QStyle::State_Selected) != 0)
        painter->fillRect(option.rect, option.palette.dark());
    else
    {
        QBrush background_color;

        int row = index.row();
        if ((row > 0) && (row % 2 != 0))
            background_color = option.palette.alternateBase();
        else
            background_color = option.palette.base();

        painter->fillRect(option.rect, background_color);
    }

    // avatar
    painter->drawPixmap(margin, option.rect.y() + ((height / 2) - (IDENTICON_SIZE / 2)), avatar);

    int x = option.rect.x() + margin + IDENTICON_SIZE + margin;
    int y = option.rect.y() + title_font_metrics.height();

    // profile name
    painter->setFont(title_font);
    painter->drawText(x, y, profile_label);

    // game folder
    painter->setFont(normal_font);

    y += margin + normal_font_metrics.height();
    painter->drawText(x, y, game_folder_label);

    // command line
    y += margin + normal_font_metrics.height();
    painter->drawText(x, y, command_line_label);

    // profile mode
    y = option.rect.y();
    y += margin + normal_font_metrics.height();
    x = option.rect.width() - normal_font_metrics.width(profile_mode_label) - margin;
    painter->drawText(x, y, profile_mode_label);

    painter->restore();
}

QSize GameProfileDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    (void) option;

    QString profile_label = getProfileLabel(index);
    QString profile_mode_label = getProfileModeLabel(index);
    QString game_folder_label = getGameFolderLabel(index);
    QString command_line_label = getCommandLineLabel(index);

    int width = (margin * 2) + title_font_metrics.width(profile_label);

    int current_label_width;
    if (width < (current_label_width = normal_font_metrics.width(profile_mode_label)))
        width = current_label_width;

    if (width < (current_label_width = normal_font_metrics.width(game_folder_label)))
        width = current_label_width;

    if (width < (current_label_width = normal_font_metrics.width(command_line_label)))
        width = current_label_width;

    width += IDENTICON_SIZE + margin + width_padding;

    return QSize(width, height);
}

QString GameProfileDelegate::getProfileLabel(const QModelIndex & index) const
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();
    return model->data(model->index(profile_index, 0)).toString();
}

QString GameProfileDelegate::getProfileModeLabel(const QModelIndex & index) const
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();

    bool standalone_mode = model->data(model->index(profile_index, 1)).toBool();
    if (standalone_mode)
        return tr(LABEL_STANDALONE_PROFILE_MODE);
    else
        return tr(LABEL_SHARED_PROFILE_MODE);
}

QString GameProfileDelegate::getGameFolderLabel(const QModelIndex & index) const
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();
    return tr(LABEL_GAME_FOLDER) + model->data(model->index(profile_index, 2)).toString();
}

QString GameProfileDelegate::getCommandLineLabel(const QModelIndex & index) const
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();
    return tr(LABEL_COMMAND_LINE) + model->data(model->index(profile_index, 3)).toString();
}

QPixmap GameProfileDelegate::getAvatarImage(const QModelIndex & index) const
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();
    return model->data(model->index(profile_index, 4)).value<QPixmap>();
}
