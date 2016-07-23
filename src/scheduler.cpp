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

#include "scheduler.h"





Scheduler::Scheduler()
{
    _pid = 0;
    _num = 0;
}





int
Scheduler::enqueue(QString command)
{
    _pid++;
    _queue.append(_pid);
    _commands.insert(_pid, command);

    process();

    return _pid;
}



void
Scheduler::abort(int pid)
{
    _queue.removeOne(pid);

    QProcess *p = _processes.key(pid, NULL);
    if (p != NULL)
    {
        // It will raise finished event
        p->kill();
    }
    else
    {
        // The process hasn't run. We will emit the event.
        _commands.remove(pid);
        emit statusChanged(Finished, pid, 0);
    }
}



bool
Scheduler::running(int pid)
{
    /* When the queue contains a pid, then the relevant process
     * hasn't yet started.
     * In addition, when a process finishes, its pid is removed
     * from the queue
     */
    return !_queue.contains(pid);
}



void
Scheduler::setConcurrentTasks(int num)
{
    _num = num;
}



void
Scheduler::onCompleted(int exitCode)
{
    QProcess *p = qobject_cast<QProcess*>(QObject::sender());
    int pid = _processes.value(p);
    _processes.remove(p);
    _commands.remove(pid);
    p->deleteLater();
    emit statusChanged(Finished, pid, exitCode);

    process();
}




void
Scheduler::process()
{
    if (_queue.count() == 0)
        return;

    // Current number of running processes
    int current = _processes.count();
    int remaining = _num - current;


    for (int i = 0; i < remaining; i++)
    {
        int pid = _queue.dequeue();
        QString command = _commands.value(pid);
        QProcess *process = new QProcess();

        _processes.insert(process, pid);

        connect (process,
                 SIGNAL(finished(int)),
                 this,
                 SLOT(onCompleted(int)));

        process->start(command);

        emit statusChanged(Started, pid, 0);
    }

}

