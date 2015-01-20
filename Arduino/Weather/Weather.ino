#include <SPI.h>
#include <Temboo.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "TembooAccount.h"

Adafruit_CC3000 cc3000(10, 3, 5); // CS, IRQ, VBEN
Adafruit_CC3000_Client client;
Adafruit_ST7735 tft(7, 8, 9); // CS, Reset, DC
unsigned long ip;

void setup() {
  delay(3000);
  Serial.begin(9600);

  // TFT backlight  
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextWrap(true);
  
  Serial.println("Hello.");
  tft.println("Hello.");
  
  if (!cc3000.begin()) {
    Serial.println("cc3000.begin() failed.");
    while (1);
  }
  while (!cc3000.connectToAP("Mellis", "", WLAN_SEC_UNSEC)) {
    Serial.println("cc3000.connectToAP() failed. retrying in 10 seconds.");
    delay(10000);
  }
  while(!cc3000.checkDHCP()) {
    delay(100);
  }
  /* Display the IP address DNS, Gateway, etc. */  
  while (!displayConnectionDetails()) {
    delay(1000);
  }
  
  Serial.println("Connected.");
  tft.println("Connected.");
}


void loop() {
  Serial.println("Running GetWeatherByAddress");

  TembooChoreo GetWeatherByAddressChoreo(client);

  // Invoke the Temboo client
  GetWeatherByAddressChoreo.begin();

  // Set Temboo account credentials
  GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
  GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  String AddressValue = "Cambridge, MA";
  GetWeatherByAddressChoreo.addInput("Address", AddressValue);

  // Identify the Choreo to run
  GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");

  // Run the Choreo; when results are available, print them to serial
  GetWeatherByAddressChoreo.run();

  while(GetWeatherByAddressChoreo.available()) {
    char c = GetWeatherByAddressChoreo.read();
    Serial.print(c);
  }
  GetWeatherByAddressChoreo.close();

  Serial.println("\nWaiting...\n");
  delay(30000); // wait 30 seconds between GetWeatherByAddress calls
}

bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
