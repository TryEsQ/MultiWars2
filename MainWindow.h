#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

#include "GameLauncher.h"

class MainWindow : public QMainWindow
{
private:
    Q_OBJECT

    GameLauncher game_launcher;

public:
    MainWindow(QWidget *parent = NULL);
    ~MainWindow();

    bool initialize();

protected slots:
    virtual void closeEvent(QCloseEvent *close_event);

private slots:
    void gameProfileError(const QString & error_description);

    void startGameProfile(const QModelIndex & index);
    void gameProfileStarted();
};

#endif // MAINWINDOW_H
