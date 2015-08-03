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

#include "mainform.h"
#include "ui_mainform.h"
#include "global.h"
#include "parser.h"
#include "settingsform.h"
#include "aboutform.h"
#include "messenger.h"

#include <QMessageBox>


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


    // Set UI according on settings
    switch (Settings->value("download_mode").toInt())
    {
        case 0:
            ui->cbxMode->setCurrentIndex(0); // keep music
            break;
        case 1:
            ui->cbxMode->setCurrentIndex(1); // keep video and music
            break;
        default:
            ui->cbxMode->setCurrentIndex(0);
    }

    ui->btnDownload->setEnabled(false);
}





MainForm::~MainForm()
{
    foreach (Processor *processor, _processors){
        processor->deleteLater();
    }

    delete ui;
}





void
MainForm::onModeCurrentIndexChanged(int)
{
    Settings->setValue("download_mode", ui->cbxMode->currentIndex());
    applyCurrentMode();
}





void
MainForm::onClipboardChanged(QClipboard::Mode mode)
{
    if (mode == QClipboard::Clipboard)
    {
        QString url = QApplication::clipboard()->text();
        if (isValidUrl(url))
        {
            QString c = canonicalizeUrl(url);
            if (_registeredUrls.contains(c) == false) {
                registerAndParseUrl(c);
            }
        }
    }
}





