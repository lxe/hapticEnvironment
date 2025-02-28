#pragma once

#ifndef _PLATFORM_COMPAT_H_
#define _PLATFORM_COMPAT_H_

#ifdef _WIN32
    #include <winsock2.h>  // Must come before windows.h
    #include <ws2tcpip.h>
    #include <windows.h>

namespace platform {
    // Sleep functions compatibility
    inline void usleep(unsigned long usec) {
        ::Sleep((usec + 999) / 1000); // Convert microseconds to milliseconds, rounding up
    }

    inline void sleep(unsigned long sec) {
        ::Sleep(sec * 1000); // Convert seconds to milliseconds
    }

    // Socket option compatibility
    #ifndef SO_REUSEPORT
        #define SO_REUSEPORT SO_REUSEADDR
    #endif

    // Socket function compatibility
    inline int setsockopt(SOCKET s, int level, int optname, const void* optval, int optlen) {
        return ::setsockopt(s, level, optname, static_cast<const char*>(optval), optlen);
    }

    // Handle ioctl differences
    inline int ioctl(SOCKET s, long cmd, int* argp) {
        u_long arg = *argp;
        int result = ioctlsocket(s, cmd, &arg);
        *argp = arg;
        return result;
    }

    // Handle close for sockets
    inline int close(SOCKET s) {
        return closesocket(s);
    }

    // Handle bind for sockets - using Windows types
    inline int bind(SOCKET s, const struct sockaddr* name, int namelen) {
        return ::bind(s, name, namelen);
    }
} // namespace platform

    // Constant expression helper
    #define CONSTEXPR constexpr

#else
    #define CONSTEXPR constexpr
#endif // _WIN32

#endif // _PLATFORM_COMPAT_H_ 