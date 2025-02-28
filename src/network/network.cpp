#include "network.h"
#include "core/controller.h"
#include "platform_compat.h"

using namespace chai3d;
using namespace std;

/**
 * @file network.h
 * @file network.cpp
 * @brief General networking functions.
 *
 * Functions in this file are responsible for subscribing to MessageHandler, and opening sockets for
 * messaging. See streamer.cpp and listener.cpp for using the threads used for listening to messages
 * and streaming data. The robot, haptic, and graphics code are one module. Each module has its own
 * IP address and port, which is assigned to a specific module number by the MessageHandler.
 * At least two modules are required to run: 1) Robot and haptic code, and 2) Trial control code.
 */

extern ControlData controlData;

struct sockaddr_in msgStruct;
int msgLen = sizeof(msgStruct);

/**
 * This function adds the robot environment to the MessageHandler. Information such as the module
 * number, IP address, and port are set through command-line inputs and stored in the controlData
 * external struct.
 */
int addMessageHandlerModule()
{
  auto addMod = controlData.client->call("addModule", controlData.MODULE_NUM, controlData.IPADDR, controlData.PORT);
  return addMod.as<int>();
}

/**
 * Subscribe to the trial control module. This tells MessageHandler to take all messages sent by
 * Trial Control and send them to this module.
 */
int subscribeToTrialControl() 
{
  bool subscribed = false;
  clock_t begin = clock();
  while (subscribed == false) {
    auto subscribe = controlData.client->async_call("subscribeTo", 1, 2);
    subscribe.wait();
    if (subscribe.get().as<int>() == 1) {
      subscribed = true;
      return 1;
    }
    clock_t now = clock();
    double elapsed = double(now - begin)/CLOCKS_PER_SEC;
    if (elapsed > 120) {
      cout << "Error subscribing to Trial Control, exiting." << endl;
      return 0;
    }
    platform::sleep(5); // 1000 microseconds = 1 millisecond
  }
  return 1;
}

/**
 * This specifically opens the \b listening port for this module. Sending ports are opened via
 * MessageHandler. In other words, this opens the socket that is assigned to this module.
 */
int openMessagingSocket()
{
  cout << "Opening messaging socket..." << endl;
  cout << "Creating socket with AF_INET, SOCK_DGRAM, IPPROTO_UDP..." << endl;
  controlData.msg_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (controlData.msg_socket < 0) {
    cout << "Opening messaging socket failed" << endl;
    exit(1);
  }
  cout << "Socket created successfully with fd: " << controlData.msg_socket << endl;

  cout << "Setting up socket address structure..." << endl;
  memset((char*) &msgStruct, 0, msgLen);
  msgStruct.sin_family = AF_INET;
  msgStruct.sin_port = htons(controlData.PORT);
  msgStruct.sin_addr.s_addr = inet_addr(controlData.IPADDR);
  cout << "Socket structure initialized with port " << controlData.PORT << " and IP " << controlData.IPADDR << endl;

  cout << "Setting socket options..." << endl;
  int opt = 1;
  int broadcast = platform::setsockopt(controlData.msg_socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)); 
  cout << "SO_BROADCAST result: " << broadcast << endl;
  int reuseAddr = platform::setsockopt(controlData.msg_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  cout << "SO_REUSEADDR result: " << reuseAddr << endl;
  int reusePort = platform::setsockopt(controlData.msg_socket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
  cout << "SO_REUSEPORT result: " << reusePort << endl;

  if (broadcast < 0 || reuseAddr < 0 || reusePort < 0)
  {
    cout << "Failed to set socket options" << endl;
    cout << "broadcast=" << broadcast << ", reuseAddr=" << reuseAddr << ", reusePort=" << reusePort << endl;
    exit(1);
  }
  cout << "Socket options set successfully" << endl;

  cout << "Binding socket..." << endl;
  int bind_sock_in = platform::bind(controlData.msg_socket, (struct sockaddr*) &msgStruct, msgLen);
  if (bind_sock_in < 0) {
    cout << "Error binding messaging socket, result=" << bind_sock_in << endl;
    exit(1);
  }
  cout << "Socket bound successfully" << endl;
  return 1; 
}

/**
 * Closes the socket for this module.
 */
void closeMessagingSocket()
{
  shutdown(controlData.msg_socket, 2);
}

/**
 * Receives messages on the messaging socket and sends them to be parsed by the controller
 * @param packetPointer is a char pointer to store the read-in bytes
 * @see parsePacket
 */
int readPacket(char* packetPointer)
{
  int value = 0, bytesRead = 0;
  platform::ioctl(controlData.msg_socket, FIONREAD, &value);
  if (value > 0) {
    bytesRead = recvfrom(controlData.msg_socket, packetPointer, MAX_PACKET_LENGTH, 0, (struct sockaddr*) &msgStruct, (socklen_t *) &msgLen);
    //cout << bytesRead << " bytes read from socket" << endl;
  }
  return bytesRead;
}

/**
 * Close all messaging sockets
 */
void closeAllConnections()
{
  platform::close(controlData.msg_socket);
}
