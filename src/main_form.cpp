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



#include <QMessageBox>

#include "main_form.h"
#include "ui_main_form.h"
#include "global.h"
#include "settings_form.h"
#include "about_form.h"
#include "messenger.h"
#include "core/output_format.h"



MainForm::MainForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);


    connect(ui->btnStart,
            SIGNAL(clicked()),
            this,
            SLOT(onStartClicked()));

    connect(ui->btnStop,
            SIGNAL(clicked()),
            this,
            SLOT(onStopClicked()));

    connect(ui->btnDelete,
            SIGNAL(clicked()),
            this,
            SLOT(onDeleteClicked()));

    connect(ui->btnClear,
            SIGNAL(clicked()),
            this,
            SLOT(onClearClicked()));

    connect(ui->btnDownload,
            SIGNAL(clicked()),
            this,
            SLOT(onDownloadClicked()));

    connect(ui->cbxMode,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(onModeCurrentIndexChanged(int)));

    connect(QApplication::clipboard(),
            SIGNAL(changed(QClipboard::Mode)),
            this,
            SLOT(onClipboardChanged(QClipboard::Mode)));

    connect(&_parser,
            SIGNAL(parsed(QList<Download>)),
            this,
            SLOT(onParserFinished(QList<Download>)));

    connect(&_processor,
            SIGNAL(downloadsStarted()),
            this,
            SLOT(onDownloadsStarted()));

    connect(&_processor,
            SIGNAL(downloadsFinished()),
            this,
            SLOT(onDownloadsFinished()));

    connect(&_processor,
            SIGNAL(downloadStatusChanged(int, Downloader::Status)),
            this,
            SLOT(onDownloadStatusChanged(int, Downloader::Status)));

    connect(&_processor,
            SIGNAL(downloadProgressChanged(int, DownloaderProgress)),
            this,
            SLOT(onDownloadProgressChanged(int, DownloaderProgress)));


    _processor.setAutoProcessing(false);
    _processor.setSavepath(Settings->value("download_path").toString());
    _processor.setConcurrentDownloads(Settings->value("sim_downloads").toInt());

    _downloadsModel.setHorizontalHeaderItem(0, new QStandardItem(tr("Title")));
    _downloadsModel.setHorizontalHeaderItem(1, new QStandardItem(tr("Status")));
    _downloadsModel.setHorizontalHeaderItem(2, new QStandardItem(tr("Progress")));
    _downloadsModel.setHorizontalHeaderItem(3, new QStandardItem(tr("Speed")));
    _downloadsModel.setHorizontalHeaderItem(4, new QStandardItem(tr("ETA")));

    ui->tvwDownloads->setModel(&_downloadsModel);
    ui->tvwDownloads->verticalHeader()->setVisible(false);
    ui->tvwDownloads->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvwDownloads->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvwDownloads->setColumnWidth(0, 300);
    ui->tvwDownloads->setColumnWidth(1, 90);
    ui->tvwDownloads->setColumnWidth(2, 80);
    ui->tvwDownloads->setColumnWidth(3, 70);
    ui->tvwDownloads->setColumnWidth(4, 70);
    ui->tvwDownloads->setItemDelegate(&_progressItemDelegate);


    OutputFormat mp3Format;
    mp3Format.title = tr("MP3 Music");
    mp3Format.extension = "mp3";
    mp3Format.isVideo = false;
    mp3Format.uid = 0;

    OutputFormat mp4Format;
    mp4Format.title = tr("MP4 Video");
    mp4Format.extension = "mp4";
    mp4Format.isVideo = true;
    mp4Format.uid = 1;

    OutputFormat aviFormat;
    aviFormat.title = tr("AVI Video");
    aviFormat.extension = "avi";
    aviFormat.isVideo = true;
    aviFormat.uid = 2;

    OutputFormat oggFormat;
    oggFormat.title = tr("OGG Music");
    oggFormat.extension = "ogg";
    oggFormat.isVideo = false;
    oggFormat.uid = 3;

    OutputFormat ogvFormat;
    ogvFormat.title = tr("OGV Video");
    ogvFormat.extension = "ogv";
    ogvFormat.isVideo = true;
    ogvFormat.uid = 4;

    _outputFormats.append(mp3Format);
    _outputFormats.append(mp4Format);
    _outputFormats.append(aviFormat);
    _outputFormats.append(oggFormat);
    _outputFormats.append(ogvFormat);


    foreach (OutputFormat format, _outputFormats) {
        ui->cbxMode->addItem(format.title);
    }


    int outputFormatId = Settings->value("download_mode").toInt();


    // Set Output Format according to stored format id.
    int idx;
    for (idx = 0; idx < _outputFormats.length(); idx++)
    {
        if (_outputFormats[idx].uid == outputFormatId) {
            ui->cbxMode->setCurrentIndex(idx);
        }
    }


    ui->btnDownload->setEnabled(false);

    _loaded = true;
}



