#ifndef UART_H
#define UART_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

int UART_open(void);
void UART_close(void);
int UART_read(char *buf, size_t bufsize);
void UART_write(unsigned char *msg);
int UART_check_connection(void);

#endif