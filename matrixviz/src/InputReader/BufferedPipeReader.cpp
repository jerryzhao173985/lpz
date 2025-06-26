
#include "BufferedPipeReader.h"
#include <unistd.h>

#define BUFFERED_PIPE_READER_WAIT_TIME 1000

/**
* Constructor of the BufferedPipeReader
* set the signal-slot
*/

BufferedPipeReader::BufferedPipeReader ( AbstractPipeReader *apr ) : apr ( apr )
{
  std::cout << "new BufferedPipeReader()" << std::endl;
  // AbstractPipeReader is not a QObject, so we need to check if it's a QObject-derived class
  if (QObject* qobj = dynamic_cast<QObject*>(apr)) {
    QObject::connect ( qobj, SIGNAL ( newData() ), this, SLOT ( newIncomingData() ) );
    QObject::connect ( qobj, SIGNAL ( finished() ), this, SLOT ( terminate() ) );
  }
}

/**
* main loop of BufferedPipeReader
* fire a signal that will bee initiate a reading of the buffer
* are no data in buffer, this thread will be wait for specific time
*/
void BufferedPipeReader::run()
{
  std::cout << "BufferedPipeReader: start()" << std::endl;
  // AbstractPipeReader doesn't have start() method
  // apr->start();

  while ( true )
  {
    usleep(100);
  }
}

/**
* represent the slot that will fill the buffer, if a string-line
* is read from the stdin (or something else)
*/

void BufferedPipeReader::newIncomingData()
{
  QMutexLocker locker ( &mutex );

  input_buffer.push_back ( ( buffer_t ) apr->getDataLine() );
  std::cout << "BufferedPipeReader: SLOT(newIncomingData()) - " << input_buffer.size() << std::endl;
  emit newData();
}

// void BufferedPipeReader::waitForChannelData() {
//   apr->waitForChannelData();
// }

/**
* return the channelLine, that is hold by the given AbstractPipeReader
* (ex. SimplePipeReader)
*/
std::list<std::string> BufferedPipeReader::getChannelLine()
{
//   std::cout << "BufferedPipeReader: getChannelLine()" << std::endl;
  QMutexLocker locker ( &mutex );
  return apr->getChannelLine();
}

/**
* return the first data of the buffer
*/
std::list<double> BufferedPipeReader::getDataLine()
{
//   std::cout << "BufferedPipeReader: getDataLine()" << std::endl;
  QMutexLocker locker ( &mutex );
  std::list<double> dataLine;
  if (!input_buffer.empty()) {
    dataLine = input_buffer.front();
    input_buffer.pop_front();
  }
  return dataLine;
}

BufferedPipeReader::~ BufferedPipeReader()
{

        std::cout << "BuferedPipeReader: ByeBye()" << std::endl;
}
