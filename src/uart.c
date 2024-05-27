#include "../include/uart.h"

int serial_port;

int UART_open(void)
{
    struct termios tty;

    serial_port = open("/dev/ttyACM0", O_RDWR);

    // Check for errors
    if (serial_port < 0)
    {
        perror("Error opening serial port");
        return 1;
    }

    if(tcgetattr(serial_port, &tty) != 0) 
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    //cflag bepaald de flow control, data bits, stop bits, en parity bits
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)


    tty.c_lflag &= ~ICANON;//disable canonical mode
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);//disable any special handling of received bytes

    //oflag bepaald de output modes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    //hoe blocking of non blocking is de uart funcites 
    tty.c_cc[VTIME] = 0;// Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;// Wait for up to 0 bytes to be received

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tcsetattr(serial_port, TCSANOW, &tty);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    printf("Serial port opened\n");
    return 0;
}


void UART_write(unsigned char *msg)
{

    printf("Sending: %s\n", msg);
    write(serial_port, msg, strlen(msg));
    write(serial_port, "\r\n", 1);
}

int UART_read(char *buf, size_t bufsize)
{
    if (read(serial_port, buf, bufsize))
    {
        return 0;
    }
    return -1;
}

int UART_check_connection(void)
{
    char test_msg[] = "\0\r\n";
    if (write(serial_port, test_msg, strlen(test_msg)) < 0) {
        return -1;
    }
    return 0;
}

void UART_close(void)
{
    close(serial_port);
}
