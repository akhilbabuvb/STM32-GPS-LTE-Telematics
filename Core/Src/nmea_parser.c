#include "nmea_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>



static void GetField(char *sentence,
                     uint8_t fieldNumber,
                     char *destination,
                     uint8_t maxLength)
{
    uint8_t commaCount = 0;
    uint8_t index = 0;

    while (*sentence != '\0')
    {
        if (*sentence == ',')
        {
            commaCount++;

            if (commaCount > fieldNumber)
                break;

            sentence++;
            continue;
        }

        if (commaCount == fieldNumber)
        {
            if (index < (maxLength - 1))
            {
                destination[index++] = *sentence;
            }
        }

        sentence++;
    }

    destination[index] = '\0';
}

static float ConvertCoordinate(char *coordinate)
{
    float value = atof(coordinate);

    int degrees = (int)(value / 100);

    float minutes = value - (degrees * 100);

    return degrees + (minutes / 60.0f);
}

static void FormatTime(char *utc, char *output)
{
    sprintf(output,
            "%c%c:%c%c:%c%c",
            utc[0], utc[1],
            utc[2], utc[3],
            utc[4], utc[5]);
}

static void FormatDate(char *date, char *output)
{
    sprintf(output,
            "%c%c/%c%c/20%c%c",
            date[0], date[1],
            date[2], date[3],
            date[4], date[5]);
}

void NMEA_ParseRMC(char *sentence, GPS_Data_t *gps)
{
    char utc[20];
    char status[5];
    char latitude[20];
    char ns[5];
    char longitude[20];
    char ew[5];
    char speed[20];
    char heading[20];
    char date[20];

    memset(gps, 0, sizeof(GPS_Data_t));

    GetField(sentence,1,utc,sizeof(utc));
    GetField(sentence,2,status,sizeof(status));
    GetField(sentence,3,latitude,sizeof(latitude));
    GetField(sentence,4,ns,sizeof(ns));
    GetField(sentence,5,longitude,sizeof(longitude));
    GetField(sentence,6,ew,sizeof(ew));
    GetField(sentence,7,speed,sizeof(speed));
    GetField(sentence,8,heading,sizeof(heading));
    GetField(sentence,9,date,sizeof(date));

    strcpy(gps->status,status);

    FormatTime(utc,gps->utcTime);
    FormatDate(date,gps->date);

    if(strlen(latitude))
    {
        gps->latitude = ConvertCoordinate(latitude);

        if(ns[0]=='S')
            gps->latitude *= -1;
    }

    if(strlen(longitude))
    {
        gps->longitude = ConvertCoordinate(longitude);

        if(ew[0]=='W')
            gps->longitude *= -1;
    }

    if(strlen(speed))
    {
        gps->speed = atof(speed) * 1.852f;
    }

    if(strlen(heading))
    {
        gps->heading = atof(heading);
    }
}
