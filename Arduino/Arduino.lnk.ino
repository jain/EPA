#include <Adafruit_VC0706.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <stdlib.h>
#include "utility/debug.h"
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <avr/wdt.h>

#define ADAFRUIT_CC3000_IRQ   2  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  7  // These can be any two pins
#define ADAFRUIT_CC3000_CS    53
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIV2); // you can change this clock speed

#define WLAN_SSID       "t1"//"Hua's iPhone"//"iPhone"//"TempSpot"//           // cannot be longer than 32 characters!
#define WLAN_PASS      "abcdefghij"//"123456789"  //"abcdefghij"//"12345678"//"asdfghjkl"//
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data
// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

int32_t ip;
// What page to grab!
//#define WEBSITE        "192.168.43.213"
#define WEBSITE      "epa4012.herokuapp.com"
#define WEBPAGE      "/"


#if ARDUINO >= 100
// On Uno: camera TX connected to pin 2, camera RX to pin 3:
//SoftwareSerial cameraconnection = SoftwareSerial(2, 3);
// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(69, 3);
#else
NewSoftSerial cameraconnection = NewSoftSerial(69, 3);
#endif

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);



#include <SD.h>                      // need to include the SD library
#define SD_ChipSelectPin 53  //using digital pin 4 on arduino nano 328 //example uses hardware SS pin 53 on Mega2560
#include <TMRpcm.h>           //  also need to include this library...



TMRpcm tmrpcm;   // create an object for use in this sketch




// Melody

int ledPin = 13;
int ledPin2 = 12;
int speakerOut = 11;

int counter = 0;
int count2 = 0;
int count3 = 0;
int MAX_COUNT = 2;
int statePin = LOW;
int statePin2 = LOW;


