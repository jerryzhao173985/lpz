/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
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
#ifndef __IMAGEPPM_H
#define __IMAGEPPM_H

#include <string>
#include <vector>
#include <memory>

class ImagePPM{

private:
  int image_width = 0;
  int image_height = 0;
  std::vector<unsigned char> image_data;

public:
  ImagePPM ();
  /// data must contain width*height*3 static_cast<RGB>(values)!
  explicit ImagePPM (int width, int height, const unsigned char* data);
  ~ImagePPM();
  int loadImage(const std::string& filename); // load from PPM file (returns 0 if error)
  int storeImage(const std::string& filename); // store to PPM file (returns 0 if error)
  [[nodiscard]] int width() const { return image_width; }
  [[nodiscard]] int height() const { return image_height; }
  unsigned char *data() { return image_data.data(); }
  const unsigned char *data() const { return image_data.data(); }

};

#endif
