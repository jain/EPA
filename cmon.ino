#include <Adafruit_VC0706.h>
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include <stdlib.h>
#include "utility/debug.h"
#include <SoftwareSerial.h>

#define ADAFRUIT_CC3000_IRQ   2  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  7  // These can be any two pins
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "t1"           // cannot be longer than 32 characters!
#define WLAN_PASS       "abcdefghij"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

int32_t ip;
// What page to grab!
#define WEBSITE      "epa4012.herokuapp.com"
#define WEBPAGE      "/"

// Using SoftwareSerial (Arduino 1.0+) or NewSoftSerial (Arduino 0023 & prior):
#if ARDUINO >= 100
SoftwareSerial cameraconnection = SoftwareSerial(4, 3);
#else
NewSoftSerial cameraconnection = NewSoftSerial(4, 3);
#endif

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);


void setup() {
#if !defined(SOFTWARE_SPI)
  if (ADAFRUIT_CC3000_CS != 10) {
    pinMode(10, OUTPUT); // SS on Uno, etc.
  }
#endif

  Serial.begin(115200);
  if (cam.begin()) {
    Serial.println(F("Camera Found:"));
  } else {
    Serial.println(F("No camera found?"));
    return;
  }

  //cam.setImageSize(VC0706_640x480);        // biggest
  //cam.setImageSize(VC0706_320x240);        // medium
  cam.setImageSize(VC0706_160x120);          // small



  if (!cc3000.begin()) {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while (1);
  }

  // Optional SSID scan
  // listSSIDResults();

  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

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
  pinMode(8, OUTPUT);

  uint8_t* buffer;
  int16_t jpglen = cam.frameLength();
  Serial.println(jpglen);
  uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
  // DO NOT USE 128 BYTES

  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    //Adafruit_CC3000_ClientRef client = server.available();
    if (client) {
      Serial.println(F("heya"));
      client.println("POST /oct HTTP/1.1");
      client.println("Host:  epa4012.herokuapp.com");
      client.println("Accept: */*");
      client.println("User-Agent: Arduino/1.0");
      client.println("Connection: keep-alive");
      client.println("Content-Type: application/octet-stream");
      client.print("Content-Length: ");
      //client.println(4);
      client.println(jpglen);
      client.println();
      /*client.print('a');
      client.print('b');
      client.print('c');
      client.print('d');*/
      while (jpglen > 0) {
        // read 32 bytes at a time;
        uint8_t *buffer;
        uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
        buffer = cam.readPicture(bytesToRead);
        //client.print((char *)buffer);
        client.write(buffer, bytesToRead);
        //Serial.write((char *)buffer, bytesToRead);
        jpglen -= bytesToRead;
        Serial.println(jpglen);
      }
      /*for (int i = 0; i < 125; i++) {
        if (i % 100 == 0) {
          Serial.println(F("wow"));
        }
        client.print("dddddddd");
        }*/
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

  Serial.write(0);
  //Serial.println();


  time = millis() - time;
  //Serial.print(time); Serial.println(" ms elapsed");
}

void loop() {

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