void
MainForm::onStartClicked()
{
    QModelIndexList indexes = ui
                                ->tvwDownloads
                                ->selectionModel()
                                ->selection().indexes();

    for (int index = 0; index < indexes.count(); index++)
    {
        QModelIndex modelIndex = indexes.at(index);
        foreach (Processor *processor, _processors)
        {
            if (processor->getDownload()->titleItem ==
                _downloadsModel.item(modelIndex.row(), 0))
            {
                Processor::Status status = processor->getStatus();

                if (status == Processor::Complete)
                {
                    QMessageBox::information(
                        this,
                        tr("Information"),
                        tr("Hooray! This download is already complete!")
                    );
                }

                if (status != Processor::Downloading &&
                    status != Processor::Converting) {
                    processor->start();
                }
                else
                {
                    QMessageBox::information(
                        this,
                        tr("Information"),
                        tr("This download is already in progress.")
                    );
                }
                return;
            }
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



    for (int index = 0; index < indexes.count(); index++)
    {
        QModelIndex modelIndex = indexes.at(index);

        foreach (Processor *processor, _processors)
        {
            if (processor->getDownload()->titleItem ==
                _downloadsModel.item(modelIndex.row(), 0))
            {
                Processor::Status status = processor->getStatus();
                if (status == Processor::Downloading ||
                    status == Processor::Converting) {
                    processor->stop();
                }
                else
                {
                    QMessageBox::information(
                        this,
                        tr("Information"),
                        tr("This download is not currently in progress."));
                }
                return;
            }
        }
    }
}





void
MainForm::onDownloadClicked()
{
    ProcessorStats stats = getProcessorStats();

    // If downloads in progress, then the button is used as a cancel button.
    if (stats.downloading + stats.converting > 0)
    {
        foreach (Processor *processor, _processors) {
            Processor::Status status = processor->getStatus();
            if (status == Processor::Ready ||
                status == Processor::Downloading ||
                status == Processor::Converting) {
                processor->stop();
            }
        }
    }
    else
    {
        foreach (Processor *p, _processors)
        {
            Processor::Status status = p->getStatus();
            if (status != Processor::Complete && status != Processor::Ready) {
                p->reset();
            }
        }
        processDownloads();
    }

}





void
MainForm::onDeleteClicked()
{
    QModelIndexList indexes = ui
                                ->tvwDownloads
                                ->selectionModel()
                                ->selection().indexes();


    for (int index = 0; index < indexes.count(); index++)
    {
        QModelIndex modelIndex = indexes.at(index);

        foreach (Processor *processor, _processors)
        {
            Download* download = processor->getDownload();
            if (download->titleItem == _downloadsModel.item(modelIndex.row(), 0))
            {
                Processor::Status status = processor->getStatus();
                if (status != Processor::Downloading &&
                    status != Processor::Converting)
                {
                    processor->disconnect();
                    _processors.removeOne(processor);
                    _registeredUrls.removeOne(download->normalUrl);
                    _downloadsModel.removeRow(modelIndex.row());
                    processor->deleteLater();
                }
                else
                {
                    QMessageBox::information(
                        this,
                        tr("Information"),
                        tr("This download is in progress. Please stop it first.")
                    );
                }
                return;
            }
        }
    }

    if (_downloadsModel.rowCount() == 0) {
        ui->btnDownload->setEnabled(false);
    }
}





void
MainForm::onClearClicked()
{
    bool downloading = false;
    foreach (Processor *processor, _processors)
    {
        Processor::Status status = processor->getStatus();
        if (status == Processor::Downloading ||
            status == Processor::Converting)
        {
            downloading = true;
            break;
        }
    }

    if (downloading)
    {
        QMessageBox::information(
            this,
            tr("Information"),
            tr("One or more downloads are in progress. Please cancel them first.")
        );
        return;
    }

    foreach (Processor *processor, _processors)
    {
        processor->disconnect();
        processor->deleteLater();
    }

    _processors.clear();
    _registeredUrls.clear();
    _downloadsModel.removeRows(0, _downloadsModel.rowCount());

    ui->btnDownload->setEnabled(false);
}





/**
 * @brief SLOT. Runs when a processor reports a new status.
 * We take appropriate action according to reported status.
 */
void MainForm::onProcessorStatusChanged()
{
    Processor *processor = qobject_cast<Processor*>(QObject::sender());
    Processor::Status status = processor->getStatus();

    if (status == Processor::ErrorConnection ||
        status == Processor::ErrorIO ||
        status == Processor::Complete ||
        status == Processor::Converting ||
        status == Processor::Canceled)
    {
        ProcessorStats stats = getProcessorStats();
        int run = stats.canceled + stats.completed + stats.errored;

        if (run == _processors.count()) {
            doDownloadsFinished();
        } else {
            processDownloads();
        }
    }
    else
    {
        // Downloading (Ready is never signaled)
        ui->btnDownload->setText(tr("Cancel"));
    }
}





/**
 * @brief
 * SLOT. Runs when the parser has finished fetching information about a video.
 */
void
MainForm::onParserFinished(QList<Download> downloads)
{
    int count = downloads.length();
    QString path = Settings->value("download_path").toString();
    Parser *parser = qobject_cast<Parser*>(QObject::sender());


    parser->deleteLater();


    foreach (Download d, downloads)
    {
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


        /* These properties are needed in order for the processor to be able to
         * set the status fields of each download.
         */
        d.titleItem = list.at(0);
        d.statusItem = list.at(1);
        d.progressItem = list.at(2);
        d.speedItem = list.at(3);
        d.etaItem = list.at(4);

        _downloadsModel.appendRow(list);

        if (count > 1)
        {
            /* The parser returned multiple downloads for one url.
             * This is a playlist.
             * We should register the canonicalized version each video's url
             * so the user can't reenter it for downloading.
             */
            _registeredUrls.append(canonicalizeUrl(d.normalUrl));
        }


        Processor *p = new Processor(d, path);

        connect(p,
                SIGNAL(statusChanged()),
                this,
                SLOT(onProcessorStatusChanged()));

        _processors.append(p);

        ui->btnDownload->setEnabled(true);
    }


    // New processors have been added. Set their mode.
    applyCurrentMode();


    // If autostart is enabled, try starting the new downloads
    if (Settings->value("autostart") == true)
        processDownloads();


    /* Each finished parser should be removed from the list of the currently
     * active parsers and if there aren't parsers working at the moment, then
     * we should update UI accordingly.
     */
    _parsers.removeOne(parser);
    if (_parsers.count() == 0) {
        MessageBus->send("parsing_finished");
    }

}





void
MainForm::applyCurrentMode()
{
    // If index == 0, then the user want to keep the sound of the video
    // The processor will convert the downloaded sound stream to mp3.
    // Otherwise specify no conversion extension.
    // The processor will keep the downloaded video intact.
    QString ext = ui->cbxMode->currentIndex() == 0 ? "mp3" : "";
    foreach (Processor *processor, _processors) {
        processor->getDownload()->convertExtension = ext;
    }
}





bool
MainForm::isValidUrl(QString url)
{
    return url.contains("youtube.com/watch");
}





void
MainForm::doDownloadsFinished()
{
    QList<Processor::Status> statuses;

    foreach (Processor *processor, _processors) {
        statuses.append(processor->getStatus());
    }

    ui->btnDownload->setText(tr("Download"));
    ui->cbxMode->setEnabled(true);

    MessageBus->send("downloading_finished");

    if (statuses.count() > 0)
    {
        if (statuses.contains(Processor::ErrorConnection))
        {
            QString msg = tr("One or more downloads have failed due to Internet connection problems. Check your Internet connection and try again!");
            QMessageBox::warning(this, tr("Warning"), msg);
        }
        else if (statuses.contains(Processor::ErrorIO))
        {
            QString msg = tr("One or more downloads have failed because their conversion failed.");
            QMessageBox::warning(this, tr("Warning"), msg);
        }
        else
        {
            QString msg = tr("Hooray! Your downloads have been completed.");
            QMessageBox::information(this, tr("Information"), msg);
        }
    }
}





QString
MainForm::canonicalizeUrl(QString url)
{
    if (url.startsWith("https://"))
        url = url.replace("https://", "http://");

    QStringList urlParts = url.split("?");
    QString urlBasicPart = urlParts.at(0);
    QString urlQueryPart = urlParts.at(1);
    QStringList urlQueryItems = urlQueryPart.split("&");
    QString videoQueryItem = "";
    QString listQueryItem = "";

    foreach (QString i, urlQueryItems)
    {
        QStringList t = i.split("=");
        QString verb = t.at(0);
        if (verb == "v")
            videoQueryItem = i;
        else if (verb == "list")
            listQueryItem = i;
    }

    return
        urlBasicPart +
        "?" +
        videoQueryItem +
        (listQueryItem.isEmpty() ? "" : "&" + listQueryItem);
}





void
MainForm::registerAndParseUrl(QString url)
{
    MessageBus->send("parsing_started");

    Parser *parser = new Parser();
    connect(parser,
            SIGNAL(finished(QList<Download>)),
            this,
            SLOT(onParserFinished(QList<Download>)));

    _parsers.append(parser);
    _registeredUrls.append(url);

    parser->parse(url);
}





ProcessorStats
MainForm::getProcessorStats()
{
    ProcessorStats stats;
    stats.canceled = 0;
    stats.completed = 0;
    stats.converting = 0;
    stats.downloading = 0;
    stats.errored = 0;
    stats.ready = 0;

    foreach (Processor *processor, _processors)
    {
        Processor::Status status = processor->getStatus();
        switch (status)
        {
            case Processor::Ready:
                stats.ready++;
                break;
            case Processor::Downloading:
                stats.downloading++;
                break;
            case Processor::Converting:
                stats.converting++;
                break;
            case Processor::Complete:
                stats.completed++;
                break;
            case Processor::Canceled:
                stats.canceled++;
                break;
            case Processor::ErrorConnection:
            case Processor::ErrorIO:
                stats.errored++;
        }
    }

    return stats;
}





void
MainForm::processDownloads()
{
    ProcessorStats stats = getProcessorStats();
    int downloading = stats.downloading;
    int ready = stats.ready;


    // No processors currently run, now starting.
    if (ready == _processors.length()) {
        ui->cbxMode->setDisabled(true);
        MessageBus->send("downloading_started");
    }


    /* We will start a new download as soon as a processor has finished
     * downloading taking into account the maximum number of concurrent
     * downloads the user has set.
     */
    if (ready > 0)
    {
        int simDownloads = Settings->value("sim_downloads").toInt();
        int availableSlots = simDownloads - downloading;
        int newDownloadSlots = 0;

        /* It may be less than zero if the user has decreased concurrent
         * downloads while downloading has started
         */
        if (availableSlots <= 0)
            return;

        if (availableSlots >= ready) {
            newDownloadSlots = ready;
        } else {
            newDownloadSlots = availableSlots;
        }

        foreach (Processor *processor, _processors)
        {
            if (processor->getStatus() == Processor::Ready)
            {
                processor->start();
                newDownloadSlots--;
            }

            if (newDownloadSlots == 0)
                break;
        }
    }
}



