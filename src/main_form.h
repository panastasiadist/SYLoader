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
#ifndef MAINFORM_H
#define MAINFORM_H



#include <QWidget>
#include <QList>
#include <QClipboard>
#include <QMap>
#include "core/output_format.h"
#include "core/download.h"
#include "core/downloader.h"
#include "core/downloader_stats.h"
#include "core/downloader_progress.h"
#include "core/url_processor.h"
#include "core/queue_processor.h"
#include "progress_item_delegate.h"



namespace Ui {
class MainForm;
}

class MainForm : public QWidget
{
    Q_OBJECT

public:
    explicit MainForm(QWidget *parent = 0);
    ~MainForm();


private:
    Ui::MainForm *ui;
    ProgressItemDelegate _progressItemDelegate;
    QStandardItemModel _downloadsModel;
    QList<QString> _registeredUrls;
    UrlProcessor _parser;
    QueueProcessor _processor;
    QMap<int, int> _idToRowIndex;
    QList<OutputFormat> _outputFormats;
    bool _loaded = false;

    DownloaderStats getProcessorStats();
    void registerAndParseUrl(QString url);
    void applyCurrentMode();
    void processDownloads();


private slots:
    void onParserFinished(QList<Download> downloads);
    void onDownloadStatusChanged(int id, Downloader::Status status);
    void onDownloadProgressChanged(int id, DownloaderProgress progress);
    void onDownloadsStarted();
    void onDownloadsFinished();
    void onDownloadClicked();
    void onStartClicked();
    void onStopClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onDownloadDirectoryClicked();
    void onDownloadListDoubleClicked(QModelIndex);
    void onModeCurrentIndexChanged(int);
    void onClipboardChanged(QClipboard::Mode mode);
};



#endif // MAINFORM_H
