/***************************************************************************
 *   Copyright (C) 2008-2011 LpzRobots development team                    *
 *    Antonia Siegert (original author)                                  *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
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
 *                                                                         *
 ***************************************************************************/
#include "MatrixVisualizer.h"

#include <QString>
#include <iostream>

using namespace std;


MatrixVisualizer::MatrixVisualizer(QWidget *parent, bool noVideo)
  : AbstractRobotGUI(parent) {

  pipe_reader = new SimplePipeReader(noVideo);
  pipe_reader->waitUntilGo();
//  vector_filter = new VectorPipeFilter(pipe_reader);
  matrix_filter = new MatrixPipeFilter(pipe_reader);

  // if '#QUIT' matrixvis quits too
  SimplePipeReader* simple_reader = dynamic_cast<SimplePipeReader*>(pipe_reader);
  if (simple_reader) {
    connect(simple_reader, SIGNAL(finished()), this, SLOT(close()));
    connect(simple_reader, SIGNAL(sourceName(QString)), this, SLOT(sourceName(QString)));
    connect(simple_reader, SIGNAL(captureFrame(long,QString)), this, SLOT(captureFrame(long,QString)));
  }
  // let pipe reader start
  if (simple_reader) {
    simple_reader->start();
  }
  if (debug) cout << "Here I AM!!!" << endl;

  channelList = matrix_filter->getChannelList();
  matrices = matrix_filter->getMatrixChannels();
  vectors = matrix_filter->getVectorChannels();

  initGui();
  config = new configFile();
  config->load(this);
  connect(this, SIGNAL(sendQuit()), config, SLOT(doQuit()));
  pipe_reader->goReadData();
}

MatrixVisualizer::~MatrixVisualizer() {
}

void MatrixVisualizer::initGui() {

  main_layout = new QVBoxLayout;
  nameLabel = new QLabel();
  main_layout->addWidget(nameLabel);
  main_layout->addLayout(makeButtons());
  setLayout(main_layout);
  // nach pack() ähnlichem gucken!
  resize(150,150); //adjustSize ();
  show();
}

void MatrixVisualizer::sourceName(const QString& name){
  srcName=name;
  QList<VisualiserSubWidget*> openWindows=config->getOpenWindows();
  for (QList<VisualiserSubWidget*>::iterator it = openWindows.begin(); it != openWindows.end(); ++it) {
    if(*it) (*it)->sourceName(srcName);
  }
  if(nameLabel) nameLabel->setText(srcName);
}

QHBoxLayout* MatrixVisualizer::makeButtons(){

  QHBoxLayout *chooseBoxL = new QHBoxLayout;
  QVBoxLayout *matrBox = new QVBoxLayout;
  QVBoxLayout *vecBox = new QVBoxLayout;

  visButtons = new QButtonGroup();
  visButtons->setExclusive(false);

  int id = 0;
  for(vector<MatrixPlotChannel*>::iterator i = matrices.begin(); i != matrices.end(); ++i){
    QString qs( (*i)->getChannelName().c_str() );
    QPushButton *pB = new QPushButton(qs);
    matrBox->addWidget(pB);
    visButtons->addButton(pB);
    ++id;
  }
  if(debug) cout << "size of Vectors: " << vectors.size() << endl;
  for(vector<VectorPlotChannel*>::iterator i = vectors.begin(); i != vectors.end(); ++i){
    if(debug) cout << "addButton for vector" << endl;
      QString qs( (*i)->getChannelName().c_str() );
      QPushButton *pB = new QPushButton(qs);
      vecBox->addWidget(pB);
      visButtons->addButton(pB);
      ++id;
    }

  connect(visButtons, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(visualize(QAbstractButton *)));

  chooseBoxL->addLayout(matrBox);
  chooseBoxL->addLayout(vecBox);

  return chooseBoxL;
}

void MatrixVisualizer::visualize(QAbstractButton * button){
  // contains the button to a matrix or a vector
  QString name = button->text();
  // Try to get as MatrixPlotChannel first, then VectorPlotChannel
  MatrixPlotChannel *matrixPlotChannel = getMatrixPlotChannel(name);
  if(matrixPlotChannel == nullptr){
    // VectorPlotChannel cannot be used with VisualiserSubWidget directly
    return; // Skip visualization for VectorPlotChannel
  }
  VisualiserSubWidget *vis = new VisualiserSubWidget(matrixPlotChannel);
  config->newOpenedWindow(vis);
  vis->show();
  connectWindowForUpdate(vis);
}

void MatrixVisualizer::linkChannels() { //not needed
//  for (std::list<AbstractPlotChannel*>::iterator i=channelList.begin(); i!=channelList.end(); ++i){
//    //widget->addPlotChannel(i*)
//  }

}

void MatrixVisualizer::captureFrame(long idx, QString directory){
  QList<VisualiserSubWidget*> openWindows=config->getOpenWindows();
  for (QList<VisualiserSubWidget*>::iterator it = openWindows.begin(); it != openWindows.end(); ++it) {
    if(*it) (*it)->captureFrame(idx, directory);
  }
  pipe_reader->goReadData();
}

void MatrixVisualizer::connectWindowForUpdate(VisualiserSubWidget *vis){
  SimplePipeReader* simple_reader2 = dynamic_cast<SimplePipeReader*>(pipe_reader);
  if (simple_reader2) {
    connect(simple_reader2, SIGNAL(newData()), vis, SLOT(updateViewableChannels())/*, Qt::DirectConnection*/);
    connect(simple_reader2, SIGNAL(sourceName(QString)), vis, SLOT(sourceName(QString)));
  }
  vis->sourceName(srcName);
}

VectorPlotChannel* MatrixVisualizer::getVectorPlotChannel(QString name){
  for(vector<VectorPlotChannel*>::iterator i = vectors.begin(); i != vectors.end(); ++i){
      QString vecName( (*i)->getChannelName().c_str() );
      if(vecName == name) return (*i);
    }
  return 0;
}

MatrixPlotChannel* MatrixVisualizer::getMatrixPlotChannel(QString name){
  for(vector<MatrixPlotChannel*>::iterator i = matrices.begin(); i != matrices.end(); ++i){
      QString matName( (*i)->getChannelName().c_str() );
      if(matName == name) return (*i);
    }
  return 0;
}

void MatrixVisualizer::closeEvent(QCloseEvent * event){
  if(debug) cout << "MatrixViz::closeEvent" << endl;
  emit sendQuit();
  event->accept();
}
