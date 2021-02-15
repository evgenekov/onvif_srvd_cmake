
#include "mosquitto_hander.h"
#include "smacros.h"


void init_mosquitto(struct mosquitto *mosq)
{
    // Mosquitto stuff

    mosquitto_lib_init();
    
    mosq = mosquitto_new("onvif_srvd", true, NULL);
    
    if(!mosq)
    {
        DEBUG_MSG("Unable to initialise mosquitto\n");
        mosquitto_lib_cleanup();
    }
    
    //mosquitto_publish_callback_set(mosq, on_publish);
    mosquitto_connect(mosq, "localhost", 1883, 60);

    const char* msg = "This is simply a test";
    size_t size = strlen(msg);
    
    publish_to_watchman(mosq, size, msg);
    
    //mosquitto_disconnect(mosq);

}

int publish_to_watchman(struct mosquitto *mosq_, int payloadLen, const char* payload)
{
#ifdef  DEBUG
    DEBUG_MSG("\nSend message to test:\npayloadLen: %d\npayload: %s\n", payloadLen, payload);
    int errorNo = mosquitto_publish(mosq_, NULL, "Test", payloadLen, payload, 0, false);
    
    DEBUG_MSG("ErrorNo: %d\n", errorNo);
#endif
    
    return mosquitto_publish(mosq_, NULL, "watchman_command_json", payloadLen, payload, 0, false);
}

    

void on_publish()
{
    fprintf(stderr, "Sent Message");
}
