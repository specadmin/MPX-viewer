//-----------------------------------------------------------------------------
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include "common.h"
#include "dump.h"
//-----------------------------------------------------------------------------
__inline BYTE CRC8(const BYTE* buf, BYTE size)
{
    BYTE crc = 0;
    while(size--)
    {
        crc = CRC8_table[crc ^ *buf++];
    }
    return crc;
}
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    int fd;
    BYTE rx_buf[64];
    BYTE buf[32];
    BYTE pos = 0;
    if(argc < 2)
    {
        printf("Error: not enough parameters\n");
        printf("Usage: MPX-viewer {port}\n");
        printf("Example: MPX-viewer /dev/ttyUSB0\n");
        return 1;
    }

    char* port_name = argv[1];
    struct termios options;

    fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
    fcntl(fd, F_SETFL, 0);
    memset(&options, 0, sizeof(struct termios));
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);

    if(fd == -1)
    {
        printf("Unable to open port: %s\n", port_name);
        return ENOENT;
    }


    BYTE data;
    CDump dump;
    while(1)
    {
        while(read(fd, rx_buf, 1) <= 0);
        data = rx_buf[0];
        if(!pos && data == 0x40)
        {
            continue;
        }
        buf[pos++] = data;
        if(data == 0x7E)
        {
            if(CRC8(buf, pos - 2) == buf[pos - 2])
            {
                dump.push(buf);
                dump.show();
            }
            else
            {
                fprintf(stderr, "Invalid CRC\n");
            }
            pos = 0;
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------
