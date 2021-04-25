#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "index.h"          //index html
#include "bitmaps.h"          //bitmaps
#include "device_status.h"  //device status

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

String client_ssid = "";
String client_password = "";

const String prefix_ssid = "CoinTT";  //Enter Soft AP prefix SSID here
const String password = "12345678";  //Enter Soft AP password here

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

int connectErrorCount = 0; 

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");

  String clientMac = "";
unsigned char mac[6];
WiFi.macAddress(mac);
clientMac += macToStr(mac);
clientMac.replace(":","");
clientMac.toUpperCase();
const String ssid_mac_substr = prefix_ssid + "_" + clientMac.substring(0,6);

  // initialize with the I2C addr 0x3C
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

// Clear the buffer.
	display.clearDisplay();
  drawInitialScreenBitmap();
  delay(2000);

  WiFi.disconnect();
  WiFi.softAP(ssid_mac_substr.c_str(), password.c_str());
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  device_status = "WAITING_FOR_WIFI_CONNECTION";

  server.on("/", send_html_main_page);
  server.on("/submit_data", handle_submit_data);
  server.on("/status", handle_fetch_status);
  server.onNotFound(handle_NotFound);
  server.begin();

  Serial.println("HTTP server started");
  Serial.print( clientMac);
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
        connectErrorCount = 0;
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        show_one_line_text(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        connectErrorCount++;
      }
      // Free resources
      http.end();
    } else {
      Serial.println("WiFi not connected");

      if (!client_ssid.equals("")) {
        Serial.println("Trying to reconnect to wifi");
        connect_to_wifi();
      } else {
        show_need_configuration_text();
        Serial.println("No SSID is defined, not trying to connect to wifi");
      }
    }
    lastTime = millis();
  }
}

void connect_to_wifi() {
  show_one_line_text("Connecting to WIFI...");
  device_status = CONNECTING_STATUS;
  WiFi.begin(client_ssid.c_str(), client_password.c_str());
  for (int x = 0; x < 20; x = x++) {
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      device_status = WIFI_CONNECTED_STATUS;
      show_one_line_text("Connection Established!!");
      Serial.print("Connected to WiFi network with IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }
  }
  WiFi.disconnect();
  show_one_line_text("WIFI connection Failed.");
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

void show_need_configuration_text(){
  display.clearDisplay();

const String  text1 = "Please follow the";
const String  text2 = "instructions to";
const String  text3 = "configure the device.";
  int textPosition1 = (SCREEN_WIDTH - text1.length()*6)/2;
  int textPosition2 = (SCREEN_WIDTH - text2.length()*6)/2;
  int textPosition3 = (SCREEN_WIDTH - text3.length()*6)/2;

// Display Text
	display.setTextSize(1);
	display.setTextColor(WHITE);

	display.setCursor(textPosition1,15);
	display.println(text1.c_str());

  display.setCursor(textPosition2,30);
	display.println(text2.c_str());

  display.setCursor(textPosition3,45);
	display.println(text3.c_str());
	
  display.display();  
}

void show_one_line_text(String text){
  display.clearDisplay();
  int textPosition = (SCREEN_WIDTH - text.length()*6)/2;
// Display Text
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(textPosition,32);
	display.println(text.c_str());
  display.display();  
  delay(2000)
}

void drawInitialScreenBitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - 128 ) / 2,
    (display.height() - 64) / 2,
    initialScreenBitmap, 128, 64, 1);
  display.display();
  delay(1000);
}

String macToStr(const uint8_t* mac)
{
String result;
for (int i = 0; i < 6; ++i) {
result += String(mac[i], 16);
if (i < 5)
result += ':';
}
return result;
}