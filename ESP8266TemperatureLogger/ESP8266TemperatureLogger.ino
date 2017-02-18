#include <SparkFun_Si7021_Breakout_Library.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// ==============================================================
// CONSTANTS - change these before uploading to ESP8266
// ==============================================================
const char* wifiSsid      = "YOUR_WIFI_SSID";
const char* wifiPassword  = "YOUR_WIFI_PSK";

const char* matrixHomeserver    = "YOUR_HS"; // do not include trailing slash or "https://" parts
const char* matrixAccessToken   = "YOUR_AT";
const char* matrixRoomId        = "YOUR_ROOM";

const bool  useHttps            = true;
const char* certFingerprint     = "YOUR_CERT_FINGERPRINT";
const int   httpsPort           = 443;

const long  wifiConnectTimeout  = 30000;
const bool  useLedIfFailedConn  = true;
// --------------------------------------------------------------

// ==============================================================
// GLOBAL VARIABLES - things used by the entire application
// ==============================================================
Weather sensor;
HTTPClient http;

float humidity = 0;
float tempc = 0;
float tempf = 0;
// --------------------------------------------------------------

// ==============================================================
// FUNCTIONS
// ==============================================================
void readWeather() {
  // Always read humidity first - it automatically loads in the temperature data, making the call
  // slightly faster
  humidity = sensor.getRH();
  tempc = sensor.getTemp();
  tempf = sensor.getTempF();
}

void createEventBody(char* buffer, int bufferLen, float tempc, float tempf, float humidity) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  root["temp_c"] = tempc;
  root["temp_f"] = tempf;
  root["rel_humidity"] = humidity;

  root.printTo(buffer, bufferLen);
}

void doSecurePut(char* data) {
  WiFiClientSecure https;

  if (!https.connect(matrixHomeserver, httpsPort)) {
    Serial.println("HTTP - Failed to connect");
    return;
  }

  if (https.verify(certFingerprint, matrixHomeserver)) {
    Serial.println("HTTP - Certificate OK");
  } else {
    Serial.println("HTTP - Certificate doesn't match (continuing anyway)");
  }

  String url = "/_matrix/client/r0/rooms/" + String(matrixRoomId) + "/send/io.t2l.matrix.weather/e" + String(millis()) + "?access_token=" + String(matrixAccessToken);
  https.print("PUT " + url + " HTTP/1.1\r\n");
  https.print("Host: " + String(matrixHomeserver) + ":" + String(httpsPort) + "\r\n");
  https.print("Connection: close\r\n");
  https.print("Content-Type: application/json\r\n");
  https.print("Content-Length: " + String(String(data).length()) + "\r\n");
  https.print("\r\n");
  https.print(String(data));

  Serial.println("HTTP - Listening for response");
  delay(100);

  // Need to read the first line to ensure that we get a correct available() return
  String line = https.readStringUntil('\n');
  Serial.print(line);

  while (https.connected() || https.available() > 0) {
    while (https.available() > 0) {
      Serial.print(char(https.read()));
    }
  }

  Serial.println("\nDone processing");
}

void postMatrixEvent() {
  char buffer[512];
  createEventBody(buffer, 512, tempc, tempf, humidity);

  if (useHttps) {
    doSecurePut(buffer);
    return;
  }

  String scheme = "http://";
  if (useHttps) {
    scheme = "https://";
  }
  String url = scheme + String(matrixHomeserver) + "/_matrix/client/r0/rooms/" + String(matrixRoomId) + "/send/io.t2l.matrix.weather/e" + String(millis()) + "?access_token=" + String(matrixAccessToken);
  Serial.println("PUT - " + url);
  Serial.println(buffer);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int rc = http.sendRequest("PUT", buffer);
  Serial.print("HTTP ");
  Serial.println(rc);

  String body = http.getString();
  Serial.println(body);
}

void publishWeather() {
  Serial.print("Temp ");
  Serial.print(tempc);
  Serial.print("C (");
  Serial.print(tempf);
  Serial.print("F), Humidity ");
  Serial.print(humidity);
  Serial.println("%");

  if (WiFi.status() != WL_CONNECTED) {
    return; // can't send data, so don't
  }

  postMatrixEvent();
}

void connectWifi() {
  WiFi.disconnect();

  Serial.print("Connecting to wifi: ");
  Serial.println(wifiSsid);
  
  WiFi.begin(wifiSsid, wifiPassword);
  
  long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifiConnectTimeout) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WARNING - Was not able to connect to Wifi");

    if (useLedIfFailedConn) {
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, LOW); // actually on
    }
  } else {
    Serial.println("Connected to wifi");
    if (useLedIfFailedConn) {
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, HIGH); // actually off
    }
  }
}
// --------------------------------------------------------------

// ==============================================================
// SETUP & LOOP
// ==============================================================
void setup() {
  Serial.begin(9600);

  sensor.begin(); // set up SI7021
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WARNING - Wifi not connected");
    connectWifi();
  }
  
  readWeather();
  publishWeather();

  delay(1000);
}
// --------------------------------------------------------------
