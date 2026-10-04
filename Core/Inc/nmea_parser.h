#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include "gps.h"

void NMEA_ParseRMC(char *sentence,
                   GPS_Data_t *gps);

#endif
