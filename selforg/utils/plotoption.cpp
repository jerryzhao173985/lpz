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

#include "plotoption.h"
#include <assert.h>
#include <iostream>
#include <quickmp.h>
#include <selforg/inspectable.h>
#include <selforg/stl_adds.h>
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>

using namespace std;

bool
PlotOption::open() {
  QMP_CRITICAL(601);
  char cmd[255];
  bool returnCode = true;
  std::cout << "open a stream " << std::endl;
  switch (mode) {
    case PlotMode::File:
      struct tm* t;
      time_t tnow;
      time(&tnow);
      t = localtime(&tnow);
      char logfilename[255];
      if (!parameter.empty()) {
        snprintf(logfilename, sizeof(logfilename), "%s%s.log", parameter.c_str(), name.c_str());
      } else {
        snprintf(logfilename,
                 sizeof(logfilename),
                 "%s_%02i-%02i-%02i_%02i-%02i-%02i.log",
                 name.c_str(),
                 t->tm_year % 100,
                 t->tm_mon + 1,
                 t->tm_mday,
                 t->tm_hour,
                 t->tm_min,
                 t->tm_sec);
      }
      pipe = fopen(logfilename, "w");
      if (pipe)
        std::cout << "Now logging to file \"" << logfilename << "\"." << std::endl;
      break;
    case PlotMode::GuiLogger_File:
      pipe = popen("guilogger -m pipe -l", "w");
      break;
    case PlotMode::GuiLogger:
      snprintf(cmd, sizeof(cmd), "guilogger -m pipe %s", parameter.c_str());
      pipe = popen(cmd, "w");
      break;
    case PlotMode::MatrixViz: {
      // Build command with parameters if provided
      if (!parameter.empty()) {
        snprintf(cmd, sizeof(cmd), "matrixviz -noCtrlC -novideo %s", parameter.c_str());
      } else {
        snprintf(cmd, sizeof(cmd), "matrixviz -noCtrlC -novideo");
      }
      std::cout << "PlotOption: Attempting to launch MatrixViz with command: " << cmd << std::endl;
      std::cout << "PlotOption: Parameter passed: '" << parameter << "'" << std::endl;

      pipe = popen(cmd, "w");

      if (!pipe) {
        std::cerr << "PlotOption: Initial MatrixViz launch failed, trying alternatives..."
                  << std::endl;

        // Try with LPZROBOTS_HOME environment variable
        const char* lpzrobots_home = getenv("LPZROBOTS_HOME");
        if (lpzrobots_home) {
          std::cerr << "PlotOption: Using LPZROBOTS_HOME=" << lpzrobots_home << std::endl;
        }

        if (lpzrobots_home) {
          // Try pipe-friendly wrapper first
          if (!parameter.empty()) {
            snprintf(cmd,
                     sizeof(cmd),
                     "%s/matrixviz/matrixviz-pipe -noCtrlC -novideo %s",
                     lpzrobots_home,
                     parameter.c_str());
          } else {
            snprintf(
              cmd, sizeof(cmd), "%s/matrixviz/matrixviz-pipe -noCtrlC -novideo", lpzrobots_home);
          }
          std::cerr << "PlotOption: Trying pipe-friendly wrapper: " << cmd << std::endl;
          pipe = popen(cmd, "w");

          if (!pipe) {
            // Try regular wrapper with full path
            if (!parameter.empty()) {
              snprintf(cmd,
                       sizeof(cmd),
                       "%s/matrixviz/matrixviz -noCtrlC -novideo %s",
                       lpzrobots_home,
                       parameter.c_str());
            } else {
              snprintf(
                cmd, sizeof(cmd), "%s/matrixviz/matrixviz -noCtrlC -novideo", lpzrobots_home);
            }
            std::cerr << "PlotOption: Trying regular wrapper: " << cmd << std::endl;
            pipe = popen(cmd, "w");

            if (!pipe) {
              // Also try guilogger directory
              if (!parameter.empty()) {
                snprintf(cmd,
                         sizeof(cmd),
                         "%s/guilogger/matrixviz -noCtrlC -novideo %s",
                         lpzrobots_home,
                         parameter.c_str());
              } else {
                snprintf(
                  cmd, sizeof(cmd), "%s/guilogger/matrixviz -noCtrlC -novideo", lpzrobots_home);
              }
              std::cerr << "PlotOption: Trying guilogger directory: " << cmd << std::endl;
              pipe = popen(cmd, "w");
            }
          }
        }
      }

      if (pipe) {
        std::cout << "PlotOption: MatrixViz stream opened successfully" << std::endl;
      } else {
        std::cerr << "PlotOption: MatrixViz stream open failed" << std::endl;
      }
      break;
    }
    case PlotMode::ECBRobotGUI:
      pipe = popen("SphericalRobotGUI", "w");
      if (pipe)
        std::cout << "open a SphericalRobotGUI-Stream " << std::endl;
      else
        std::cout << "can't open SphericalRobotGUI-Stream " << std::endl;
      break;
    case PlotMode::SoundMan:
      snprintf(cmd, sizeof(cmd), "soundMan %s", parameter.c_str());
      pipe = popen(cmd, "w");
      break;
    default: // and PlotMode::NoPlot
      returnCode = false;
  }
  if (pipe == 0) {
    fprintf(stderr, "%s:%i: could not open plot tool!\n", __FILE__, __LINE__);
    returnCode = false;
  }
  QMP_END_CRITICAL(601);
  return returnCode;
}

