/* My own UDP Demo */

#include "esp_common.h"
#include "espconn.h"
#include "Router.h"

// Need SSID and PASSWORD
// #define SSID "xxxx"
// #define PASSWORD "xxxx"

const char *Start_Send_to_Server = "Hello, This is xxx Client!";

const char SERVER_IP[4] = {192, 168, 1, 184};
const int  SERVER_PORT = 1205;


LOCAL struct espconn ptrespconn;


/******************************************************************************
 * FunctionName : user_udp_sent_cb
 * Description  : Data has been sent successfully and acknowledged by the remote host.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void 
user_udp_sent_cb(void *arg)
{
    struct espconn *pespconn = arg;
    printf("user_udp_sent_cb\n");
}

/******************************************************************************
 * FunctionName : user_udp_sent
 * Description  : Processing the application data and sending it to the host
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void
user_udp_sent(struct espconn *pespconn)
{
    int res;

    printf("user_udp_sent function\n");
    unsigned short length;
    unsigned short packet_size = 1024;
    char *pbuf = (char *)malloc(packet_size);
    memset(pbuf, 0, packet_size);

    length=strlen(Start_Send_to_Server);
    memcpy(pbuf, Start_Send_to_Server, length);

    while((res = espconn_sent(pespconn, pbuf, strlen(pbuf))) != 0)
    {
        printf("ESP send error!\n");
        vTaskDelay(1000 / portTICK_RATE_MS); 
    }

    printf("ESP send successfully!\n");
    // vTaskDelay(200 / portTICK_RATE_MS);
    // res = espconn_sent(pespconn, pbuf, strlen(pbuf));
    // if(res != 0)
    //     printf("ESP send error!\n"); 
    // else
    //     printf("ESP send successfully!\n");
    
    free(pbuf);
}   

LOCAL void
user_udp_recv(void *arg, char *pdata, unsigned short len)
{
    struct espconn *pespconn = (struct espconn*)arg;
    unsigned short i;

    printf("Receive from server!\n");
    printf("len: %d\n", len);
    for(i=0; i<len; i++)
        printf("%c", pdata[i]);
    printf("\n");

    espconn_sent(pespconn, pdata, len);
}


void user_udp_task(void* pvParameters)
{
    int wifi_status;
    (void*) pvParameters;

    ptrespconn.type = ESPCONN_UDP;
    ptrespconn.proto.udp = (esp_udp *)malloc(sizeof(esp_udp));               
    memset(ptrespconn.proto.udp, 0, sizeof(esp_udp));
    ptrespconn.proto.udp->remote_port = SERVER_PORT;                            // Remote Server Port
    memcpy(ptrespconn.proto.udp->remote_ip, SERVER_IP, 4);                      // Remote Server IP
    ptrespconn.proto.udp->local_port = espconn_port();                          // ESP8266 udp port
    printf("local_port: %d\n", ptrespconn.proto.udp->local_port);

    espconn_create(&ptrespconn);                                                // create udp

    // espconn_regist_sentcb(&ptrespconn, user_udp_sent_cb);                       // register a udp packet sending callback
    espconn_regist_recvcb(&ptrespconn, user_udp_recv);                          // register a udp packet recv callback

    while((wifi_status = wifi_station_get_connect_status()) !=  STATION_GOT_IP)
    {
        printf("wifi_status: %d\n", wifi_status);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    printf("wifi_status: %d\n", wifi_status);
    user_udp_sent(&ptrespconn);                                                 // sent data

    while(1)
    {
        taskYIELD();
        // vTaskDelay(200 / portTICK_RATE_MS);
    }
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());

    wifi_set_opmode(STATIONAP_MODE);                                //Set softAP + station mode

    struct station_config stationConf;                              //Set ap settings

    memset(&stationConf, 0, sizeof(stationConf));
    memcpy(&stationConf.ssid, SSID, sizeof(SSID));
    memcpy(&stationConf.password, PASSWORD, sizeof(PASSWORD));
    wifi_station_set_config(&stationConf);

    os_delay_us(50000);                                             //Max 65535 µs
    printf("start from here\r\n");

    espconn_init();
    xTaskCreate(user_udp_task, "UDP Task Test", 512, NULL, 1, NULL);
    // user_udp_init();                                                // Create udp listening.

}
