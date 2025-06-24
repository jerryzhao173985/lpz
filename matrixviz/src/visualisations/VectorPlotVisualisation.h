/*
 * VectorPlotVisualisation.h
 *
 *  Created on: 29.04.2010
 *      Author: oni
 */

#ifndef VECTORPLOTVISUALISATION_H_
#define VECTORPLOTVISUALISATION_H_
#include "AbstractVisualisation.h"

class VectorPlotVisualisation{
  Q_OBJECT

public:
  VectorPlotVisualisation(MatrixPlotChannel *channel, ColorPalette *colorPalette, QWidget *parent = 0);
//  VectorPlotVisualisation(VectorPlotChannel *channel, ColorPalette *colorPalette, QWidget *parent = 0);
  virtual ~VectorPlotVisualisation() override;
  //void updateView();


protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void mouseMoveEvent ( QMouseEvent *event ) override;


private:
  int maxX, maxY;
  const static bool debug = false;

};

#endif /* VECTORPLOTVISUALISATION_H_ */
