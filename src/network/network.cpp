#include "network.h"
#include "core/controller.h"

using namespace chai3d;
using namespace std;

extern ControlData controlData;

struct sockaddr_in listenerStruct;
int listenerLen = sizeof(listenerStruct);

struct sockaddr_in senderStruct; //, listenerStruct, dataStruct, dataLogStruct;
int senderLen = sizeof(senderStruct);
/*int listenerLen = sizeof(listenerStruct);
int dataLen = sizeof(dataStruct);
int dataLogLen = sizeof(dataLogStruct);
*/

void openMessagingSocket(const char* ipAddr, int listenerPort, int senderPort)
{
  cout << "Opening listener socket..." << endl;
  controlData.listener_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.listener_socket < 0) {
    cout << "Opening listener socket failed" << endl;
    exit(1);
  }
  //fcntl(controlData.listener_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &listenerStruct, 0, listenerLen);
  listenerStruct.sin_family = AF_INET;
  listenerStruct.sin_port = htons(listenerPort);
  listenerStruct.sin_addr.s_addr = inet_addr(ipAddr);

  int opt = 1;
  int broadcast = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)); 
  int reuseAddr = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int reusePort = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0)
  {
    cout << "Failed to set socket options" << endl;
    exit(1);
  }

  int bind_sock_in = bind(controlData.listener_socket, (struct sockaddr*) &listenerStruct, listenerLen);
  if (bind_sock_in < 0) {
    cout << "Error binding listener socket" << endl;
    exit(1);
  }

  cout << "Opening sender socket..." << endl;
  controlData.sender_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.sender_socket < 0) {
    cout << "Opening sender socket failed" << endl;
    exit(1);
  }
  //fcntl(controlData.listener_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &senderStruct, 0, senderLen);
  senderStruct.sin_family = AF_INET;
  senderStruct.sin_port = htons(senderPort);
  senderStruct.sin_addr.s_addr = inet_addr(ipAddr);

  broadcast = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)); 
  reuseAddr = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  reusePort = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0)
  {
    cout << "Failed to set socket options" << endl;
    exit(1);
  }
}

void closeMessagingSocket()
{
  shutdown(controlData.listener_socket, 2);
}
/*
void openMessageHandlerSendSocket(const char* ipAddr, int port) 
{
  cout << "Opening sending socket..." << endl;
  controlData.sender_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.sender_socket < 0) {
    cout << "Opening sending socket failed." << endl;
    exit(1);
  }
  
  //fcntl(controlData.sender_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &senderStruct, 0, senderLen);
  senderStruct.sin_family = AF_INET;
  senderStruct.sin_port = htons(port);
  if (inet_aton(ipAddr, &senderStruct.sin_addr) == 0) {
    cout << "inet_aton failed" << endl;
    exit(1);
  }

  int opt = 1;
  int broadcast = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
  int reuseAddr = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int reusePort = setsockopt(controlData.sender_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0) {
    cout << "Setting socket options failed" << endl;
    exit(1);
  }
}
void closeSendSocket()
{
  shutdown(controlData.sender_socket, 2);
}

void openMessageHandlerListenSocket(const char* ipAddr, int port)
{
  cout << "Opening listener socket..." << endl;
  controlData.listener_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.listener_socket < 0) {
    cout << "Opening listener socket failed" << endl;
    exit(1);
  }
  //fcntl(controlData.listener_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &listenerStruct, 0, listenerLen);
  listenerStruct.sin_family = AF_INET;
  listenerStruct.sin_port = htons(port);
  listenerStruct.sin_addr.s_addr = inet_addr(ipAddr);

  int opt = 1;
  int broadcast = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)); 
  int reuseAddr = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int reusePort = setsockopt(controlData.listener_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0)
  {
    cout << "Failed to set socket options" << endl;
    exit(1);
  }

  int bind_sock_in = bind(controlData.listener_socket, (struct sockaddr*) &listenerStruct, listenerLen);
  if (bind_sock_in < 0) {
    cout << "Error binding listener socket" << endl;
    exit(1);
  }
}

void closeListenSocket()
{
  shutdown(controlData.listener_socket, 2);
}

void openDataSocket(const char* ipAddr, int port) 
{
  cout << "Opening data streaming socket..." << endl;
  controlData.data_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.data_socket < 0) {
    cout << "Opening data streaming socket failed." << endl;
    exit(1);
  }
  //fcntl(controlData.data_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &dataStruct, 0, dataLen);
  dataStruct.sin_family = AF_INET;
  dataStruct.sin_port = htons(port);
  if (inet_aton(ipAddr, &dataStruct.sin_addr) == 0) {
    cout << "inet_aton failed" << endl;
    exit(1);
  }

  int opt = 1;
  int broadcast = setsockopt(controlData.data_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
  int reuseAddr = setsockopt(controlData.data_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int reusePort = setsockopt(controlData.data_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0) {
    cout << "Setting socket options failed" << endl;
    exit(1);
  }
}
void closeDataSocket()
{
  shutdown(controlData.data_socket, 2);
}
*/
int sendPacket(char* packet, uint16_t lengthPacket)
{
    if (sendto(controlData.sender_socket, packet, lengthPacket, 0, (struct sockaddr*) &senderStruct, senderLen) < 0)
    {
      cout << "Sending error" << endl;
      return 0;
    }
  return 1;
}

