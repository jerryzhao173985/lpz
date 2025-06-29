/***************************************************************************
 *   Copyright (C) 2010 by                                                 *
 *   Research Network for Self-Organization of Robot Behavior              *
 *    guettler@informatik.uni-leipzig.de                                   *
 *    wrabe@informatik.uni-leipzig.de                                      *
 *    Georg.Martius@mis.mpg.de                                             *
 *    ralfder@mis.mpg.de                                                   *
 *    frank@nld.ds.mpg.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *                                                                         *
 *  DESCRIPTION                                                            *
 *                                                                         *
 *   $Log$
 *   Revision 1.20  2011-03-22 16:38:13  guettler
 *   - adpaptions to enhanced configurable and inspectable interface:
 *   - qconfigurable is now restarted if initialization of agents is finished
 *
 *   Revision 1.19  2011/03/21 17:35:26  guettler
 *   - new autosave checkbox in context menu implemented and used
 *
 *   Revision 1.18  2011/02/04 13:03:16  wrabe
 *   - bugfix: Configurables are restored now when event "CommunicationStateWillChange" occurs, not in destructor
 *
 *   Revision 1.17  2011/01/28 12:15:37  guettler
 *   - restore of AutoSave File from a backup implemented
 *   - reset to original values, values AND bounds for Configurable implemented
 *   - reset to original values for tileWidgets implemented
 *
 *   Revision 1.16  2011/01/27 09:04:12  guettler
 *   - some preparations for checkbox in order to switch the autosave function
 *
 *   Revision 1.15  2011/01/24 18:40:48  guettler
 *   - autosave functionality now stores only values, bounds and descriptions of
 *   parameters if they differ from their original values
 *
 *   Revision 1.14  2010/12/16 18:37:40  wrabe
 *   -added several tooltips
 *   -corrected sentences, notation, syntax for improved informational value
 *   -bugfix: if in collapsed mode, all tiles were stored as invisible
 *   -cosmetic ui changes
 *   -other minor things
 *
 *   Revision 1.13  2010/12/16 16:39:25  wrabe
 *   - drag&drop reworked: user can now drag a parameter to a any place
 *   - rearrangement of parameters now made only when user wants this
 *   - bugfixes
 *
 *   Revision 1.12  2010/12/15 18:28:34  wrabe
 *   -preparations for drag&drop of tileWidgets to empty places
 *
 *   Revision 1.11  2010/12/15 18:06:55  wrabe
 *   -regression fix: drag and drop of tileWidgets
 *
 *   Revision 1.10  2010/12/15 17:26:28  wrabe
 *   - number of colums for tileWidgets and width of tileWidgets can
 *   now be changed (independently for each Configurable)
 *   - bugfixes
 *
 *   Revision 1.9  2010/12/15 11:00:06  wrabe
 *   -load/save multiple ConfigurableStates from one file
 *   -All current ConfigurableStates can be stored and loaded now via menu
 *   -loading a ConfigurableState for one Configurable from a file containing multiple ConfigurableStates allows to choose one desired ConfigurableState
 *
 *   Revision 1.8  2010/12/14 10:10:12  guettler
 *   -autoload/autosave now uses only one xml file
 *   -fixed getName of TileWidget which produced invisible widgets in xml files
 *
 *   Revision 1.7  2010/12/13 16:22:18  wrabe
 *   - autosave function rearranged
 *   - bugfixes
 *
 *   Revision 1.6  2010/12/09 17:00:08  wrabe
 *   - load / save function of ConfigurableState (configurable + GUI)
 *   - autoload / autosave function of ConfigurableState (configurable
 *     + GUI)
 *   - handling of equal Configurable names implemented for autoload
 *     and -save
 *   - bugfixing
 *
 *   Revision 1.5  2010/12/08 17:52:57  wrabe
 *   - bugfixing/introducing new feature:
 *   - folding of the ConfigurableWidgets now awailable
 *   - highlight the ConfigurableTile when hoovered by mouse
 *   - load/store of the state of a ConfigurableWidget to file
 *
 *   Revision 1.4  2010/12/03 11:11:53  wrabe
 *   - now handled paramVal, paramInt and paramBool, all the params are displayed
 *     as ConfigurableTiles witch can be show and hide seperatly or arranged by user
 *     (showHideDialog reacheble by contextMenu (right click an the Widget containing
 *     the tiles ), arrange the Tiles is can done by drag and drop (there is no history or
 *     storage implementet yet))
 *
 *   Revision 1.3  2010/11/30 17:07:06  wrabe
 *   - new class QConfigurableTileShowHideDialog
 *   - try to introduce user-arrangeable QConfigurationTiles (current work, not finished)
 *
 *   Revision 1.2  2010/11/28 20:33:44  wrabe
 *   - current state of work: only paramval´s
 *   - construct a configurable as a tile containing a QSlider to change the value by drag with mouse as well as a QSpinBox to change the configurable by typing new values (mouse-scrolls are also supported)
 *   - minimum and maximum boundaries can´t be changed will be so far, only a change- dialog-dummy is reacable over the context-menu
 *
 *   Revision 1.1  2010/11/26 12:22:36  guettler
 *   - Configurable interface now allows to set bounds of paramval and paramint
 *     * setting bounds for paramval and paramint is highly recommended (for QConfigurable(const Qt& GUI).
 *   - bugfixes
 *   - current development state of QConfigurable(const Qt& GUI)
 *
 *                                                                         *
 ***************************************************************************/

