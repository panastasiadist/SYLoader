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

#include "progressitemdelegate.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QStyleOptionProgressBar>
#include <QString>
#include <QVariant>
#include <QLocale>

ProgressItemDelegate::ProgressItemDelegate(QObject *object)
    : QStyledItemDelegate(object) {}

void
ProgressItemDelegate::paint(
    QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index ) const
{
    if (index.column() != 2)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    else
    {
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        int progress = index.model()
                ->data(index.model()->index(index.row(), 2)).toInt();

        progressBarOption.progress = progress < 0 ? 0 : progress;

        progressBarOption.text = QString().sprintf(
                    "%d%%",
                    progressBarOption.progress);

        QApplication::style()->drawControl(
                    QStyle::CE_ProgressBar,
                    &progressBarOption,
                    painter);
    }
}