void
PlotOption::close() {
  QMP_CRITICAL(602);
  if (pipe) {

    switch (mode) {
      case PlotMode::File:
        std::cout << "logfile closing...SUCCESSFUL" << std::endl;
        fclose(pipe);
        break;
      case PlotMode::GuiLogger:
      case PlotMode::GuiLogger_File:
        // std::cout << __PLACEHOLDER_42__
        //           << std::endl;
        //  send quit message to pipe
        fprintf(pipe, "#QUIT\n");
        fflush(pipe);
        pclose(pipe);
        std::cout << "guilogger pipe closing...SUCCESSFUL" << std::endl;
        break;
      case PlotMode::MatrixViz:
        //       std::cout << __PLACEHOLDER_45__;
        fprintf(pipe, "#QUIT\n");
        pclose(pipe);
        std::cout << "MatixViz pipe closing...SUCCESSFUL" << std::endl;
        break;
      case PlotMode::ECBRobotGUI:
        //       std::cout << __PLACEHOLDER_48__;
        fprintf(pipe, "#QUIT\n");
        pclose(pipe);
        std::cout << "ECBRobotGUI pipe closing...SUCCESSFUL" << std::endl;
        break;
      case PlotMode::SoundMan:
        std::cout << "SoundMan closing...SUCCESSFUL" << std::endl;
        fclose(pipe);
        break;

      default:
        break;
    }
    pipe = 0;
  }
  QMP_END_CRITICAL(602);
}

// flushes pipe (depending on mode)
void
PlotOption::flush(long step) {
  QMP_CRITICAL(603);
  if (pipe) {
    switch (mode) {
      case PlotMode::File:
        if ((step % (interval * 1000)) == 0)
          fflush(pipe);
        break;
      case PlotMode::GuiLogger:
      case PlotMode::GuiLogger_File:
      case PlotMode::MatrixViz:
      case PlotMode::ECBRobotGUI:
      case PlotMode::SoundMan: {
        int ttt = fflush(pipe);
        if (ttt != 0) {
          printf("Pipe broken: %s\n", strerror(ttt));
          close();
        }
        break;
      }
      default:
        break;
    }
  }
  QMP_END_CRITICAL(603);
}

void
PlotOption::setFilter(const list<string>& accept, const list<string>& ignore) {
  this->accept = accept;
  this->ignore = ignore;
}

/// sets a filter to this plotoption: syntax: +acceptregexp -ignoreregexp ...
void
PlotOption::setFilter(const std::string& filter) {
  //  cout << filter << endl;
  istringstream iss(filter);
  do {
    string substr;
    iss >> substr;
    if (!substr.empty()) {
      bool flag = !(substr[0] == '-');
      if (substr[0] == '+' || substr[0] == '-')
        substr = substr.substr(1);
      if (!substr.empty()) {
        if (flag) {
          accept += substr;
          //          cout << __PLACEHOLDER_53__ << substr << endl;
        } else {
          ignore += substr;
          //          cout << __PLACEHOLDER_54__ << substr << endl;
        }
      }
    }
  } while (iss);
}

bool
PlotOption::useChannel(const string& name) {
  bool rv = accept.empty() ? true : false;
  for (auto& r : accept) {
    if (name.find(r) == 0) {
      rv = true;
      break;
    }
  }
  for (auto& r : ignore) {
    if (name.find(r) == 0) {
      rv = false;
    }
  }
  return rv;
}