MainForm::~MainForm()
{
    delete ui;
}



void
MainForm::onModeCurrentIndexChanged(int)
{
    applyCurrentMode();
}



void
MainForm::onClipboardChanged(QClipboard::Mode mode)
{
    if (mode == QClipboard::Clipboard)
    {
        QString url = QApplication::clipboard()->text();

        if (_parser.isSupported(url))
        {
            if (_registeredUrls.contains(url) == false) {
                registerAndParseUrl(url);
            }
        }
    }
}



void
MainForm::onStartClicked()
{
    QModelIndexList indexes = ui->tvwDownloads
                                ->selectionModel()
                                ->selection().indexes();

    int rowIndex = indexes.at(0).row();
    int downloadId = _idToRowIndex.key(rowIndex);

    Downloader::Status status = _processor.getDownloader(downloadId)
            ->getStatus();

    if (status == Downloader::Complete)
    {
        QMessageBox::information(
            this,
            tr("Information"),
            tr("Hooray! This download is already complete!")
        );
    }
    else
    {
        if (status != Downloader::Downloading &&
            status != Downloader::Converting)
        {
            _processor.start(downloadId);
        }
        else
        {
            QMessageBox::information(
                this,
                tr("Information"),
                tr("This download is already in progress.")
            );
        }
    }
}



void
MainForm::onStopClicked()
{
    QModelIndexList indexes = ui
                                ->tvwDownloads
                                ->selectionModel()
                                ->selection().indexes();

    int rowIndex = indexes.at(0).row();
    int downloadId = _idToRowIndex.key(rowIndex);

    Downloader::Status status = _processor.getDownloader(downloadId)
            ->getStatus();

    if (status == Downloader::Downloading ||
        status == Downloader::Converting) {
        _processor.stop(downloadId);
    }
    else
    {
        QMessageBox::information(
            this,
            tr("Information"),
            tr("This download is not currently in progress."));
    }
}



void
MainForm::onDownloadClicked()
{
    // If downloads in progress, then the button is used as a cancel button.
    if (_processor.running()) {
        _processor.stopAll();
    } else {
        _processor.process();
    }
}



void
MainForm::onDeleteClicked()
{
    QModelIndexList indexes = ui
                                ->tvwDownloads
                                ->selectionModel()
                                ->selection().indexes();

    int rowIndex = indexes.at(0).row();
    int downloadId = _idToRowIndex.key(rowIndex);

    Downloader *downloader = _processor.getDownloader(downloadId);
    Download *download = downloader->getDownload();
    Downloader::Status status = downloader->getStatus();

    if (status != Downloader::Downloading &&
        status != Downloader::Converting)
    {
        _processor.remove(downloadId);
        _registeredUrls.removeOne(download->normalUrl);
        _downloadsModel.removeRow(rowIndex);
    }
    else
    {
        QMessageBox::information(
            this,
            tr("Information"),
            tr("This download is in progress. Please cancel it first.")
        );
    }

    if (_downloadsModel.rowCount() == 0) {
        ui->btnDownload->setEnabled(false);
    }
}



void
MainForm::onClearClicked()
{
    if (_processor.running())
    {
        QMessageBox::information(
            this,
            tr("Information"),
            tr("One or more downloads are in progress. \
               Please cancel them first.")
        );
        return;
    }

    _processor.clear();
    _registeredUrls.clear();
    _idToRowIndex.clear();
    _downloadsModel.removeRows(0, _downloadsModel.rowCount());

    ui->btnDownload->setEnabled(false);
}



void
MainForm::onDownloadStatusChanged(int id, Downloader::Status status)
{
    QString statusStr = "";

    int rowIndex = _idToRowIndex.value(id);

    switch (status)
    {
        case Downloader::Ready:
            statusStr = tr("Queued");
            break;
        case Downloader::Downloading:
            statusStr = tr("Downloading");
            break;
        case Downloader::Converting:
            statusStr = tr("Converting");
            break;
        case Downloader::Complete:
            statusStr = tr("Complete");
            break;
        case Downloader::ErrorIO:
            statusStr = tr("I/O Error");
            break;
        case Downloader::ErrorConnection:
            statusStr = tr("Connection Error");
            break;
        case Downloader::Canceled:
            statusStr = tr("Canceled");
            break;
    }

    _downloadsModel.item(rowIndex, 1)->setText(statusStr);

    if (status != Downloader::Ready && status != Downloader::Downloading)
    {
        if (Settings->value("autostart") == true) {
            _processor.process();
        }
    }
}



