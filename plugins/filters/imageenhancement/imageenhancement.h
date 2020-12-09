/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef IMAGEENHANCEMENT_H
#define IMAGEENHANCEMENT_H

#include <QObject>
#include <QVariant>

class KritaImageEnhancement : public QObject
{
    Q_OBJECT
public:
    KritaImageEnhancement(QObject *parent, const QVariantList &);
    ~KritaImageEnhancement() override;
};

#endif
