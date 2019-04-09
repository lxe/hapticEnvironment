#include "listener.h"
#include "haptics/haptics.h"
#include "network.h"
#include "core/controller.h"

using namespace chai3d;
using namespace std;

extern ControlData controlData;
extern HapticData hapticsData;

void startListener()
{
  openMessageHandlerListenSocket(controlData.LISTENER_IP, controlData.LISTENER_PORT);
  controlData.listenerThread = new cThread();
  controlData.listenerThread->start(updateListener, CTHREAD_PRIORITY_HAPTICS);
  controlData.listenerUp = true;
}

void updateListener()
{
  char rawPacket[MAX_PACKET_LENGTH];
  char* packetPointer = rawPacket;

  while (controlData.simulationRunning)
  {
    memset(rawPacket, 0, MAX_PACKET_LENGTH);
    int bytesRead = readPacket(packetPointer);
    if (bytesRead > 0) {
      //cout << "Bytes read " << bytesRead << endl;
      parsePacket(packetPointer);
    }
    usleep(50); // 1000 microseconds = 1 millisecond
  }
 closeListenSocket();
 controlData.listenerUp = false;
}

void closeListener()
{
  closeListenSocket();
}

void startDataLogger(void)
{
  openDataSavingSocket(controlData.SENDER_IP, controlData.DATA_LOG_PORT);
  controlData.dataLogThread = new cThread();
  controlData.dataLogThread->start(updateDataLogger, CTHREAD_PRIORITY_HAPTICS);
  controlData.dataLoggerUp = true;
}

void closeDataLogger()
{
 closeDataSavingSocket(); 
}

void updateDataLogger()
{
  char rawPacket[MAX_PACKET_LENGTH];
  char* packetPointer = rawPacket;

  while (controlData.dataFile.is_open())
  {
    if (controlData.simulationRunning == false)
    {
      controlData.dataFile.close();
      break;
    }
    memset(rawPacket, 0, MAX_PACKET_LENGTH);
    int bytesRead = readData(packetPointer);
    if (bytesRead > 0) {
      controlData.dataFile.write(rawPacket, bytesRead);
    }
    usleep(50); // 1000 microseconds = 1 millisecond
  }
 closeDataSavingSocket();
 controlData.dataLoggerUp = false;
}
