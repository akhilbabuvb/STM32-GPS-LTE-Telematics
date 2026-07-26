#ifndef GPS_H
#define GPS_H

typedef struct
{
    char utcTime[20];
    char formattedTime[20];

    char date[20];
    char formattedDate[20];

    char status[5];

    float latitude;
    float longitude;

    float speed;
    float heading;

    float totalDistance;

} GPS_Data_t;

void GPS_Init(void);
void GPS_Task(void);
void GPS_UARTCallback(void);

#endif
