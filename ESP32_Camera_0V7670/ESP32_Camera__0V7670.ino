
//*********************** Bibliotecas incluidas *************************************

#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "canvas_htm.h"
#include "OV7670.h"

//*************************** para usar o led da placa *****************************

#define ONBOARD_LED  2

//************************** pinout camera *****************************************

#define SIOD 21 //SDA
#define SIOC 22 //SCL

#define VSYNC 34
#define HREF 35

#define XCLK 32
#define PCLK 33

#define D0 27
#define D1 17 //TX2
#define D2 16 //RX2
#define D3 15
#define D4 14
#define D5 13
#define D6 12
#define D7 4

// pino reset da camera vai no enable do esp

//*************************** para usar o led da placa *****************************

#define ONBOARD_LED  2

//*********************** definições wifi *************************

#define ssidAP "ESP32_AP"
#define passwordAP "123456789"
#define ssidWN "Your internet name"
#define passwordWN "your internte password"

WiFiServer server(80);
unsigned char pix = 0;

//*********************** bmpHeader *************************
unsigned char start_flag = 0xAA;
unsigned char end_flag = 0xFF;
unsigned char ip_flag = 0x11;

//************************** definições camera ************************************

OV7670 *camera = nullptr;

//************************* Funções **********************************************

//************************** Websocket ************************************

WebSocketsServer webSocket(81);    // create a websocket server on port 81

void startWebSocket()
{ // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startWebServer()
{
  server.begin();
  Serial.println("Http web server started.");


}


//*************************** Serve *********************************************

void serve()
{
  WiFiClient client = server.available();
  if (client)
  {
    //Serial.println("New Client.");
    String currentLine = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(canvas_htm);
            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }

      }
    }
    // close the connection:
    client.stop();

  }
}

//************************* WiFI **********************************************

IPAddress ip(192,168,3,63); //COLOQUE UMA FAIXA DE IP DISPONÍVEL DO SEU ROTEADOR
IPAddress gateway(192,168,3,1); //GATEWAY DE CONEXÃO
IPAddress subnet(255,255,255,0); //MASCARA DE REDE

void initWifiStation()
{
  WiFi.mode(WIFI_AP_STA);

  Serial.println("\n[*] Creating ESP32 AP");
  WiFi.softAP(ssidAP, passwordAP);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  WiFi.begin(ssidWN, passwordWN);
  WiFi.config(ip, gateway, subnet); //PASSA OS PARÂMETROS PARA A FUNÇÃO QUE VAI SETAR O IP FIXO NO NODEMCU
  Serial.println("\n[*] Connecting to WiFi Network");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.print("\n[+] Connected to the WiFi network with local IP : ");
  Serial.println(WiFi.localIP());
  digitalWrite(ONBOARD_LED, HIGH);
}

//************************** WebSocket p.II ************************************************

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t payloadlength)
{ // When a WebSocket message is received

  int blk_count = 0;

  char canvas_VGA[] = "canvas-VGA";
  char canvas_Q_VGA[] = "canvas-Q-VGA";
  char canvas_QQ_VGA[] = "canvas-QQ-VGA";
  char canvas_QQQ_VGA[] = "canvas-QQQ-VGA";
  char ipaddr[26] ;
  IPAddress localip;

  switch (type)
  {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        webSocket.sendBIN(0, &ip_flag, 1);
        localip = WiFi.localIP();
        sprintf(ipaddr, "%d.%d.%d.%d", localip[0], localip[1], localip[2], localip[3]);
        webSocket.sendTXT(0, (const char *)ipaddr);

      }
      break;
    case WStype_TEXT:                     // if new text data is received
      if (payloadlength == sizeof(canvas_QQQ_VGA) - 1) {
        if (memcmp(canvas_QQQ_VGA, payload, payloadlength) == 0) {
          Serial.printf("canvas_QQQ_VGA");
          webSocket.sendBIN(0, &end_flag, 1);
          delete camera;
          camera = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } else if (payloadlength == sizeof(canvas_QQ_VGA) - 1) {
        if (memcmp(canvas_QQ_VGA, payload, payloadlength) == 0) {
          Serial.printf("canvas_QQ_VGA");
          webSocket.sendBIN(0, &end_flag, 1);
          delete camera;
          camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } else if (payloadlength == sizeof(canvas_Q_VGA) - 1) {
        if (memcmp(canvas_Q_VGA, payload, payloadlength) == 0) {
          Serial.printf("canvas_Q_VGA");
          webSocket.sendBIN(0, &end_flag, 1);
          delete camera;
          camera = new OV7670(OV7670::Mode::QVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      } if (payloadlength == sizeof(canvas_VGA) - 1) {
        if (memcmp(canvas_VGA, payload, payloadlength) == 0) {
          Serial.printf("canvas_VGA");
          webSocket.sendBIN(0, &end_flag, 1);
          delete camera;
          camera = new OV7670(OV7670::Mode::VGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
        }
      }

      blk_count = camera->yres / I2SCamera::blockSlice; //30, 60, 120
      for (int i = 0; i < blk_count; i++)
      {

        if (i == 0)
        {
          camera->startBlock = 1;
          camera->endBlock = I2SCamera::blockSlice;
          webSocket.sendBIN(0, &start_flag, 1);
        }

        if (i == blk_count - 1)
        {
          webSocket.sendBIN(0, &end_flag, 1);
        }

        camera->oneFrame();
        webSocket.sendBIN(0, camera->frame, camera->xres * I2SCamera::blockSlice * 2);
        camera->startBlock += I2SCamera::blockSlice;
        camera->endBlock   += I2SCamera::blockSlice;
      }

      break;
    case WStype_ERROR:                     // if new text data is received
      Serial.printf("Error \n");
    default:
      Serial.printf("WStype %x not handled \n", type);

  }
}




//************************** Setup ************************************************

void setup() {
  Serial.begin(115200);
  pinMode(ONBOARD_LED, OUTPUT);

  initWifiStation();
  startWebSocket();
  startWebServer();
}

//**************************** loop ***********************************************

void loop() {
  webSocket.loop();
  serve();
}

//**********************************************************************************
