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
#include <QDir>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>
#include "settings_form.h"
#include "ui_settings_form.h"
#include "global.h"



SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

    ui->cbxAutocheckUpdates->setChecked(
        Settings->value("autocheck_updates", QVariant(true)).toBool());

    ui->sbxDownloads->setValue(
        Settings->value("sim_downloads", QVariant(4)).toInt());

    ui->cbxAutostart->setChecked(
        Settings->value("autostart", QVariant(true)).toBool());

    ui->txtSavepath->setText(Settings->value("download_path").toString());


    connect (ui->btnBrowse, SIGNAL(clicked()), this, SLOT(onBrowseClicked()));
    connect (ui->btnSave, SIGNAL(clicked()), this, SLOT(onSaveClicked()));


    // Load languages found in languages directory

    QDir directory (QString("translations"), QString("*.qm"));
    QStringList languages = directory.entryList();
    foreach (QString language, languages)
    {
        QString languageName = language.replace(".qm","");
        ui->cbxLanguage->addItem(languageName);
    }


    // Add the builtin language

    ui->cbxLanguage->addItem("English");


    // Select current language

    QString language = (Settings->value("language", "English")).toString();
    for (int index = 0; index < ui->cbxLanguage->count(); index++)
    {
        if (language == ui->cbxLanguage->itemText(index))
        {
            ui->cbxLanguage->setCurrentIndex(index);
            break;
        }
    }
}



SettingsForm::~SettingsForm()
{
    delete ui;
}



void
SettingsForm::onBrowseClicked()
{
    QString d = QFileDialog::getExistingDirectory(
                this,
                tr("Select downloads directory"),
                ui->txtSavepath->text(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (d != "")
        ui->txtSavepath->setText(d);
}



void
SettingsForm::onSaveClicked()
{
    Settings->setValue(
        "language",
        ui->cbxLanguage->itemText(ui->cbxLanguage->currentIndex()));

    Settings->setValue(
        "autocheck_updates",
        ui->cbxAutocheckUpdates->isChecked());

    Settings->setValue("sim_downloads", ui->sbxDownloads->value());

    Settings->setValue("autostart", ui->cbxAutostart->isChecked());

    Settings->setValue("download_path", ui->txtSavepath->text());

    Settings->sync();

    QMessageBox::information(
                this,
                tr("Information"),
                tr("Your settings have been updated!"));

}
