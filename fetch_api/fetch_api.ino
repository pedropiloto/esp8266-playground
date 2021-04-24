#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include "index.h"          //index html
#include "device_status.h"  //device status

const char* ssid = "NodeMCU";       // Enter SSID here
const char* password = "12345678";  //Enter Password here
/* IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

String client_ssid = "";
String client_password = "";

String device_status = STARTING_STATUS;

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
  Serial.println("Setup");

  WiFi.disconnect();

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  device_status = "WAITING_FOR_WIFI_CONNECTION";

  server.on("/", send_html_main_page);
  server.on("/submit_data", handle_submit_data);
  server.on("/status", handle_fetch_status);
  server.onNotFound(handle_NotFound);

  server.begin();

  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      device_status = WIFI_CONNECTED_STATUS;
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());

      HTTPClient http;

      // Your Domain name with URL path or IP address with path
      Serial.println(serverName);
      Serial.println(serverName.c_str());
      http.begin(serverName.c_str());

      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi not connected");
      if (!client_ssid.equals("")) {
        connect_to_wifi();
        Serial.println("Trying to reconnect to wifi");
      } else {
        Serial.println("No SSID is defined, not trying to connect to wifi");
      }
    }
    lastTime = millis();
  }
}

void connect_to_wifi() {
  device_status = CONNECTING_STATUS;
  WiFi.begin(client_ssid.c_str(), client_password.c_str());
  for (int x = 0; x < 20; x = x++) {
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      device_status = WIFI_CONNECTED_STATUS;
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }
  }
  WiFi.disconnect();
  device_status = CONNECTION_FAILED_STATUS;
}

void handle_submit_data() {
  Serial.println("handle submiting data");

  String message = "Body received:\n";
  message += server.arg("ssid_data");
  message += "\n";
  message += server.arg("pass_data");
  message += "\n";

  Serial.println(message);

  client_ssid = server.arg("ssid_data");
  client_password = server.arg("pass_data");

  if (!client_ssid.equals("")) {
    connect_to_wifi();
    Serial.println("Connecting");
  } else {
    Serial.println("No SSID was provided. Not connecting");
  }
}

void handle_fetch_status() {
  server.send(200, "text/plane", device_status);
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void send_html_main_page() {
  Serial.println("serving HTML");
  server.send(200, "text/html", main_page_html);
}