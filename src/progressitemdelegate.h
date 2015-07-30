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

#ifndef PROGRESSITEMDELEGATE_H
#define PROGRESSITEMDELEGATE_H

#include <QObject>
#include <QtWidgets/QStyledItemDelegate>

class ProgressItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ProgressItemDelegate(QObject *object = 0);

    virtual void paint(
        QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index
    ) const;
};

#endif // PROGRESSITEMDELEGATE_H
