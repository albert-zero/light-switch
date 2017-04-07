
In the current sketch the ESP8266 connects to the WIFI
    <TheNetwork> <ThePassword>
 
It opens a HTTP connection with following fixed IP address:
        http://192.168.178.88:80

    
The server accepts input for GET and POST method using XMLHttpRequest
        var xRequest = new XMLHttpRequest();

        /* The following calls digitalWrite(2, 1) */
        xRequest.open("GET", "http://192.168.178.88/gpio/1", true);
        xRequest.send();

        /* The following calls analogWrite(2, 100) */
        var xJsonReq = {pin: 6, value: 100};
        xRequest.open("POST", "http://192.168.178.88/gpio", true);
        xRequest.send(JSON.stringify(xJsonReq));

        
The response is a JSON string, that could be parsed in method callback onReadyStateChanged:
        var Response; 
        xRequest.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
                Response = JSON.parse(this.response);
            }
        };

The response contains a return structure, which could be used to evaluate error:
        { "return" : {"code": 200, "value": "OK"}
        }
		
		200 - OK
		404 - not found
		500 - server error

To make things running change the following lines 
        const char* ssid     = "TheNetwork";
        const char* password = "ThePassword";

        // Create an instance of the server
        // specify the port to listen on as an argument
        WiFiServer server(80);
        IPAddress  aIp (192, 168, 178, 88); 
        IPAddress  aMs (255, 255, 255, 0);


Next steps:
    Create a configuration file on 8266 
    Create a command line interface to serial port to change the connection values
    Read connection data from configuration file

    
    