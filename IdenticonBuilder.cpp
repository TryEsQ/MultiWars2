/*
 * This is an ugly hack that allow us to use any of the several different
 * identicon-like avatar generation scripts without any modification AT ALL!
 *
 * Credits for the Javascript code:
 * jQuery (John Resig)
 * Sizzle.js (The Dojo Foundation)
 * identicon5.js (Francis Shanahan)
 */

#include "IdenticonBuilder.h"

#include <QObject>
#include <QString>
#include <QtWebKit>
#include <QWebView>
#include <QPixmap>
#include <QCryptographicHash>

static QString GetIdenticonHTMLPagePart_P(int part)
{
    if (part <= 0 || part > 3)
        return "";

    QString resource_path = QString(":/identicon_part") + QString::number(part);

    QFile html_page(resource_path);
    if (!html_page.open(QFile::ReadOnly))
        return "";

    return QString(html_page.readAll());
}

static QString GetIdenticonHTMLPage_P(QString string_to_hash, int identicon_size)
{
    QString md5_hash = QCryptographicHash::hash(string_to_hash.toLatin1(), QCryptographicHash::Md5).toHex();
    return GetIdenticonHTMLPagePart_P(1) + QString::number(identicon_size) + GetIdenticonHTMLPagePart_P(2) + md5_hash + GetIdenticonHTMLPagePart_P(3);
}

QImage GetIdenticon(QString string_to_hash, int size)
{
    QString page_content = GetIdenticonHTMLPage_P(string_to_hash, size);

    QWebView web_view;
    web_view.setHtml(page_content);
    while (web_view.paintingActive());

    return web_view.grab(QRect(0, 0, size, size)).toImage();
}
