#pragma once 

#ifndef _NETWORK_H_
#define _NETWORK_H_  

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#endif

#include "messageDefinitions.h"
#include "platform_compat.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <ctime>
#include <assert.h>
#include <math.h>
#include <sstream>
#include <iostream>

#ifndef _WIN32
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

int addMessageHandlerModule();
int subscribeToTrialControl();
int openMessagingSocket();
void closeMessagingSocket();
int readPacket(char* packet);

#endif
