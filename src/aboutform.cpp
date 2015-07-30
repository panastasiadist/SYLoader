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

#include "aboutform.h"
#include "ui_aboutform.h"
#include "global.h"
#include "updater.h"
#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>





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
