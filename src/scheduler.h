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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QMap>
#include <QQueue>
#include <QProcess>

class Scheduler : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Started,
        Finished
    };

    Scheduler();
    int enqueue(QString command);
    void abort(int pid);
    bool running(int pid);
    void setConcurrentTasks(int num);

private:
    QQueue<int> _queue;
    QMap<QProcess*, int> _processes;
    QMap<int, QString> _commands;
    int _pid;
    int _num;

    void process();

signals:
    void statusChanged(Scheduler::Status status, int pid, int exitCode);

public slots:
    void onCompleted(int exitCode);
};

#endif // SCHEDULER_H
