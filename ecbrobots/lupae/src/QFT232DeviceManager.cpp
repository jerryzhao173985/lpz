/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *    guettler@informatik.uni-leipzig.de                                   *
 *    jhoffmann@informatik.uni-leipzig.de                                  *
 *    wolfgang.rabe@01019freenet.de                                        *
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
 *   Revision 1.2  2010-11-09 17:56:55  wrabe
 *   - change of the communication protocoll between lupae and usb-isp-adapter
 *   - therefore recoding the dedicated methods
 *   - reduction of the overloded send_Message methods to one method only
 *   - insertion of QExtActions to join all events of menu-buttons as well of quickstart-buttons
 *   - adding two new functions to read out and write into the eeprom-space of the atmega128 at an ecb
 *   - change of the fontSize in the hexViewer, change of the total-width of the window
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "QFT232DeviceManager.h"

namespace lpzrobots {

  QFT232DeviceManager::QFT232DeviceManager() {
    int ret;
    devlist = NULL;
    deviceName = "";
    baudrate = 0;
    receiveBuffer.resize(1024);
    receiveBuffer.fill(0xFF);
    runListener = false;
    opened = false;

    /* Initialize context for subsequent function calls */
    ret = ftdi_init(&ftdic);
    if (ret < 0)
    {
      //fprintf(stderr, "<ftdi_init> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_init> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      //fprintf(stdout, "<ftdi_init> ok.\n");
      if (debug)
        emit textLog("<ftdi_init> ok.");
    }

    createDeviceList();
  }