void
MainForm::onDownloadProgressChanged(int id, DownloaderProgress progress)
{
    int rowIndex = _idToRowIndex.value(id);

    int kbps = progress.kbps;
    int minutes = progress.seconds / 60;
    int seconds = progress.seconds % 60;
    int percent = progress.percent;

    QString speedStr = QString("%1 KB/s").arg(kbps);
    QString percentStr = QString::number(percent);
    QString remainingStr = QString("%1m %2s").arg(minutes).arg(seconds);

    _downloadsModel.item(rowIndex, 2)->setText(percentStr);
    _downloadsModel.item(rowIndex, 3)->setText(speedStr);
    _downloadsModel.item(rowIndex, 4)->setText(remainingStr);
}



void
MainForm::onDownloadsStarted()
{
    ui->btnDownload->setText(tr("Cancel"));
}



void
MainForm::onDownloadsFinished()
{
    QList<Downloader::Status> statuses;

    foreach (Downloader *processor, _processor.getDownloaders()) {
        statuses.append(processor->getStatus());
    }

    ui->btnDownload->setText(tr("Download"));
    ui->cbxMode->setEnabled(true);

    MessageBus->send("downloading_finished");

    if (statuses.count() > 0)
    {
        if (statuses.contains(Downloader::ErrorConnection))
        {
            QString msg = tr("One or more downloads have failed due to \
                             Internet connection problems. Check your Internet \
                             connection and try again!");
            QMessageBox::warning(this, tr("Warning"), msg);
        }
        else if (statuses.contains(Downloader::ErrorIO))
        {
            QString msg = tr("One or more downloads have failed because their \
                             conversion failed.");
            QMessageBox::warning(this, tr("Warning"), msg);
        }
        else
        {
            QString msg = tr("Hooray! Your downloads have been completed.");
            QMessageBox::information(this, tr("Information"), msg);
        }
    }
}



void
MainForm::onParserFinished(QList<Download> downloads)
{
    OutputFormat format = _outputFormats.at(ui->cbxMode->currentIndex());

    int count = downloads.length();

    foreach (Download d, downloads)
    {
        // Each download has a unique signature generated by the specific video.
        // Check if this download is already registered and ignore it.

        if (_processor.hasDownload(d.signature)) {
            continue;
        }

        QList<QStandardItem *> list;
        list << new QStandardItem(d.videoTitle);
        list << new QStandardItem("");
        list << new QStandardItem("");
        list << new QStandardItem("");
        list << new QStandardItem("");

        list.at(0)->setEditable(false);
        list.at(1)->setEditable(false);
        list.at(2)->setEditable(false);
        list.at(3)->setEditable(false);
        list.at(4)->setEditable(false);

        _downloadsModel.appendRow(list);


        if (count > 1)
        {
            // The parser returned multiple downloads for one url.
            // This is a playlist. We should register each video's url so the
            // user can't reenter it for downloading.

            _registeredUrls.append(d.normalUrl);
        }


        d.outputFormat = format;

        int id = _processor.enqueue(d);
        _idToRowIndex.insert(id, _downloadsModel.rowCount() - 1);


        // Call the downloader to report its initial status and progress values.

        _processor.getDownloader(id)->report();

        ui->btnDownload->setEnabled(true);
    }

    _processor.setConcurrentDownloads(Settings->value("sim_downloads").toInt());


    // If autostart is enabled, try starting the new downloads.

    if (Settings->value("autostart") == true) {
        _processor.process();
    }


    // If there aren't parsers working at the moment, then we should update UI
    // accordingly.

    if (!_parser.parsing()) {
        MessageBus->send("parsing_finished");
    }
}



void
MainForm::applyCurrentMode()
{
    OutputFormat format = _outputFormats.at(ui->cbxMode->currentIndex());


    // Store the new output format id in order to be remembered on next launch.

    if (_loaded) {
        Settings->setValue("download_mode", format.uid);
    }


    // Apply the new output format setting to all downloaders not currently
    // downloading or converting.

    foreach (int downloadId, _idToRowIndex.keys())
    {
        // Ask the Processor for the downloader assigned to the download id.
        Downloader *downloader = _processor.getDownloader(downloadId);

        // Ready downloader's status data and act accordingly.
        Downloader::Status status = downloader->getStatus();

        if (status != Downloader::Downloading &&
            status != Downloader::Converting)
        {
            Download *download = downloader->getDownload();
            download->outputFormat = format;
        }
    }
}



void
MainForm::registerAndParseUrl(QString url)
{
    MessageBus->send("parsing_started");

    _registeredUrls.append(url);

    _parser.parse(url);
}
