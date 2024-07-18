// aux_functions.h
#ifndef AUX_FUNCTIONS_H
#define AUX_FUNCTIONS_H

#include <ncurses.h>
#include "main.h"

void gps_to_ymdhms(CLOCK *clock);

void gps_prep(unsigned char *packet, CLOCK *clock);

void decodeStatus(const unsigned char status, const unsigned int channel, WINDOW *win);

void checkStatus(unsigned char *packetBytes, WINDOW *win);

double u2r(const double u, str_def_usr_var *usr_var);

double r2t(const double r, str_def_usr_var *usr_var);

double convertToTemperature(const int rawData, str_def_usr_var *usr_var);

double returnVoltage(const int rawData, str_def_usr_var *usr_var);

int AccessNonalignedInt(unsigned char *address);

#endif // AUX_FUNCTIONS_H