int readPacket(char* packetPointer)
{
  int value = 0, bytesRead = 0;
  ioctl(controlData.listener_socket, FIONREAD, &value);
  if (value > 0) {
    bytesRead = recvfrom(controlData.listener_socket, packetPointer, MAX_PACKET_LENGTH, 0, (struct sockaddr*) &listenerStruct, (socklen_t *) &listenerLen);
    //cout << bytesRead << " bytes read from socket" << endl;
  }
  return bytesRead;
}
/*
int sendData(char* packet, uint16_t lengthPacket) //, bool isData)
{
    if (sendto(controlData.data_socket, packet, lengthPacket, 0, (struct sockaddr*) &dataStruct, dataLen) < 0)
    {
      cout << "Data streaming error" << endl;
      return 0;
    }
  return 1;
}

void openDataSavingSocket(const char* ipAddr, int port) 
{
  cout << "Opening data logging socket..." << endl;
  controlData.dataLog_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.dataLog_socket < 0) {
    cout << "Opening data logging socket failed." << endl;
    exit(1);
  }
  
  //fcntl(controlData.dataLog_socket, F_SETFL, O_NONBLOCK);

  memset((char *) &dataLogStruct, 0, dataLogLen);
  dataLogStruct.sin_family = AF_INET;
  dataLogStruct.sin_port = htons(port);
  if (inet_aton(ipAddr, &dataLogStruct.sin_addr) == 0) {
    cout << "inet_aton failed" << endl;
    exit(1);
  }

  int opt = 1;
  int broadcast = setsockopt(controlData.dataLog_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
  int reuseAddr = setsockopt(controlData.dataLog_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  int reusePort = setsockopt(controlData.dataLog_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0) {
    cout << "Setting socket options failed" << endl;
    exit(1);
  }
  int bind_sock_in = bind(controlData.dataLog_socket, (struct sockaddr*) &dataLogStruct, dataLogLen);
  if (bind_sock_in < 0) {
    cout << "Error binding data logging socket" << endl;
    exit(1);
  }
}

void closeDataSavingSocket()
{
  shutdown(controlData.dataLog_socket, 2);
}

int readData(char* packetPointer)
{
  int value = 0, bytesRead = 0;
  ioctl(controlData.dataLog_socket, FIONREAD, &value);
  if (value > 0) {
    bytesRead = recvfrom(controlData.dataLog_socket, packetPointer, MAX_PACKET_LENGTH, 0, (struct sockaddr*) &dataLogStruct, (socklen_t *) &dataLogLen);
    //cout << bytesRead << " bytes read from socket" << endl;
  }
  return bytesRead;
}
*/
void closeAllConnections()
{
  close(controlData.listener_socket);

  //close(controlData.sender_socket);
  //close(controlData.listener_socket);
  //close(controlData.data_socket);
  //close(controlData.dataLog_socket);
}
