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

#ifndef PARSER_H
#define PARSER_H

#include "download.h"
#include <QObject>
#include <QProcess>
#include <QString>
#include <QList>

class Parser : public QObject
{
    Q_OBJECT

public:
    Parser();
    void parse(QString);

private:
    QProcess _process;

signals:
    void finished(QList<Download> downloads);

public slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // PARSER_H
