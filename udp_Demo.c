/* My own UDP Demo */

//说明：LOCAL 即 static

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
client_on_start_udp_send(struct espconn *pespconn)
{
    int res;

    printf("Client on start send!\n");
    unsigned short length  = strlen(Start_Send_to_Server);
    char *pbuf = (char *)malloc(length);
    if(pbuf == NULL)
    {
        printf("user_udp_sent: malloc pbuf(size: %d) error!\n", length);
        return;
    }

    length = strlen(Start_Send_to_Server);
    memcpy(pbuf, Start_Send_to_Server, length);

    while((res = espconn_sent(pespconn, pbuf, strlen(pbuf))) != 0)          //send error
    {
        printf("ESP send error!\n");
        vTaskDelay(1000 / portTICK_RATE_MS); 
    }

    printf("ESP send successfully!\n");
    
    free(pbuf);
}   

// LOCAL void
// user_udp_sent(struct espconn *pespconn)
// {
//     int res;

//     printf("Client start send!\n");
//     unsigned short length;
//     unsigned short packet_size = 1024;
//     char *pbuf = (char *)malloc(packet_size);
//     if(pbuf == NULL)
//     {
//         printf("user_udp_sent: malloc pbuf(size: %d) error!\n", packet_size);
//         return;
//     }
//     memset(pbuf, 0, packet_size);

//     length = strlen(Start_Send_to_Server);
//     memcpy(pbuf, Start_Send_to_Server, length);

//     while((res = espconn_sent(pespconn, pbuf, strlen(pbuf))) != 0)
//     {
//         printf("ESP send error!\n");
//         vTaskDelay(1000 / portTICK_RATE_MS); 
//     }

//     printf("ESP send successfully!\n");
    
//     free(pbuf);
// }   

/******************************************************************************
 * FunctionName : user_udp_recv
 * Description  : recv callback function (need espconn_regist_recvcb to register)
 * Parameters   : arg: esp连接；pdata：接收到的数据的头地址；len：数据长度
 * Returns      : none
*******************************************************************************/
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

    espconn_sent(pespconn, pdata, len);                     //原样返回给服务器
}

/******************************************************************************
 * FunctionName : user_udp_task
 * Description  : a udp task
 * Parameters   : pvParameters
 * Returns      : none
*******************************************************************************/
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
        switch(wifi_status)
        {
            case STATION_IDLE: printf("wifi_status: STATION_IDLE\n"); break;
            case STATION_CONNECTING: printf("wifi_status: ESP8266 station is connecting to AP\n"); break;
            case STATION_WRONG_PASSWORD: printf("wifi_status: the password is wrong\n"); break;
            case STATION_NO_AP_FOUND: printf("wifi_status: ESP8266 station can not find the target AP\n"); break;
            case STATION_CONNECT_FAIL: printf("wifi_status: ESP8266 station fail to connect to AP\n"); break;
            default: printf("Unknow wifi status!!!\n");
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    printf("wifi_status: %d\n", wifi_status);
    client_on_start_udp_send(&ptrespconn);                                                 

    vTaskDelete(NULL);
    // while(1)
    // {
    //     taskYIELD();
    //     // vTaskDelay(200 / portTICK_RATE_MS);
    // }
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

    wifi_set_opmode(STATION_MODE);                                  //终端模式

    struct station_config stationConf;                              //路由连接设置
    memset(&stationConf, 0, sizeof(stationConf));
    memcpy(&stationConf.ssid, SSID, sizeof(SSID));
    memcpy(&stationConf.password, PASSWORD, sizeof(PASSWORD));
    wifi_station_set_config(&stationConf);

    espconn_init();
    xTaskCreate(user_udp_task, "UDP Task Test", 512, NULL, 1, NULL);
}



