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
 * along with SYLoader.  If not, see http://www.gnu.org/licenses.
 ******************************************************************************/

#ifndef MAINFORM_H
#define MAINFORM_H

#include "parser.h"
#include "download.h"
#include "processor.h"
#include "progressitemdelegate.h"
#include "processorstats.h"
#include <QWidget>
#include <QList>
#include <QClipboard>



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
    Parser _parser;
    ProgressItemDelegate _progressItemDelegate;
    QStandardItemModel _downloadsModel;
    QList<Processor*> _processors;
    QList<Parser*> _parsers;
    QList<QString> _registeredUrls;



    QString canonicalizeUrl(QString url);
    void registerAndParseUrl(QString url);
    void doDownloadsFinished();
    void applyCurrentMode();
    bool isValidUrl(QString url);
    void processDownloads();
    ProcessorStats getProcessorStats();




private slots:
    void onParserFinished(QList<Download> downloads);
    void onProcessorStatusChanged();
    void onDownloadClicked();
    void onStartClicked();
    void onStopClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onModeCurrentIndexChanged(int);
    void onClipboardChanged(QClipboard::Mode mode);
};

#endif // MAINFORM_H