#ifndef __QCONFIGURABLEWIDGET_H_
#define __QCONFIGURABLEWIDGET_H_

#include "selforg/configurable.h"
#include "selforg/callbackable.h"
#include "QAbstractConfigurableTileWidget.h"
#include <QGroupBox>
#include <QFrame>
#include <QScrollBar>
#include <QMap>
#include <QMenu>
#include <QPalette>
#include <QDomElement>
#include "QDummyConfigurableTileWidget.h"
#include "QGridPos.h"


namespace lpzrobots {



  class QConfigurableWidget : public QGroupBox, public Callbackable {

    Q_OBJECT

    public:
      QConfigurableWidget(Configurable* config, int nameIndex);
      virtual ~QConfigurableWidget();
      QDomElement toXml(bool insertDefaultConfigurableValues, bool inAutoSaveMode);
      int fromXml(const QDomElement &qde_configurableState, bool inAutoSaveMode);
      int getNameIndex() {return nameIndex; }
      Configurable* getConfigurable() const { return config; }
      QString getName() const { return configName; }

      virtual void doOnCallBack(BackCaller* source, BackCaller::CallbackableType type = BackCaller::DEFAULT_CALLBACKABLE_TYPE);

      void lampyris_noctiluca();

    public slots:
      void sl_mousePressEvent(QMouseEvent* event);
      void sl_resetToOriginalValuesAndBounds();




    signals:
      void sig_tileWidgetResize(const QSize& newSize);
      void sig_configurableChanged(QConfigurableWidget* sourceWidget);


    protected:
      virtual void enterEvent(QEvent * event);
      virtual void leaveEvent(QEvent * event);
      virtual void mousePressEvent(QMouseEvent * event);
      virtual void mouseDoubleClickEvent(QMouseEvent * event);
      virtual void dragEnterEvent(QDragEnterEvent *event);
      virtual void dragMoveEvent(QDragMoveEvent *event);
      virtual void dropEvent(QDropEvent *event);
      virtual void dragLeaveEvent(QDragLeaveEvent *event);

    private slots:
      void sl_execContextMenu(const QPoint &pos);
      void sl_changeNumberTileColumns();
      void sl_showAndHideParameters();
      void sl_loadConfigurableStateFromFile();
      void sl_saveConfigurableStateToFile();
      void sl_rearrangeConfigurableTiles();
      void sl_toggled(bool on);
      void sl_resetToOriginalValues();

    private:
      void setFolding(bool folding);
      int loadConfigurableState(const QString &fileName);
      bool saveConfigurableState(const QString &fileName);
      void createConfigurableLines();
      void initBody();
      void arrangeConfigurableTiles();
      void setToolTip();


      QMenu contextMenuShowHideDialog;
      QAction* actionToggleAutoSave;
      QGridLayout layout;
      QPalette defaultPalette;
      Configurable* config;
      QMap<QString, QAbstractConfigurableTileWidget*> configTileWidgetMap;
      QMap<QGridPos, QAbstractConfigurableTileWidget*> tileIndexConfigWidgetMap;
      QMap<QGridPos, QAbstractConfigurableTileWidget*> configTiles_shownBeforeCollapse;
      QList<QDummyConfigurableTileWidget*> dummyConfigTileList;


      bool dragging;
      bool isCollapsed;
      QPoint lastMousePos;
      QPoint configurableTile_mousePressedOffset;
      QAbstractConfigurableTileWidget* configurableTile_dragging;

      QString log;
      QString configName;
      int nameIndex;
      int numberOfTilesPerRow;
      int numberOfVisibleTiles;

  };

}

#endif /* __QCONFIGURABLEWIDGET_H_ */
