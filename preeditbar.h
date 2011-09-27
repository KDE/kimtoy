/*
 *  This file is part of KIMToy, an input method frontend for KDE
 *  Copyright (C) 2011 Ni Hui <shuizhuyuanluo@126.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREEDITBAR_H
#define PREEDITBAR_H

#include <QWidget>

// class Themer;
class ThemerFcitx;
class ThemerNone;
class ThemerPlasma;
class ThemerSogou;

class PreEditBar : public QWidget
{
    Q_OBJECT
    public:
        explicit PreEditBar();
        virtual ~PreEditBar();
    protected:
        virtual bool eventFilter( QObject* object, QEvent* event );
        virtual void resizeEvent( QResizeEvent* event );
        virtual void paintEvent( QPaintEvent* event );
    private Q_SLOTS:
        void slotUpdateSpotLocation( int x, int y );
        void slotShowPreedit( bool show );
        void slotShowAux( bool show );
        void slotShowLookupTable( bool show );
        void slotUpdatePreeditCaret( int pos );
        void slotUpdatePreeditText( const QString& text,
                                    const QString& attrs );
        void slotUpdateAux( const QString& text,
                            const QString& attrs );
        void slotUpdateLookupTable( const QStringList& labels,
                                    const QStringList& candidates,
                                    const QStringList& attrs,
                                    bool hasPrev,
                                    bool hasNext );
    private:
        void updateVisible();
        void updateSize();
    private:
        QPoint m_pointPos;
        bool m_moving;

        bool preeditVisible;
        bool auxVisible;
        bool lookuptableVisible;

//         friend class Themer;
        friend class ThemerFcitx;
        friend class ThemerNone;
        friend class ThemerPlasma;
        friend class ThemerSogou;
        QString m_text;
        int m_cursorPos;
        QString m_auxText;
        QStringList m_labels;
        QStringList m_candidates;
        bool m_hasPrev;
        bool m_hasNext;
};

#endif // PREEDITBAR_H
