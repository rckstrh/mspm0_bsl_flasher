#include "serial.h"

Serial::Serial(const char* __file, int _verbose_level) : port(__file), verbose_level(_verbose_level)
{
    
}

Serial::~Serial()
{
    _close();
}

bool Serial::_open(speed_t __speed)
{
    serial_port = open(port, O_RDWR);
    if (serial_port < 0) {
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return false;
    }
    
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        serial_port = -1;
        return false;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10;   // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;     // purely time based read

    change_baud(__speed);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        serial_port = -1;
        return false;
    } 

    _flush();
   

    return true;
}

inline void Serial::change_baud(speed_t __speed)
{
    cfsetispeed(&tty, __speed);
    cfsetospeed(&tty, __speed);
}

inline void Serial::_flush()
{
    ioctl(serial_port, TCFLSH, 0); // flush receive
    ioctl(serial_port, TCFLSH, 1); // flush transmit
    ioctl(serial_port, TCFLSH, 2); // flush both
}

int Serial::_close() 
{
    if(serial_port >= 0)
        return close(serial_port);
    
    return -1;
}

int Serial::writeBytes(const char buff[], size_t buf_size) 
{
    if (serial_port < 0)
        return -1;

    // debug printfs
    if(verbose_level > 2) {
        printf("Serial write %ld bytes: ", buf_size);
        for(int i=0; i < buf_size; i++) {
            printf("%02x ", (unsigned char) buff[i]);
        }
        printf("\n");
    }


    int n = write(serial_port, buff, buf_size);
    return n;
}

int Serial::readBytes(char buff[], size_t buf_size, int _max_timeout_tries) 
{
    if (serial_port < 0)
        return -1;

    int timeout_tries = _max_timeout_tries;
    int bytes_read = 0;

    while(bytes_read != buf_size) {
        int new_bytes_read = read(serial_port, (char*) buff+bytes_read, buf_size-bytes_read);
        bytes_read += new_bytes_read;

        if(new_bytes_read == 0)
            timeout_tries--;
        else
            timeout_tries = _max_timeout_tries;
        
        if(timeout_tries == 0) {
            return -1;
        }
    }

    // debug printfs
    if(verbose_level > 2) {
        printf("Serial read %d bytes: ", bytes_read);
        for(int i=0; i < bytes_read; i++) {
            printf("%02x ", (unsigned char) buff[i]);
        }
        printf("\n");
    }

    return bytes_read;
}
