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

const char* ssid     = "TheNetwork";
const char* password = "ThePassword";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
IPAddress  aIp (192, 168, 178, 88); 
IPAddress  aMs (255, 255, 255, 0);

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

  pinMode(2, OUTPUT);
}

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
void loop() {
  // Check if a client has connected
  WiFiClient xClient = server.available();
  if (!xClient) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client request");
  while(!xClient.available()){
    delay(1);
  }
  
  // Read the request, which is either GET or POST:
  // GET : Read one line 
  // POST: Read all lines until empty line \r\n 
  String xHeader;
  String xBody;
  int    xPinNr = 2;
  int    xValue = 0;
  
  StaticJsonBuffer<200> xJsonBuffer;
  JsonObject& xReturn = xJsonBuffer.createObject();

  xReturn["code"]  = 200;
  xReturn["value"] = "OK";
    
  xHeader = xClient.readStringUntil('\n');

  // Check if the right method is called
  if (xHeader.indexOf("/gpio") == -1) {
    Serial.println(xHeader);
    xReturn["code"]  = 500;
    xReturn["value"] = "invalid request";

    writeResponse(xClient, xReturn);
    return;
  }

  if (xHeader.indexOf("POST") != -1) {
    // Handle POST request
    while (xHeader.length() > 2) {
      Serial.println(xHeader);
      xHeader = xClient.readStringUntil('\n');
    }
    xBody = xClient.readStringUntil('\n');
    Serial.println(xBody);

    if (!parseUserData(xBody)) {
      xReturn["code"]  = 500;
      xReturn["value"] = "Invalid input";      
    }
  }
  else if (xHeader.indexOf("GET") != -1) {
    // Handle GET request
    Serial.println(xHeader);
    
    if (xHeader.indexOf("/gpio/0") != -1) {
      digitalWrite(2, 0);
    }
    else if (xHeader.indexOf("/gpio/1") != -1) {
      digitalWrite(2, 1);
    }
    else {
      xReturn["code"]  = 500;      
      xReturn["value"] = "Unknown request";      
    }
  }  
  else {
    xReturn["code"]  = 500;      
    xReturn["value"] = "Unknown method";      
  }

  writeResponse(xClient, xReturn);
  Serial.println("Client disonnected");
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
    Serial.println("JSON parsing failed!");
    return false;
  }

  if (xRoot.containsKey("pin")) {
    xPinNr = xRoot["pin"].as<long>();
  }
  if (xRoot.containsKey("value")) {
    xValue = xRoot["value"].as<long>();
  }
  pinMode(xPinNr, OUTPUT);
  analogWrite(xPinNr, xValue);

  return true;
}

/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */
void writeResponse(WiFiClient& xClient, JsonObject& xJson) {
  xClient.println("HTTP/1.1 200 OK");
  xClient.println("Content-Type: application/json");
  xClient.println("Connection: close");
  xClient.println("Access-Control-Allow-Origin:*");
  xClient.println();

  xJson.printTo(xClient);
  xClient.flush();
  delay(1);
}