int
PlotOption::printInspectableNames(const list<const Inspectable*>& inspectables, int cnt) {
  if (!pipe)
    return cnt;
  // here we also set the mask array
  FOREACHC(list<const Inspectable*>, inspectables, insp) {
    if (*insp) {
      // then the internal parameters
      list<Inspectable::iparamkey> l = (*insp)->getInternalParamNames();
      for (list<Inspectable::iparamkey>::iterator i = l.begin(); i != l.end(); ++i) {
        const string& str = (*i);
        if (static_cast<int>(mask.size()) <= cnt) {
          mask.resize(cnt * 2);
        }
        if (useChannel(str)) {
          fprintf(pipe, " %s", str.c_str());
          mask[cnt] = true;
        } else
          mask[cnt] = false;
        ++cnt;
      }
      cnt += printInspectableNames((*insp)->getInspectables(), cnt);
    }
  }
  return cnt;
}

int
PlotOption::printInspectables(const std::list<const Inspectable*>& inspectables, int cnt) {
  if (!pipe)
    return cnt;

  // internal parameters ( we allocate one place more to be able to realise when the number raises)
  Inspectable::iparamvallist l;
  FOREACHC(list<const Inspectable*>, inspectables, insp) {
    if (*insp) {
      l = (*insp)->getInternalParams();
      FOREACHC(Inspectable::iparamvallist, l, i) {
        if (cnt >= static_cast<int>(mask.size()) || cnt < 0) {
          fprintf(
            stderr, "PlotOption: mask to short: %zu <= %i", mask.size(), cnt); // should not happen
        } else {
          if (mask[cnt])
            fprintf(pipe, " %f", (*i));
        }
        ++cnt;
      }
      cnt += printInspectables((*insp)->getInspectables(), cnt);
    }
  }
  return cnt;
}

void
PlotOption::printInspectableInfoLines(const list<const Inspectable*>& inspectables) {
  if (!pipe)
    return;
  FOREACHC(list<const Inspectable*>, inspectables, insp) {
    const list<string>& infoLines = (*insp)->getInfoLines();
    FOREACHC(list<string>, infoLines, infoLine) {
      fprintf(pipe, "#I [%s] %s\n", (*insp)->getNameOfInspectable().c_str(), (*infoLine).c_str());
    }
    printInspectableInfoLines((*insp)->getInspectables());
  }
}

void
PlotOption::printNetworkDescription(const string& name, const Inspectable* inspectable) {
  assert(inspectable);
  if (!pipe)
    return;
  fprintf(pipe, "#N neural_net %s\n", name.c_str());
  list<Inspectable::ILayer> layers = inspectable->getStructuralLayers();
  list<Inspectable::IConnection> conns = inspectable->getStructuralConnections();
  // print layers with neurons
  for (list<Inspectable::ILayer>::iterator i = layers.begin(); i != layers.end(); ++i) {
    Inspectable::ILayer& l = (*i);
    fprintf(pipe, "#N layer %s %i\n", l.layername.c_str(), l.rank);
    for (int n = 0; n < l.dimension; ++n) {
      if (l.biasname.empty()) {
        fprintf(pipe, "#N neuron %s[%i]\n", l.vectorname.c_str(), n);
      } else {
        fprintf(pipe, "#N neuron %s[%i] %s[%i]\n", l.vectorname.c_str(), n, l.biasname.c_str(), n);
      }
    }
  }

  // print connections
  for (list<Inspectable::IConnection>::iterator i = conns.begin(); i != conns.end(); ++i) {
    Inspectable::IConnection& c = (*i);
    // find the layers refered in the connection description
    list<Inspectable::ILayer>::iterator l1it =
      find_if(layers.begin(), layers.end(), Inspectable::matchName(c.vector1));
    list<Inspectable::ILayer>::iterator l2it =
      find_if(layers.begin(), layers.end(), Inspectable::matchName(c.vector2));
    assert(l1it != layers.end()); // we need to find them otherwise
    assert(l2it != layers.end());

    Inspectable::ILayer& l1 = (*l1it);
    Inspectable::ILayer& l2 = (*l2it);
    for (int j = 0; j < l1.dimension; ++j) {
      for (int k = 0; k < l2.dimension; ++k) {
        fprintf(pipe,
                "#N connection %s[%i,%i] %s[%i] %s[%i]\n",
                c.matrixname.c_str(),
                k,
                j,
                l1.vectorname.c_str(),
                j,
                l2.vectorname.c_str(),
                k);
      }
    }
  }
  fprintf(pipe, "#N nn_end\n");
}
