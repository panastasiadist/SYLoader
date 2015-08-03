/*******************************************************************************
 * Copyright 2015 Panagiotis Anastasiadis
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

#include "global.h"
#include "mainwindow.h"
#include "scheduler.h"
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QDesktopServices>
#include <QtWidgets/QMessageBox>
#include <QFile>
#include <QDir>

QSettings *Settings;
Scheduler *Tasks;
Messenger *MessageBus;
GatewayPool *Gateway;


int
main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("SYLoader");
    QApplication::setOrganizationName("SYLoader");


    Settings = new QSettings();
    MessageBus = new Messenger();
    Gateway = new GatewayPool();
    Tasks = new Scheduler();
    Tasks->setConcurrentTasks(1);


    if (Settings->contains("download_path") == false) {
        QVariant path = QStandardPaths
                ::standardLocations(QStandardPaths::DesktopLocation)
                .at(0);
        Settings->setValue("download_path", path);
    }


    if (Settings->contains("sim_downloads") == false) {
        Settings->setValue("sim_downloads", 1);
    }


    if (Settings->contains("download_mode") == false) {
        Settings->setValue("download_mode", 0);
    }


    if (Settings->contains("autocheck_updates") == false) {
        Settings->setValue("autocheck_updates", true);
    }


    if (Settings->contains("autostart") == false) {
        Settings->setValue("autostart", true);
    }


    QTranslator translator;
    QString language = (Settings->value("language", "notset")).toString();


    /* Load saved or default language
     * First time the program is run.
     * None language is set.
     * If available, try applying the system language
     */
    if (language == "notset")
    {
        QString systemLanguage = QLocale
                ::languageToString(QLocale::system().language());

        bool applySystemLanguage = false;

        QDir directory (QString("languages"), QString("*.qm"));
        QStringList languages = directory.entryList();

        foreach (QString language, languages)
        {
            QString languageName = language.replace(".qm","");
            if (languageName == systemLanguage)
            {
                applySystemLanguage = true;
                break;
            }
        }

        // The system language was found installed
        if (applySystemLanguage)
        {
            translator.load(QString("languages/%1.qm").arg(systemLanguage));
            Settings->setValue("language", systemLanguage);
        }
    }
    else {
        translator.load(QString("languages/%1.qm").arg(language));
    }


    a.installTranslator(&translator);



    MainWindow w;
    w.show();

    return a.exec();
}
