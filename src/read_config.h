#ifndef READ_CONFIG_H
#define READ_CONFIG_H

#include "main.h"
#include <libconfig.h>
#include <stdlib.h>
#include <ncurses.h>

void read_conf_file(struct str_def_usr_var *usr_var, const char* conf_file, WINDOW *win);

#endif // READ_CONFIG_H