  QFT232DeviceManager::~QFT232DeviceManager() {
    closeDevice();

    // delete Device-List
    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);
  }

  void QFT232DeviceManager::createDeviceList() {
    int ret;

    // Create Device-List
    devlist = NULL;

    ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6001);
    if (ret < 0)
    {
      // fprintf(stderr, "<ftdi_usb_find_all> failure: error-code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_usb_find_all> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      // fprintf(stdout, "<ftdi_device_list> ok, number devices found %d.\n", ret);
      if (debug)
        emit textLog("<ftdi_usb_find_all> ok, number devices found " + QString::number(ret) + ".");

      struct ftdi_device_list* devlist_tmp = devlist;
      while (devlist_tmp != NULL)
      {
        char manufacturer[100];
        char product_description[100];
        char serial_string[100];

        ret = ftdi_usb_get_strings(&ftdic, devlist_tmp->dev, static_cast<char*> &manufacturer, 100, static_cast<char*> &product_description, 100, static_cast<char*> &serial_string, 100);
        if (debug)
          emit textLog("  " + QString(manufacturer) + "-" + QString(product_description));

        devlist_tmp = devlist_tmp->next;
      }
    }
  }

  QStringList QFT232DeviceManager::getDeviceList() {
    int ret;
    struct ftdi_device_list* devlist_tmp = devlist;
    QStringList deviceNames;

    createDeviceList();

    while (devlist_tmp != NULL)
    {
      char manufacturer[100];
      char product_description[100];
      char serial_string[100];

      ret = ftdi_usb_get_strings(&ftdic, devlist_tmp->dev, static_cast<char*> &manufacturer, 100, static_cast<char*> &product_description, 100, static_cast<char*> &serial_string, 100);

      if (QString(manufacturer).startsWith("FTDI"))
      {
        if (QString(product_description).startsWith("USB-XBEE-Adapter"))
          deviceNames.append(product_description);
        if (QString(product_description).startsWith("USB-USART-Adapter"))
          deviceNames.append(product_description);
        if (QString(product_description).startsWith("USB-ISP-Adapter"))
          deviceNames.append(product_description);
      }

      // Weiter mit dem nächsten Eintrag in der Liste
      devlist_tmp = devlist_tmp->next;
    }
    return deviceNames;
  }
  bool QFT232DeviceManager::isDeviceAvailable(QString deviceName) {
    int ret;
    struct ftdi_device_list* devlist_tmp = devlist;

    if (deviceName.length() == 0)
      return false;

    while (devlist_tmp != NULL)
    {
      char manufacturer[100];
      char product_description[100];
      char serial_string[100];

      ret = ftdi_usb_get_strings(&ftdic, devlist_tmp->dev, static_cast<char*> &manufacturer, 100, static_cast<char*> &product_description, 100, static_cast<char*> &serial_string, 100);

      if (QString(manufacturer).startsWith("FTDI") && deviceName.startsWith(QString(product_description)))
        return true;

      // Weiter mit dem nächsten Eintrag in der Liste
      devlist_tmp = devlist_tmp->next;
    }
    return false;
  }

  int QFT232DeviceManager::openDevice(struct usb_device* usb_device_to_open, int baudrate_to_use) {
    int ret = 0;

    // Öffne dieses Device zum lesen und schreiben!
    ret = ftdi_usb_open_dev(&ftdic, usb_device_to_open);
    if (ret < 0)
    {
      //fprintf(stderr, "<ftdi_usb_open_dev> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_usb_open_dev> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      //fprintf(stdout, "<ftdi_usb_open_dev> ok.\n");
      if (debug)
        emit textLog("<ftdi_usb_open_dev> ok.");

      // Setze die Baudrate
      setBaudrate(baudrate_to_use);
      // Setze Latency-Timer auf 1ms.
      setLatencyTimer(1);
      // Setze die DTR-Line auf 0
      setDTR(0);

      usb_device_opened = usb_device_to_open;
      opened = true;
      runListener = true;
      start();
      emit deviceOpened();
    }
    return ret;
  }
  int QFT232DeviceManager::openDeviceByName(QString usb_deviceName_to_open, int baudrate_to_use) {
    int ret = 0;
    struct ftdi_device_list* devlist_tmp = devlist;

    // Wird der Thread bereits ausgeführt?
    if (isRunning())
    {
      runListener = false;
      while (isRunning())
        msleep(1);
    }

    while (devlist_tmp != NULL)
    {
      char manufacturer[100];
      char product_description[100];
      char serial_string[100];

      ret = ftdi_usb_get_strings(&ftdic, devlist_tmp->dev, static_cast<char*> &manufacturer, 100, static_cast<char*> &product_description, 100, static_cast<char*> &serial_string, 100);
      if (ret < 0)
      {
        if (debug)
          emit textLog("<ftdi_usb_get_strings> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
      } else
      {

        if (QString(manufacturer).startsWith("FTDI") && usb_deviceName_to_open.startsWith(QString(product_description)))
        {
          // Öffne dieses Device zum lesen und schreiben!
          ret = ftdi_usb_open_dev(&ftdic, devlist_tmp->dev);
          if (ret < 0)
          {
            //fprintf(stderr, "<ftdi_usb_open_dev> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
            if (debug)
              emit textLog("<ftdi_usb_open_dev> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
          } else
          {
            //fprintf(stdout, "<ftdi_usb_open_dev> ok.\n");
            if (debug)
              emit textLog("<ftdi_usb_open_dev> ok, '" + usb_deviceName_to_open + "'");

            // Setze die Baudrate
            setBaudrate(baudrate_to_use);
            // Setze Latency-Timer auf 1ms.
            setLatencyTimer(1);
            // Setze die DTR-Line auf 0
            setDTR(0);

            usb_device_opened = devlist_tmp->dev;
            opened = true;
            runListener = true;
            start();
            deviceName = QString(usb_deviceName_to_open);
            emit
            deviceOpened();
            break;
          }
        }// if 'FTDI'
      }

      // Weiter mit dem nächsten Eintrag in der Liste
      devlist_tmp = devlist_tmp->next;
    }//while
    return ret;
  }
  int QFT232DeviceManager::setBaudrate(int baudrate_to_set) {
    int ret;

    // Setze die Baudrate
    ret = ftdi_set_baudrate(&ftdic, baudrate_to_set);
    if (ret < 0)
    {
      //fprintf(stderr, "<ftdi_set_baudrate> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_set_baudrate> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      // fprintf(stdout, "<ftdi_set_baudrate> ok.\n");
      if (debug)
        emit textLog("<ftdi_set_baudrate> ok, set to " + QString::number(baudrate_to_set) + "bd.");
      baudrate = baudrate_to_set;
    }
    return ret;
  }
  int QFT232DeviceManager::setLatencyTimer(int latency_to_set) {
    int ret;

    // Setze Latency-Timer ...
    unsigned char latency = 0;
    ret = ftdi_get_latency_timer(&ftdic, &latency);
    if (ret < 0)
    {
      if (debug)
        emit textLog("<ftdi_get_latency_timer> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      // Is latency allready set?
      if (latency_to_set == latency)
      {
        if (debug)
          emit textLog("<ftdi_get_latency_timer> ok, latency allready set to " + QString::number(latency_to_set) + ".");
      } else
      {
        // set to new latency-time
        ret = ftdi_set_latency_timer(&ftdic, latency_to_set);
        if (ret < 0)
        {
          if (debug)
            emit textLog("<ftdi_set_latency_timer> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
        } else
        {
          if (debug)
            emit textLog("<ftdi_set_latency_timer> ok, set to " + QString::number(latency_to_set) + ".");
        }
      }
    }
    return ret;
  }

  int QFT232DeviceManager::setDTR(int dtr_val) {
    int ret;

    ret = ftdi_setdtr(&ftdic, dtr_val);
    if (ret < 0)
    {
      if (debug)
        emit textLog("<ftdi_setdtr> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      if (debug)
        emit textLog("<ftdi_setdtr> ok, set to " + QString::number(dtr_val) + ".");
    }
    return ret;
  }
  int QFT232DeviceManager::closeDevice() {
    int ret;

    // den Tread beenden
    runListener = false;
    while (isRunning())
      msleep(1);

    ret = ftdi_usb_close(&ftdic);
    if (ret < 0)
    {
      //fprintf(stderr, "<ftdi_usb_close> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_usb_close> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      //fprintf(stdout, "<ftdi_usb_close> ok, usb device closed.\n");
      if (debug)
        emit textLog("<ftdi_usb_close> ok.");
    }
    return ret;
  }
  int QFT232DeviceManager::writeData(QByteArray msg) {
    int ret;

    ret = ftdi_write_data(&ftdic, (unsigned char *) msg.data(), msg.length());
    if (ret < 0)
    {
      //fprintf(stderr, "<ftdi_write_data> failure: error-Code %d (%s)\n", ret, ftdi_get_error_string(&ftdic));
      if (debug)
        emit textLog("<ftdi_write_data> failure: error-Code " + QString::number(ret) + " (" + ftdi_get_error_string(&ftdic) + ")");
    } else
    {
      //fprintf(stdout, "<ftdi_write_data> ok, %d byte written.\n", ret);
      //for(i=0; i<ret; ++i) fprintf(stdout, "%X%X ", msg[i]>>4&0xFF, msg[i]>>0&0x0F); fprintf(stdout, "\n");
      if (debug)
        emit textLog("<ftdi_write_data> ok, " + QString::number(ret) + " bytes written.");

      if (debug)
      {
        int i;
        QString msg_hex_line;
        for (i = 0; i < ret; ++i)
        {
          msg_hex_line += QString::number(msg[i] >> 4 & 0x0F, 16).toUpper();
          msg_hex_line += QString::number(msg[i] >> 0 & 0x0F, 16).toUpper();
          msg_hex_line += " ";
        }
        emit textLog((msg_hex_line));
      }

    }
    return ret;
  }
  void QFT232DeviceManager::run() {

    int ret = 0;
    int index = 0;
    int iReadMode = 0;
    int iCheckSum = 0;
    int iPacketLength = 0;

    if (!opened)
    {
      if (debug)
        emit textLog("Device is not open yet. Unable to read!");
      return;
    }

    if (debug)
      emit textLog("thread '" + QString::number(currentThreadId()) + "' started.");

    while (runListener) // main loop
    {
      unsigned char c = 0;

      //  get one character from port
      do
      {
        ret = ftdi_read_data(&ftdic, &c, 1);
        if (ret == 1 || !runListener)
          break;
        else
          usleep(10);
      } while (ret != 1);

      // Startzeichen der Uebertragung?
      if (c == 0x7E)
      {
        receiveBuffer.fill(0xFF);
        index = 0;
        iReadMode = 0;
        iCheckSum = 0;
        receiveBuffer[index++] = c;
      } else
      {
        if (c == 0x7D)
        {
          // Escape-Behandlung
          do
          {
            ret = ftdi_read_data(&ftdic, &c, 1);
            if (ret == 1 || !runListener)
              break;
            else
              usleep(10);
          } while (ret != 1);
          c ^= 0x20;
        }

        switch (iReadMode)
        {
          case 0:
            // lese das Langenfeld (16Bit)
            // Bits[15:8]
            iPacketLength = (c << 8);
            receiveBuffer[index++] = c;
            ++iReadMode;
            break;

          case 1:
            // Lese das Langenfeld (16Bit)
            // Bits[7:0]
            iPacketLength += (c << 0);
            receiveBuffer[index++] = c;
            // Wenn die Lange des Packetes groesser als 250 ist, dann nicht Empfangen!
            if (900 < iPacketLength)
              iReadMode = 0;
            ++iReadMode;
            break;

          case 2:
            // Lese das Packet bis zum Ende ein
            receiveBuffer[index] = c;
            iCheckSum += c;
            ++index;
            if (index >= iPacketLength + 3)
              iReadMode = 900;
            break;

          case 900:
            // Lese die Pruefsumme ein
            receiveBuffer[index] = c;
            ++index;
            iCheckSum += c;
            iCheckSum += 1;
            iCheckSum = iCheckSum % 256;

            if (iCheckSum == 0)
            {
              emit newData(receiveBuffer.mid(0, index));
            }
            ++iReadMode;
            break;
          default:
            iReadMode = 0;
            break;
        } // switch
      } // if
    }// while
  }

}//namespace lpzrobots
