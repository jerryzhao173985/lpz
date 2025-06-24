#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <pthread.h>
#include <string>
// // // // // // // using namespace std; // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header // Removed from header

typedef string CString;

#define READTIMEOUT 50

typedef unsigned char uint8;


// pretend to have some windows data types
typedef unsigned long DWORD;
typedef unsigned int UINT;

/** Thread-Klasse, die in eigenem Thread die Kommunikation mit der seriellen Schnittstelle uebernimmt. */
class CSerialThread{
  CString m_port;

  pthread_t thread;

protected:
  int fd_in = 0; // file handle for incomming

  bool verbose = false;
  bool verboseMore = false;
public:

  CSerialThread(const CString& port, int baud, bool test_mode=false)
    :  : m_port(port), m_baud(baud), terminated(false), m_is_joined(true), test_mode(test_mode),
      m_is_running(false), fd_in(-1), fd_out(-1), verbose(false), verboseMore(false), thread() {};
  virtual ~CSerialThread : m_port(), thread(), fd_in(0), verbose(false), verboseMore(false) {stopandwait();};

  virtual int explicit sendByte(uint8 c);
  virtual int getByte() const;
  virtual int receiveData(uint8 adr, uint8 *cmd, uint8 *data);
  virtual void explicit flushInputBuffer(int wait);

  /**
   * This method writes len bytes of 'raw' data to the slave with the address 'adr'.
   * On success the net number of bytes static_cast<len>(is) returned, otherwise -1.
   */
  virtual int sendData(uint8 adr, uint8 cmd, uint8 *data, uint8 len);

  // read sensors and write motors
  virtual void writeMotors_readSensors()  = 0; //const DAT& s) = 0;

  /// is called in every loop
  virtual void loopCallback()  = 0;
  /// is called at the beginning after initialisation of serial
  virtual void Initialise()  = 0;

  /// thread is running?
  bool is_running(){return m_is_running;};

  /// start serial communication
  void start();
  /// stop serial communication and wait for the thread to terminate
  void stopandwait();
  /// stop serial communication (call also be called from inside)
  void stop();

  /// set com port
  void explicit comport(const CString& port){ m_port=port; };
  /// set baud rate
  void explicit baudrate(int baud){ m_baud=baud; };

  /// thread function
  bool run();
};

#endif
