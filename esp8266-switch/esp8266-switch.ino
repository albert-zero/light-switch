/* -----------------------------------------------------------------
 *  Author: Albert Zedlitz
 *  
 *  Description;
 *  The server will set a GPIO pin depending on the request
 *    http://192.168.178.88/gpio
 *    POST a json string
 *    {"pin": <number>, "value": <analog value 0..255>}
 *    
 *  This first approach uses a fixed ip address. 
 *  An interface to the serial port for configuration would be possible
 *  using the file system of ESP8266
 */

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <math.h>

const char* ssid     = "UntereMuehle";
const char* password = "74285140323410393146";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
IPAddress  aIp (192, 168, 178, 88); 
IPAddress  aMs (255, 255, 255, 0);
WiFiClient xClient;

StaticJsonBuffer<200> xJsonBuffer;
JsonObject           *xReturn;

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
void setup() {
    Serial.begin(115200);
  
    // Connect to WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);

    //WiFi.softAPConfig(ip, ip, ip);
    WiFi.config(aIp, aIp, aMs);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
       delay(100);
       Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
  
    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.println(WiFi.localIP());

    pinMode( 2, OUTPUT);
    pinMode( 4, OUTPUT);
    pinMode( 5, OUTPUT);
    pinMode(16, OUTPUT);

    xReturn = &xJsonBuffer.createObject();
}

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
void loop() {
    // Check if a client has connected
    int    xLoopCnt   = 0;

    // Wait until the client sends some data
    Serial.println("new client request");
    while(!xClient.available()){
        xClient = server.available();
        delay(5);        
    }
    Serial.println("start request loop");
    // Read the request, which is either GET or POST:
    // GET : Read one line 
    // POST: Read all lines until empty line \r\n 
    String xHeader;
    String xBody;
    int    xPinNr     = 2;
    int    xValue     = 0;
    int    xAvailable = 0;
    char   xBuffer[1024];

    xLoopCnt   = 0;
    
    (*xReturn)["code"]   = 200;
    (*xReturn)["value"]  = "OK";

    xClient.setTimeout(100); 
    if (!xClient.connected()) {
        Serial.println();
        Serial.println("disconnecting from server.");
        xClient.stop();
    }
    
    // Read first line
    xHeader = xClient.readStringUntil('\r');

    // Read the HTTP header for POST requests
    if (xHeader.indexOf("POST") != -1) {
        xBody = xClient.readStringUntil('\n');
        while (xLoopCnt == 0 || (xBody.length() > 4 && xLoopCnt < 20)) {
            xLoopCnt++;
            xBody = xClient.readStringUntil('\n');
            Serial.print(xBody.length() );
            Serial.println(":header:" + xBody);
        }
        xBody = xClient.readString();
        Serial.println("body:" + xBody);

        if (xBody.length() == 0 || !parseUserData(xBody)) {
            (*xReturn)["code"]  = 500;
            (*xReturn)["value"] = "Invalid input";      
        }
    }
    Serial.println("process body ....");
    
    // Read the HTTP header for GET requests
    // And switch the LED on board. This is for connection tests
    if (xHeader.indexOf("GET") != -1) {
        if (xHeader.indexOf("/gpio/0") != -1) {
            digitalWrite(2, 0);
        }
        else if (xHeader.indexOf("/gpio/1") != -1) {
            digitalWrite(2, 1);
        }
        else {
            (*xReturn)["code"]  = 404;      
            (*xReturn)["value"] = "Unknown request";      
        }
    }
    
    //writeResponse(xClient, xReturn);
    //xClient.flush();
    //delay(1);
    //xClient.stop();
    Serial.println("send response ....");
}

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
bool parseUserData(String& xContent) {
    long xPinNr = 2;
    long xValue = 0;
    char xCharBuf[128];

    StaticJsonBuffer<200> xJsonBuffer;
  
    xContent.toCharArray(xCharBuf, 128);
    JsonObject& xRoot = xJsonBuffer.parseObject(xCharBuf);

    if (!xRoot.success()) {
        Serial.println("JSON parsing failed! ");
        Serial.println("chars:" + xContent);
        return false;
    }

    if (xRoot.containsKey("pin")) {
        xPinNr = xRoot["pin"].as<long>();
    }
    
    if (xRoot.containsKey("value")) {
        xValue = xRoot["value"].as<long>();
    }
    analogWrite(xPinNr, xValue);
    return true;
}

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
void writeResponse(WiFiClient& xClient, JsonObject *xJson) {
    String xResponse;
    xResponse = "{\"code\":\"200\", \"value\":\"switch propagated\"}";
        
    xClient.flush();
    xClient.println("HTTP/1.1 200 OK");
    xClient.println("Content-Type: application/json");
    xClient.println("Connection: keep-alive");
    xClient.println("Access-Control-Allow-Origin:*");
    xClient.print  ("Content-Length:"); xClient.println(xResponse.length());
    xClient.println();

    //xJson.printTo(xClient);
    xClient.println(xResponse);
    xClient.flush();
    delay(4);
}

