
#include <Ethernet.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"


// assign a MAC address for the Ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
// assign an IP address for the controller:
IPAddress ip(192, 168, 100, 177);
IPAddress myDns(192,168,100, 1);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_60X);
int pinLed = 31;
bool dataReady = true;

const int LED_RED = 5;
const int LED_GREEN = 6;
const int LED_BLUE = 7;

int sensedColors[4] = {0, 0, 0, 0};
int ledColors[3] = {0, 0, 0};
long lastReadingTime = 0;
uint16_t r, g, b, cl, colorTemp, lux;

void setup() {
  // reset LED
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  analogWrite(LED_RED, 0);
  analogWrite(LED_GREEN, 0);
  analogWrite(LED_BLUE, 0);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  server.begin();

  Serial.begin(9600);
  Serial.println("Hello!");

  // give the sensor and Ethernet shield time to set up:
  delay(1000);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }
}

void loop() {
  // check for a reading no more than once every 10 seconds
  if (millis() - lastReadingTime > 10000) {
    // if there's a reading ready, read it:
    // don't do anything until the data ready pin is high:
    if (true) {
      getData();
      // timestamp the last time you got a reading:
      lastReadingTime = millis();
      dataReady = false;
    }
  }

  // listen for incoming Ethernet connections:
  listenForEthernetClients();
}

void getData() {
    digitalWrite(pinLed, HIGH);
    showRgb();
    tcs.getRawData(&r, &g, &b, &cl);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
    digitalWrite(pinLed, LOW);

    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(cl, DEC); Serial.print(" ");
    Serial.println(" ");

    sensedColors[0] = r;
    sensedColors[1] = g;
    sensedColors[2] = b;
    sensedColors[3] = cl;
    convertColors();
    
    analogWrite(LED_RED, ledColors[0]);
    Serial.print("rLed: "); Serial.println(ledColors[0]);
    analogWrite(LED_GREEN, ledColors[1]);
    Serial.print("gLed: "); Serial.println(ledColors[1]);
    analogWrite(LED_BLUE, ledColors[2]);
    Serial.print("bLed: "); Serial.println(ledColors[2]);
    dataReady = true;
    // delay(2000);
}

void listenForEthernetClients() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Got a client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          // print the current readings, in HTML format:
          client.print("Clear light: ");
          client.print(String(cl));
          client.print(" .. ");
          client.println("<br />");
          client.print("Color temperature: " + String(colorTemp));
          client.print(" K");
          client.println("<br />");
          client.print("R: " + String(r));
          client.println("<br />");
          client.print("G: " + String(g));
          client.println("<br />");
          client.print("B: " + String(b));
          client.println("<br />");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

void convertColors() {
  int min = sensedColors[0];
  int max = sensedColors[0];
  for (int mm = 0; mm < 3; mm++) {
    if (sensedColors[mm] < min) {
      min = sensedColors[mm];
    }
    if (sensedColors[mm] > max) {
      max = sensedColors[mm];
    }
  }
  float scaleFactor = sensedColors[3] * 3 / 255;
  for (int nn = 0; nn < 3; nn++) {
    ledColors[nn] = (sensedColors[nn] - min*0.95) / scaleFactor;
  }
  return;  
}

void showRgb(){
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
    delay(250);
    analogWrite(LED_RED, 128);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
    delay(250);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 128);
    analogWrite(LED_BLUE, 0);
    delay(250);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 128);
    delay(250);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
    delay(250);
}

