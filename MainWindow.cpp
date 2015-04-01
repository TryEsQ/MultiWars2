#include "MainWindow.h"
#include "GameProfileListModel.h"
#include "GameProfileDelegate.h"

#include <QListView>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
}

MainWindow::~MainWindow()
{
}

bool MainWindow::initialize()
{
    GameProfileListModel *game_profiles_model = NULL;
    QListView *profile_list = NULL;
    GameProfileDelegate *game_profile_delegate = NULL;

    try
    {
        game_profiles_model = new GameProfileListModel();
        if (game_profiles_model == NULL)
            throw 1;

        if (!connect(game_profiles_model, SIGNAL(gameProfileError(QString)), this, SLOT(gameProfileError(QString))))
            throw 1;

        QString profile_data_path(QDir::currentPath() + "/profile_data");
        QDir profile_data_directory(profile_data_path);

        if (!profile_data_directory.exists())
        {
            if (!QDir::current().mkdir(profile_data_path))
            {
                QMessageBox::critical(this, tr("Error"), tr("The \"profile_data\" directory could not be created."));
                throw 1;
            }
        }

        QString profiles_folder = QDir::currentPath() + "/profiles";
        if (!game_profiles_model->loadFromFolder(profiles_folder))
        {
            QMessageBox::critical(this, tr("Error"), tr("No profile could be loaded. Please check your configuration again and restart the program."));
            throw 1;
        }

        profile_list = new QListView();
        if (profile_list == NULL)
            throw 1;

        profile_list->setModel(game_profiles_model);

        game_profile_delegate = new GameProfileDelegate();
        if (game_profile_delegate == NULL)
            throw 1;

        profile_list->setItemDelegate(game_profile_delegate);

        statusBar()->showMessage(tr("Remember to check for updates on http://insanecoder.net!"));

        setCentralWidget(profile_list);
        resize(640, 480);

        if (!connect(profile_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(startGameProfile(QModelIndex))))
            throw 1;

        if (!connect(&game_launcher, SIGNAL(finished()), this, SLOT(gameProfileStarted())))
            throw 1;

        setWindowIcon(QIcon(":/program_icon"));
        return true;
    }

    catch (...)
    {
        delete game_profiles_model;
        delete profile_list;
        delete game_profile_delegate;

        return false;
    }
}

void MainWindow::closeEvent(QCloseEvent *close_event)
{
    if (centralWidget()->isEnabled())
        close_event->accept();
    else
        close_event->ignore();
}

void MainWindow::gameProfileError(const QString & error_description)
{
    QMessageBox::critical(this, "MultiWars2", error_description);
}

void MainWindow::startGameProfile(const QModelIndex & index)
{
    const QAbstractItemModel *model = index.model();
    int profile_index = index.row();

    QString profile_name = model->data(model->index(profile_index, 0)).toString();
    bool standalone_mode = model->data(model->index(profile_index, 1)).toBool();
    QString game_folder = model->data(model->index(profile_index, 2)).toString();
    QString command_line = model->data(model->index(profile_index, 3)).toString();

    statusBar()->showMessage(tr("Starting game profile \"") + profile_name + "\"...");
    centralWidget()->setEnabled(false);

    game_launcher.setGameFolder(game_folder);
    game_launcher.setCommandLine(command_line);
    game_launcher.enableStandaloneMode(standalone_mode);
    game_launcher.setProfileName(profile_name);
    game_launcher.start();

    return;
}

void MainWindow::gameProfileStarted()
{
    centralWidget()->setEnabled(true);

    QString error_messages = game_launcher.getErrorMessages();
    if (error_messages.size() != 0)
    {
        QMessageBox::critical(this, "MultiWars2", tr("The game profile could not be started due to the following error(s): ") + error_messages);
        statusBar()->showMessage(tr("Failed to start the game"));
    }
    else
        statusBar()->showMessage(tr("The game has been started"));
}