void setup() {
  //pinMode(wifiPower, OUTPUT);
  //pinMode(sdPower, OUTPUT);
  //digitalWrite(53, LOW);
  Serial.begin(115200);
  pinMode(53, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  wifi_setup();
  sendStatus();
}
void Beep() {
  digitalWrite(ledPin, !statePin);
  digitalWrite(ledPin2, !statePin2);
  analogWrite(speakerOut, 2500);
  delay(1000);
  digitalWrite(ledPin, statePin);
  digitalWrite(ledPin2, statePin2);
  analogWrite(speakerOut, 0);
  delay(1000);
  digitalWrite(ledPin, !statePin);
  digitalWrite(ledPin2, !statePin2);
  analogWrite(speakerOut, 1500);
  delay(1000);
  digitalWrite(ledPin, statePin);
  digitalWrite(ledPin2, statePin2);
  analogWrite(speakerOut, 0);
  delay(1000);
  digitalWrite(ledPin, !statePin);
  digitalWrite(ledPin2, !statePin2);
  analogWrite(speakerOut, 1500);
  delay(1000);
  digitalWrite(ledPin, statePin);
  digitalWrite(ledPin2, statePin2);
  analogWrite(speakerOut, 0);
  delay(1000);

}


void wifi_setup() {
#if !defined(SOFTWARE_SPI)
  if (ADAFRUIT_CC3000_CS != 53) {
    pinMode(53, OUTPUT); // SS on Uno, etc.
  }
#endif
  Serial.println(F("wifi begin"));
  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }
  Serial.println("whats going on?");

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  // Optional SSID scan
  // listSSIDResults();


  //cc3000.disconnect();
  //return;
  Serial.println(F("Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("  "));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  /* Display the IP address DNS, Gateway, etc. */
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  cc3000.printIPdotsRev(ip);
}


void picAndSend() {
  Serial.println(F("reached"));
  if (cam.begin()) {
    Serial.println(F("Camera Found:"));
  } else {
    Serial.println(F("No camera found?"));
    return;
  }

  cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  //cam.setImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();
  //Serial.println(F("Snap in 2 secs..."));
  delay(2000);

  if (! cam.takePicture())
    Serial.println(F("Failed to snap!"));
  else
    Serial.println(F("Picture taken!"));

  // Get the size of the image (frame) taken
  int32_t time = millis();
  uint8_t* buffer;
  int16_t jpglen = cam.frameLength();
  Serial.println(jpglen);
  uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
  // DO NOT USE 128 BYTES

  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  int d = 40  ;
  if (client.connected()) {
    //Adafruit_CC3000_ClientRef client = server.available();
    if (client) {
      Serial.println(F("heya"));
      client.println("POST /oct HTTP/1.1");
      //wdt_reset();
      delay(d);
      client.println("Host:  epa4012.herokuapp.com");
      //wdt_reset();
      delay(d);
      client.println("Accept: */*");
      //wdt_reset();
      delay(d);
      client.println("User-Agent: Arduino/1.0");
      //wdt_reset();
      delay(d);
      client.println("Connection: keep-alive");
      //wdt_reset();
      delay(d);
      client.println("Content-Type: application/octet-stream");
      //wdt_reset();
      delay(d);
      client.print("Content-Length: ");
      //wdt_reset();
      client.println(jpglen);
      //wdt_reset();
      client.println();
      //wdt_reset();
      Serial.println(F("maga"));
      while (jpglen > 0) {
        delay(d);
        // read 32 bytes at a time;
        uint8_t *buffer;
        uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
        buffer = cam.readPicture(bytesToRead);
        //client.print((char *)buffer);
        client.write(buffer, bytesToRead);
        jpglen -= bytesToRead;
        //  Serial.println(bytesToRead);
        Serial.println(jpglen);
      }
      client.flush();
      Serial.println(F("sent"));
      /*Read data until either the connection is closed, or the idle timeout is reached. */
      unsigned long lastRead = millis();
      while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
        while (client.available()) {
          char c = client.read();
          Serial.print(c);
          lastRead = millis();
        }
      }
      client.close();
    }
  } else {
    Serial.println(F("Connection failed"));
    cc3000.disconnect();
    return;
  }

  //Serial.write(0);
  //Serial.println();


  time = millis() - time;
  Serial.print(time); Serial.println(" ms elapsed");
  //digitalWrite(wifiPower, LOW);
  //analogWrite(A8, 0);
  //delay(500);
}

void sendMsg(){
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  int d = 40  ;
  if (client.connected()) {
    if (client) {
      Serial.println(F("heya"));
      client.print("GET /msg/");
      client.print("message");
      client.println(" HTTP/1.1");
      //wdt_reset();
      delay(d);
      client.println("Host:  epa4012.herokuapp.com");
      //wdt_reset();
      delay(d);
      client.println("Connection: close");
      Serial.println(F("sent"));
      /*Read data until either the connection is closed, or the idle timeout is reached. */
      unsigned long lastRead = millis();
      while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
        while (client.available()) {
          char c = client.read();
          Serial.print(c);
          lastRead = millis();
        }
      }
      client.close();
    }
  } else {
    Serial.println(F("Connection failed"));
    cc3000.disconnect();
    return;
  }
}

void sendStatus(){
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  int d = 40  ;
  if (client.connected()) {
    if (client) {
      Serial.println(F("heya"));
      client.print("GET /timeout/");
      client.print("arduino1/");
      client.print("20");
      client.print("timeout");
      client.println(" HTTP/1.1");
      //wdt_reset();
      delay(d);
      client.println("Host:  epa4012.herokuapp.com");
      //wdt_reset();
      delay(d);
      client.println("Connection: close");
      Serial.println(F("sent"));
      /*Read data until either the connection is closed, or the idle timeout is reached. */
      unsigned long lastRead = millis();
      while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
        while (client.available()) {
          char c = client.read();
          Serial.print(c);
          lastRead = millis();
        }
      }
      client.close();
    }
  } else {
    Serial.println(F("Connection failed"));
    cc3000.disconnect();
    return;
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(analogRead(0));
  if (analogRead(0) > 500) {
    if (counter < 2) {
      Beep();
    }
    else if (counter == 10) {
      picAndSend();
    }
    digitalWrite(ledPin, !statePin);
    digitalWrite(ledPin2, !statePin2);
    delay(1000);
    counter++;
    digitalWrite(ledPin, statePin);
    digitalWrite(ledPin2, statePin2);
    Serial.println(counter);
  }
  else if (counter == 30){
    Serial.println("send message");
  }
  else {
    counter = 0;
  }

  delay(100);
}

bool displayConnectionDetails(void) {
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv)) {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  } else {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
