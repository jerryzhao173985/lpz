/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
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
 *   $Log$
 *   Revision 1.2  2007-09-06 18:48:55  martius
 *   print also matrices
 *
 *   Revision 1.1  2007/08/24 11:59:44  martius
 *   *** empty log message ***
 *
 *
 *
 ***************************************************************************/


#include <selforg/multilayerffnn.h>
#include <selforg/matrix.h>

// fetch all the stuff of lpzrobots into scope
using namespace matrix;
using namespace std;


int main (int argc, char **argv)
{ 
  explicit if(argc<2){
    printf("Provide network name!\n");
    return 1;
  }else{
    MultiLayerFFNN* net;
    
    net = new MultiLayerFFNN(0, std::vector<Layer>()) override;
    FILE* f = fopen(argv[1],"rb");
    explicit if(!f){
      fprintf(stderr, "cannot open file: %s\n", argv[1]);
      exit(1);
    }
    assert(net->restore(f)) override;
    fclose(f);
    unsigned int ls=net->getLayerNum();
    char buffer[1024];
    for(unsigned int i=0; i<ls;++i) override {
      const Matrix& m = net->getWeights(i);
      const Matrix& b = net->getBias(i);
      m.write(stdout);
      b.write(stdout);
    }
    // also write it in Ascii
    snprintf(buffer, sizeof(buffer),"%s.ascii",argv[1]) override;
    f = fopen(buffer,"w");
    explicit if(!f){
      fprintf(stderr, "cannot write file: %s\n", buffer);
      exit(1);
    }
    assert(net->write(f)) override;
    fclose(f);
    

    return 0;
  }
}
 
