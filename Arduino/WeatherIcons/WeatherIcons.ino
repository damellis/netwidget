// Images from: https://thenounproject.com/DmitryBaranovskiy/collection/weatheroo/

#include <SD.h>
#include <SPI.h>
#include <Temboo.h>
#include <TFT.h>
#include <Adafruit_CC3000.h>

#include "Conditions.h"
#include "TembooAccount.h"

Adafruit_CC3000 cc3000(21, 22, 23); // CS, IRQ, VBEN
Adafruit_CC3000_Client client;
TFT tft(3, 9, 8); // CS, DC, Reset
unsigned long ip;
boolean sd;

void setup() {
  Serial.begin(9600);
  
  if(SD.begin(16)) {
    sd = true;
    for (int i = 0; i < NUM_IMAGES; i++) {
      images[i] = tft.loadImage(filenames[i]);
    }
  } else {
    sd = false;
    tft.println("SD init failed.");
    Serial.println("SD init failed.");
  }

  // TFT backlight  
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  tft.begin();
  tft.setRotation(3);
  tft.background(0, 0, 0);
  tft.setCursor(0, 0);
  tft.stroke(255, 255, 255);
  tft.setTextWrap(true);
  
  tft.println(F("Hello."));
  Serial.println(F("Hello."));
  
  tft.println(F("Initializing CC3000..."));
  Serial.println(F("Initializing CC3000..."));

  if (!cc3000.begin()) {
    tft.println(F("cc3000.begin() failed."));
    Serial.println(F("cc3000.begin() failed."));
    while (1);
  }
  
  tft.print(F("Connecting to ")); tft.print(WIFI_SSID); tft.println(F("..."));
  Serial.print(F("Connecting to ")); Serial.print(WIFI_SSID); Serial.println(F("..."));
  
  while (!cc3000.connectToAP(WIFI_SSID, WIFI_PASS, WIFI_SEC)) {
    tft.println(F("Connection failed. Retrying in 10 seconds."));
    Serial.println(F("Connection failed. Retrying in 10 seconds."));
    delay(10000);
  }
  
  tft.print(F("Getting IP addresss..."));
  Serial.print(F("Getting IP addresss..."));
  while(!cc3000.checkDHCP()) {
    tft.print(".");
    Serial.print(".");
    delay(100);
  }
  tft.println();
  Serial.println();
  
  /* Display the IP address DNS, Gateway, etc. */  
  while (!displayConnectionDetails()) {
    delay(1000);
  }
  
  tft.println(F("Connected."));
  Serial.println(F("Connected"));
}


void loop() {
  Serial.print(F("Running GetWeatherByAddress..."));

  TembooChoreo GetWeatherByAddressChoreo(client);

  // Invoke the Temboo client
  GetWeatherByAddressChoreo.begin();

  // Set Temboo account credentials
  GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
  GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set Choreo inputs
  String AddressValue = "Berkeley, CA";
  GetWeatherByAddressChoreo.addInput("Address", AddressValue);
  
  // Filter Choreo output
  GetWeatherByAddressChoreo.addOutputFilter("code", "/rss/channel/item/yweather:condition/@code", "Response");  
  GetWeatherByAddressChoreo.addOutputFilter("date", "/rss/channel/item/yweather:condition/@date", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("temp", "/rss/channel/item/yweather:condition/@temp", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("text", "/rss/channel/item/yweather:condition/@text", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("date1", "/rss/channel/item/yweather:forecast[1]/@date", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("low1", "/rss/channel/item/yweather:forecast[1]/@low", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("high1", "/rss/channel/item/yweather:forecast[1]/@high", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("text1", "/rss/channel/item/yweather:forecast[1]/@text", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("date2", "/rss/channel/item/yweather:forecast[2]/@date", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("low2", "/rss/channel/item/yweather:forecast[2]/@low", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("high2", "/rss/channel/item/yweather:forecast[2]/@high", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("text2", "/rss/channel/item/yweather:forecast[2]/@text", "Response");

  // Identify the Choreo to run
  GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");
  
  // Run the Choreo; when results are available, print them to serial
  GetWeatherByAddressChoreo.run();
  
  Serial.println("done.");
  
  String date, temp, text, date1, low1, high1, text1, date2, low2, high2, text2, http_code;
  int code;
  
  Serial.println(F("Retrieving results..."));

  while (GetWeatherByAddressChoreo.available()) {
    String key = GetWeatherByAddressChoreo.readStringUntil('\x1F');
    String value = GetWeatherByAddressChoreo.readStringUntil('\x1E');
    
    key.trim(); value.trim();
    
    Serial.print(key);
    Serial.print("\t");
    Serial.print(value);
    Serial.println();
    
    if (key == "code") code = value.toInt();
    if (key == "date") date = value;
    if (key == "temp") temp = value;
    if (key == "text") text = value;
    if (key == "date1") date1 = value;
    if (key == "low1") low1 = value;
    if (key == "high1") high1 = value;
    if (key == "text1") text1 = value;
    if (key == "date2") date2 = value;
    if (key == "low2") low2 = value;
    if (key == "high2") high2 = value;
    if (key == "text2") text2 = value;
    if (key == "HTTP_CODE") http_code = value;
  }
  GetWeatherByAddressChoreo.close();

  // if successful, print the new forecast.
  if (http_code.equals("200")) {
    Serial.println("Success!");
    tft.background(0, 0, 0); // clear screen
    tft.setCursor(0, 0);
    tft.stroke(255, 255, 255);
    tft.setTextWrap(true);
    tft.setTextSize(3);
    tft.println(temp);
    tft.setTextSize(1);
    tft.println(text);
    tft.println();
    tft.println();
    tft.print("Today: "); tft.print(low1); tft.print("-"); tft.print(high1); tft.print(" F"); tft.println();
    tft.println(text1);
    tft.println();
    tft.print("Tomorrow: "); tft.print(low2); tft.print("-"); tft.print(high2); tft.print(" F"); tft.println();
    tft.println(text2);
    tft.setCursor(0, 108);
    tft.println(AddressValue);
    tft.println(date.substring(0, date.lastIndexOf(" "))); // remove timezone
    if (sd && conditions[code] != -1) tft.image(images[conditions[code]], 110, 0);
    Serial.println("Done.");
    delay(1L * 60 * 60 * 1000); // on success, wait one hour
  } else {
//    tft.setCursor(0, 100);
//    tft.print(http_code);
    delay(30L * 1000); // otherwise, wait 30 seconds
  }
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
