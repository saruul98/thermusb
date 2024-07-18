#ifndef READ_CMD_H
#define READ_CMD_H

#include "main.h"
#include <assert.h>
#include <getopt.h>
#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <thread>
 

void print_usage(WINDOW *win);
void read_cmd(int argc, char **argv, WINDOW *win, struct str_def_usr_var *usr_var);

#endif // READ_CMD_H
