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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "parser.h"
#include "download.h"
#include "processor.h"
#include "progressitemdelegate.h"
#include "processorstats.h"
#include <QMainWindow>
#include <QList>
#include <QClipboard>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool _downloading;
    bool eventFilter (QObject *object, QEvent *event);

private slots:
    void onMessageBusReceive(QString msg);
    void onMainClicked();
    void onSettingsClicked();
    void onAboutClicked();
    void onFacebookClicked();
    void onTwitterClicked();
};

#endif // MAINWINDOW_H
