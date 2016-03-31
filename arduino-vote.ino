/***********************************************************************************************
 **
 ***********************************************************************************************/
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "Settings.h"

/***********************************************************************************************
 **
 ***********************************************************************************************/
#define ENABLE_WIFI 1


/***********************************************************************************************
 **
 ***********************************************************************************************/
#define NEOPIXEL_PIN D1
#define SERVO_PIN D8

/***********************************************************************************************
 ** Log timing
 ***********************************************************************************************/
// Time between log calls (ms)
const unsigned long logTimeInterval = 100000;

// Time of last log call (ms)
volatile unsigned long logTimeLast = 0;

// Time between the two latest log calls (ms)
volatile unsigned int logTimeDelta = 0;

/***********************************************************************************************
 **
 ***********************************************************************************************/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ400);

WiFiClient client;
Servo servo;

/***********************************************************************************************
 ** NEOPixels colors
 ***********************************************************************************************/
const uint32_t COLOR_DEFAULT = pixels.Color(255, 255, 243); //primary
const uint32_t COLOR_PRINARY = pixels.Color(0, 0, 255); //primary
const uint32_t COLOR_SUCCESS = pixels.Color(0, 255, 0); //success
const uint32_t COLOR_INFO = pixels.Color(0, 255, 255); //info
const uint32_t COLOR_WARNING = pixels.Color(255, 255, 0); //warning
const uint32_t COLOR_DANGER = pixels.Color(255, 0, 0); //danger
const uint32_t COLOR_OFF = pixels.Color(0, 0, 0); //danger


int pos = 0;    // variable to store the servo position

/***********************************************************************************************
 **
 ***********************************************************************************************/
void setup() {
  Serial.begin(115200);
  delay(1000);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.println("/***********************************************************************************************");
  Serial.println(" ** Monitor v0.0.1");
  Serial.println(" ***********************************************************************************************/");


  Serial.print("Status light");
  pixels.begin();
  pixels.setBrightness(100);

  pixels.setPixelColor(0, COLOR_DEFAULT);
  pixels.setPixelColor(1, COLOR_DEFAULT);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_PRINARY);
  pixels.setPixelColor(1, COLOR_PRINARY);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_SUCCESS);
  pixels.setPixelColor(1, COLOR_SUCCESS);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_INFO);
  pixels.setPixelColor(1, COLOR_INFO);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_WARNING);
  pixels.setPixelColor(1, COLOR_WARNING);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_DANGER);
  pixels.setPixelColor(1, COLOR_DANGER);
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, COLOR_OFF);
  pixels.setPixelColor(1, COLOR_OFF);
  pixels.show();

  Serial.println(" OK");

  
  Serial.print("Starting servo");
  servo.attach(SERVO_PIN);  // attaches the servo on GIO2 to the servo object
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(10);                       // waits 15ms for the servo to reach the position
  }
  Serial.println(" OK");

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  pixels.setPixelColor(0, COLOR_WARNING);
  pixels.setPixelColor(1, COLOR_WARNING);
  pixels.show();
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    pixels.setPixelColor(0, COLOR_WARNING);
    pixels.setPixelColor(1, COLOR_OFF);
    pixels.show();
    delay(250);
    pixels.setPixelColor(0, COLOR_OFF);
    pixels.setPixelColor(1, COLOR_WARNING);
    pixels.show();
    delay(250);
  }

  pixels.setPixelColor(0, COLOR_SUCCESS);
  pixels.setPixelColor(1, COLOR_SUCCESS);
  pixels.show();
  delay(1000);

  Serial.println("");
  Serial.println("  WiFi connected");
  Serial.print("  IP address: ");
  Serial.println(WiFi.localIP());

  pixels.setPixelColor(0, COLOR_SUCCESS);
  pixels.setPixelColor(1, COLOR_SUCCESS);
  pixels.show();
  delay(1000);

  pixels.setPixelColor(0, COLOR_OFF);
  pixels.setPixelColor(1, COLOR_OFF);
  pixels.show();
  delay(500);

  pixels.setPixelColor(0, COLOR_SUCCESS);
  pixels.setPixelColor(1, COLOR_SUCCESS);
  pixels.show();
  delay(1000);

  pixels.setPixelColor(0, COLOR_OFF);
  pixels.setPixelColor(1, COLOR_OFF);
  pixels.show();

  Serial.println("##########################");
}




// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(host, 80)) {
    Serial.println("    connecting...");
    // send the HTTP GET request:
    client.println(String("GET ") + endpoint + " HTTP/1.1");
    client.println("Host: iotdemo.langeland.info");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    Serial.println("    request sent");
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("    headers received");
        break;
      }
    }
    String line = client.readStringUntil('\n');

    Serial.println("    reply was:");
    Serial.println("    ==========");
    Serial.println(String("    ") + line);
    Serial.println("    ==========");
    Serial.println("  closing connection");
    Serial.println("");

    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& result = jsonBuffer.parseObject(line);

    if (!result.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    int servoPosRaw = result["result"];
    int servoPosMap = map(servoPosRaw, 0, 10, 1, 180);
    servo.write(servoPosMap);
    delay(250);

    Serial.print("Set servo: ");
    Serial.println(servoPosMap);
    // note the time that the connection was made:
    logTimeLast = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void loop() {
  Serial.println("  - Get status");
  pixels.setPixelColor(0, COLOR_PRINARY);
  pixels.setPixelColor(1, COLOR_PRINARY);
  pixels.show();
  httpRequest();
  pixels.setPixelColor(0, COLOR_SUCCESS);
  pixels.setPixelColor(1, COLOR_SUCCESS);
  pixels.show();
  delay(1000);
}
