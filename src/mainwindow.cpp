/*******************************************************************************
 * Copyright 2015 Panagiotis Anastasiadis
 * This file is part of eTube Downloader.
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
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "parser.h"
#include "mainform.h"
#include "settingsform.h"
#include "aboutform.h"
#include "messenger.h"
#include "updater.h"
#include <QDesktopServices>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _downloading = false;

    ui->gridMain->addWidget(new MainForm());
    ui->gridSettings->addWidget(new SettingsForm());
    ui->gridAbout->addWidget((new AboutForm()));
    ui->container->setCurrentIndex(0);
    ui->btnMain->setVisible(false);




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

    connect(MessageBus,
            SIGNAL(receive(QString)),
            this,
            SLOT(onMessageBusReceive(QString)));



    // Attach event filters
    this->installEventFilter(this);



    if (Settings->value("autocheck_updates", QVariant(true)) == true)
    {
        bool update;
        bool error = Updater::checkForUpdates(update);
        if (error == false && update == true)
        {
            int msgret = QMessageBox::information(
                this,
                tr("New version available!"),
                tr("SYLoader has a new version available. You should get the new version in order to continue downloading. Go to downloads page?"),
                QMessageBox::Ok | QMessageBox::Cancel);

            if (msgret == QMessageBox::Ok)
                QDesktopServices::openUrl(QUrl(DOWNLOADS_URL));
        }
    }

    ui->statusBar->showMessage(
        tr("Download by copying YouTube URLs in your browser!"));

}


bool MainWindow::eventFilter (QObject *object, QEvent *event)
{
    // We won't allow the program to close if downloading is in progress
    if (event->type() == QEvent::Close && object == this)
    {
        if (_downloading) {
            event->ignore();
            QString msg = tr("You have one or more downloads in progress. You should stop them first.");
            QMessageBox::information(this, tr("Information"), msg);
            return true;
        }
        else
        {
            Settings->sync();
        }
    }

    return QMainWindow::eventFilter(object, event);
}


MainWindow::~MainWindow()
{
    delete Settings;
    delete ui;
}

void MainWindow::onMessageBusReceive(QString msg)
{
    if (msg == "parsing_started") {
        ui->statusBar->showMessage(tr("Preparing your new downloads..."));
    }
    else if (msg == "parsing_finished")
    {
        ui->statusBar->showMessage(
            tr("Download by copying YouTube URLs in your browser!"));
    } else if (msg == "downloading_started") {
        _downloading = true;
    } else if (msg == "downloading_finished") {
        _downloading = false;
    }
}

void MainWindow::onMainClicked()
{
    ui->container->setCurrentIndex(0);
    ui->btnMain->setVisible(false);
    ui->lblFormTitle->setText(tr("Downloads"));
}


void MainWindow::onUpdateCheckClicked()
{

}

void MainWindow::onSettingsClicked()
{
    ui->container->setCurrentIndex(1);
    ui->btnMain->setVisible(true);
    ui->lblFormTitle->setText(tr("Settings"));
}

void MainWindow::onAboutClicked()
{
    ui->container->setCurrentIndex(2);
    ui->btnMain->setVisible(true);
    ui->lblFormTitle->setText(tr("About"));

}



