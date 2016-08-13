/*******************************************************************************
 * Copyright 2016 Panagiotis Anastasiadis
 * This file is part of SYLoader.
 *
 * SYLoader is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * SYLoader is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SYLoader. If not, see http://www.gnu.org/licenses.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL. If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so. If you
 * do not wish to do so, delete this exception statement from your
 * version. If you delete this exception statement from all source
 * files in the program, then also delete it here.
 ******************************************************************************/
#include <QDesktopServices>
#include <QMessageBox>
#include "main_window.h"
#include "ui_main_window.h"
#include "global.h"
#include "main_form.h"
#include "settings_form.h"
#include "about_form.h"
#include "update_form.h"
#include "messenger.h"
#include "updater.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _downloading = false;

    ui->gridMain->addWidget(new MainForm());
    ui->gridSettings->addWidget(new SettingsForm());
    ui->gridAbout->addWidget((new AboutForm()));
    ui->gridUpdates->addWidget(new UpdateForm());
    ui->container->setCurrentIndex(0);
    ui->btnMain->setVisible(false);

    restoreGeometry(Settings->value("window_geometry").toByteArray());
    restoreState(Settings->value("window_state").toByteArray());


    connect(ui->btnMain,
            SIGNAL(clicked()),
            this,
            SLOT(onMainClicked()));

    connect(ui->btnSettings,
            SIGNAL(clicked()),
            this,
            SLOT(onSettingsClicked()));

    connect(ui->btnAbout,
            SIGNAL(clicked()),
            this,
            SLOT(onAboutClicked()));

    connect(ui->btnTwitter,
            SIGNAL(clicked()),
            this,
            SLOT(onTwitterClicked()));

    connect(ui->btnFacebook,
            SIGNAL(clicked()),
            this,
            SLOT(onFacebookClicked()));

    connect(MessageBus,
            SIGNAL(receive(QString)),
            this,
            SLOT(onMessageBusReceive(QString)));


    this->installEventFilter(this);

    //this->setGeometry(20,20,370,220);
    //this->setGeometry(20,20,700,416);

    // For website screenshot
    //this->setGeometry(20, 20, 858, 477);





#ifdef WITH_UPDATE_CHECK
    if (Settings->value("autocheck_updates", QVariant(true)) == true)
    {
        updater = new Updater();

        connect(updater,
                SIGNAL(updateCheckFinished(bool)),
                this,
                SLOT(onUpdateCheckFinished(bool)));

        connect(updater,
                SIGNAL(updateInstallationFinished(bool)),
                this,
                SLOT(onUpdateInstallationFinished(bool)));

        updater->check();
    }
#endif

    ui->statusBar->showMessage(
        tr("Download by copying YouTube URLs in your browser!"));
}



bool
MainWindow::eventFilter (QObject *object, QEvent *event)
{
    // Prevent the program from closing if downloading is in progress.

    if (event->type() == QEvent::Close && object == this)
    {
        if (_downloading)
        {
            event->ignore();
            QString msg = tr("You have one or more downloads in progress. "
                             "You should cancel them first.");
            QMessageBox::information(this, tr("Information"), msg);
            return true;
        }
        else
        {
            Settings->setValue("window_geometry", saveGeometry());
            Settings->setValue("window_state", saveState());
            Settings->sync();
        }
    }

    return QMainWindow::eventFilter(object, event);
}



MainWindow::~MainWindow()
{
    disconnect(MessageBus);

    delete Settings;
    delete Tasks;
    delete MessageBus;
    delete ui;
}



void
MainWindow::onMessageBusReceive(QString msg)
{
    if (msg == "parsing_started")
    {
        ui->statusBar->showMessage(tr("Preparing your new downloads..."));
    }
    else if (msg == "parsing_finished")
    {
        ui->statusBar->showMessage(
            tr("Download by copying YouTube URLs in your browser!"));
    }
    else if (msg == "downloading_started")
    {
        _downloading = true;
    }
    else if (msg == "downloading_finished")
    {
        _downloading = false;
    }
}



void
MainWindow::onMainClicked()
{
    ui->container->setCurrentIndex(0);
    ui->btnMain->setVisible(false);
    ui->lblFormTitle->setText(tr("Downloads"));
}



void
MainWindow::onSettingsClicked()
{
    ui->container->setCurrentIndex(1);
    ui->btnMain->setVisible(true);
    ui->lblFormTitle->setText(tr("Settings"));
}



void
MainWindow::onAboutClicked()
{
    ui->container->setCurrentIndex(2);
    ui->btnMain->setVisible(true);
    ui->lblFormTitle->setText(tr("About"));
}



void
MainWindow::onFacebookClicked()
{
    QDesktopServices::openUrl(QUrl(FACEBOOK_URL));
}



void
MainWindow::onTwitterClicked()
{
    QDesktopServices::openUrl(QUrl(TWITTER_URL));
}



void
MainWindow::onUpdateCheckFinished(bool success)
{
    if (success == false) {
        return;
    }

    UpdateInfo info = updater->getUpdateData();

    bool ignoreDependencyUpdates = false;

    if (info.ProgramUpdate)
    {
        int msgret = QMessageBox::information(
            this,
            tr("New version available!"),
            tr("SYLoader has a new version available. You are advised to "
               "get the new improved version. Go to download page?"),
            QMessageBox::Ok | QMessageBox::Cancel);

        if (msgret == QMessageBox::Ok)
        {
            ignoreDependencyUpdates = true;
            this->close();
            QDesktopServices::openUrl(QUrl(DOWNLOADS_URL));
        }
    }


    // If a new SYLoader is available, then the new package will contain the
    // latest version of the dependencies. If the user has chosen to download
    // the new version, ignore required updates of dependencies.

    if (ignoreDependencyUpdates)
    {
        updater->deleteLater();
        return;
    }


    // There is a required dependency update. Start updating.

    if (info.YdlUpdate)
    {
        ui->container->setCurrentIndex(3);
        ui->btnMain->setVisible(false);
        ui->btnSettings->setVisible(false);
        ui->btnAbout->setVisible(false);
        ui->lblFormTitle->setText(tr("Required Updates"));

        updater->updateYdl();
    }
    else
    {
        updater->deleteLater();
    }

}



void
MainWindow::onUpdateInstallationFinished(bool success)
{
    updater->deleteLater();

    ui->container->setCurrentIndex(0);
    ui->lblFormTitle->setText(tr("Downloads"));
    ui->btnMain->setVisible(true);
    ui->btnSettings->setVisible(true);
    ui->btnAbout->setVisible(true);

    if (success == false)
    {
#ifdef Q_OS_WIN32
        QString message = tr("SYLoader failed to install updates. Please try "
                             "closing SYLoader and launching it as "
                             "Administrator (right click on shortcut)");
#else
        QString message = tr("SYLoader failed to install updates. Make sure "
                             "that SYLoader has permission to write in its "
                             "directory");
#endif
        QMessageBox::information(
                    this,
                    tr("Update failed"),
                    message,
                    QMessageBox::Ok);
    }
}
