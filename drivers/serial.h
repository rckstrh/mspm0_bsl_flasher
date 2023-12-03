/*
 * serial.h
 *
 *  Created on: Dec 1, 2023
 *      Author: Jonas Rockstroh
 */

#include <iostream>
#include "fcntl.h"
#include "errno.h"
#include "termio.h"
#include "unistd.h"
#include "cstring"

class Serial {
    public:
        Serial(const char* __file);
        ~Serial();
        bool _open();
        int readBytes(char buff[], size_t buf_size, int _max_timeout_tries=10);
        int writeBytes(const char buff[], size_t buf_size);
        
    private:
        const char* port;
        int serial_port;
        struct termios tty;
};