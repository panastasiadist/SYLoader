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



#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QFile>
#include "about_form.h"
#include "ui_about_form.h"
#include "global.h"
#include "updater.h"





AboutForm::AboutForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutForm)
{
    ui->setupUi(this);

    connect (ui->lbtnHomepage,
             SIGNAL(clicked()),
             this,
             SLOT(onHomepageClicked()));

    connect (ui->lbtnContact,
             SIGNAL(clicked()),
             this,
             SLOT(onContactClicked()));

    connect (ui->lbtnLicense,
             SIGNAL(clicked()),
             this,
             SLOT(onLicenseClicked()));

#if defined(WITH_OPENSSL_NOTICE)
    ui->lblOpenSSLNotice->setVisible(true);
#else
    ui->lblOpenSSLNotice->setVisible(false);
#endif

    QFile file("CONTRIBUTORS.txt");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray bytes = file.readAll();
        ui->txtContributors->setText(QString(bytes));
        file.close();
    }

}





AboutForm::~AboutForm()
{
    delete ui;
}





void
AboutForm::onContactClicked()
{
    QDesktopServices services;
    services.openUrl(QUrl(CONTACT_URL));
}





void
AboutForm::onHomepageClicked()
{
    QDesktopServices services;
    services.openUrl(QUrl(HOMEPAGE_URL));
}





void
AboutForm::onLicenseClicked()
{
    QDesktopServices services;
    services.openUrl(QUrl("COPYING.txt"));
}
