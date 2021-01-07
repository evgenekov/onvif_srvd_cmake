#ifndef MOSQUITTO_HANDLER
#define MOSQUITTO_HANDLER


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <mosquitto.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

// Macros/Constants


// Global Variables

// Function Definitions
void init_mosquitto(struct mosquitto *mosq);
uint8_t publish_to_watchman(struct mosquitto *mosq_, int payloadLen, const char* payload);
void on_publish();


#endif //MOSQUITTO_HANDLER

