/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here
/* IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

const char* client_ssid = "";
const char* client_password = "";

//Your Domain name with URL path or IP address with path
String serverName = "http://ticker-api.pedropiloto.com/ticker";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200); 
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.on("/", handle_OnConnect);
  server.on("/submit_data", handle_submit_data);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
 
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
      
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      Serial.println(serverName);
      Serial.println(serverName.c_str());
      http.begin(serverName.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi not connected");
       WiFi.begin(client_ssid, client_password);
      Serial.println("Connecting");

    }
    lastTime = millis();
  }
}

void handle_submit_data() {
  Serial.println("handle submiting data");
 
      String message = "Body received:\n";
             message += server.arg("ssid_data");
             message += "\n";
             message += server.arg("pass_data");
             message += "\n";
 
    server.send(200, "text/html", SendHTML(LED1status)); 
      Serial.println(message);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";

  ptr +="<form action=\"/submit_data\">";
  ptr += "<label for=\"ssid_data\">SSID:</label><br>";
  ptr += "<input type=\"text\" id=\"fname\" name=\"ssid_data\" value=\"\"><br>";
  ptr += "<label for=\"pass_data\">Last name:</label><br>";
  ptr += "<input type=\"password\" id=\"lname\" name=\"pass_data\" value=""><br><br>";
  ptr += "<input type=\"submit\" value=\"Submit\">";
ptr += "</form>";

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}