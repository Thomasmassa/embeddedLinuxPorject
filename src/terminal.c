#include "../include/terminal.h"

void TerminalClear() {
    printf("\033[H\033[J");
}
