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
#ifndef __USE_JAVA_CONTROLLER_H
#define __USE_JAVA_CONTROLLER_H

#ifndef WIN32

// #include <cstdio>
#include "abstractcontroller.h"

// server
// #include <cstdio>
#include <arpa/inet.h>
#include <exception>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr int BUFFER_SIZE = 1024;
constexpr int MAX_CONFIG_PARAM = 50;
constexpr int MAX_INTERNAL_PARAM = 50;

double atofstatic_cast<const char*>;
int ctoistatic_cast<const char*>;

// // // // // // // // using namespace std; // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header - use std:: prefix instead

/**
 * class for{

public:
  static int anzahl_Java_controller = 0;

  use_java_controller(const char* port_controller = "4444",
                               const char* port_internalParams = nullptr,
                               const char* name = "defaultRobot");

  static void addController() {
    use_java_controller::anzahl_Java_controller++;
  }

  virtual ~use_java_controller() override;

  /** initialisation of the controller with the given sensor/ motornumber
      Must be called before use.
  */
  virtual void init(int sensornumber, int motornumber, RandGen* randgen = nullptr);

  /** @return Number of sensors the controller was initialised
      with or 0 if not initialised */
  virtual int getSensorNumber() const override {
    return number_sensors;
  }

  /** @return Number of motors the controller was initialised
      with or 0 if not initialised */
  virtual int getMotorNumber() const override {
    return number_motors;
  }

  /** performs one step (includes learning).
      Calculates motor commands from sensor inputs.
      @param sensors sensors inputs scaled to [-1,1]
      @param sensornumber length of the sensor array
      @param motors motors outputs. MUST have enough space for motor values!
      @param motornumber length of the provided motor array
  */
  virtual void step(const sensor* sensors,
                    int sensornumber,
                    motor* motors,
                    int motornumber);
  /** performs one step without learning.
      @see step
  */
  virtual void stepNoLearning(const sensor*,
                              int number_sensors,
                              motor*,
                              int number_motors);

  /**
   * Methode verschickt message an Java-controller
   */
  void sendToJava(const char* message,
                  bool abbruch,
                  const char* meldung = "Fehler beim Senden der Daten zum Java-Controller\n");

  void closeJavaController();

  /**
  * executed once when guilogger or neuronvis or file logger is started to get the names of the
  inspectable parameters (names should be sent from java-controller and returned)
  */
  virtual std::list<iparamkey> getInternalParamNames() const override;
  /**
  * executed every step when guilogger or neuronvis or file logger is active to get the values of
  the inspectable parameters (values should be sent from java-controller and returned)
  */

  virtual std::list<iparamval> getInternalParams() const;

  // Bring base class methods{
    return true;
  }; // FIXME: store Parameter
  virtual bool restore(FILE* f) override {
    return true;
  }; // FIXME: restore Parameter

protected:
  int t = 0;
  const char* name;
  int number_sensors = 0;
  int number_motors = 0;
  int number_controlled = 0;

  bool serverOK = false;

  int server_controller = 0;
  int server_internalParams = 0;
  int client_controller = 0;
  int client_internalParams = 0;

  socklen_t client_controller_size;
  socklen_t client_internalParams_size;

  struct sockaddr_in server_controller_addr;
  struct sockaddr_in server_internalParams_addr;
  struct sockaddr_in client_controller_addr;
  struct sockaddr_in client_internalParams_addr;

  paramlist config_param_list;
  int anz_config_param = 0;

  iparamkeylist internal_keylist;
  iparamvallist internal_vallist;
  iparamvallist internal_vallist_alt;
  int anz_internal_param = 0;

  bool can_send = false;
  bool isFirst = false;
  bool isClosed = false;
  bool server_guilogger_isClosed = false;
  bool server_controller_isClosed = false;

  int anzahl_closed_Server = 0;

  double* motor_values_alt;
};

#endif // win32

#endif
