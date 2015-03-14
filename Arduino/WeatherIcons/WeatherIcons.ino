#include <SD.h>
#include <SPI.h>
#include <Temboo.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#include "TembooAccount.h"

Adafruit_CC3000 cc3000(21, 22, 23); // CS, IRQ, VBEN
Adafruit_CC3000_Client client;
Adafruit_ST7735 tft(3, 9, 8); // CS, DC, Reset
unsigned long ip;

void setup() {
  delay(3000);
  Serial.begin(9600);
  
  if(!SD.begin(16)) {
    tft.println("SD init failed.");
    Serial.println("SD init failed.");
  }

  // TFT backlight  
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
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
  
  tft.print(F("Connecting to ")); tft.println(WIFI_SSID); tft.println(F("..."));
  Serial.print(F("Connecting to ")); Serial.println(WIFI_SSID); Serial.println(F("..."));
  
  while (!cc3000.connectToAP(WIFI_SSID, WIFI_PASS, WIFI_SEC)) {
    tft.println(F("cc3000.connectToAP() failed. retrying in 10 seconds."));
    Serial.println(F("cc3000.connectToAP() failed. retrying in 10 seconds."));
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
}


void loop() {
  Serial.println(F("Running GetWeatherByAddress"));
  tft.println(F("Weather..."));

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
  
  // Filter Choreo output
  GetWeatherByAddressChoreo.addOutputFilter("date", "/rss/channel/item/yweather:forecast[1]/@date", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("low", "/rss/channel/item/yweather:forecast[1]/@low", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("high", "/rss/channel/item/yweather:forecast[1]/@high", "Response");
  GetWeatherByAddressChoreo.addOutputFilter("text", "/rss/channel/item/yweather:forecast[1]/@text", "Response");

  // Identify the Choreo to run
  GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");
  
  tft.println("run()");

  // Run the Choreo; when results are available, print them to serial
  GetWeatherByAddressChoreo.run();
  
  tft.println("done.");
  
  String low, high, date, text, code;

  while (GetWeatherByAddressChoreo.available()) {
    String key = GetWeatherByAddressChoreo.readStringUntil('\x1F');
    String value = GetWeatherByAddressChoreo.readStringUntil('\x1E');
    
    key.trim(); value.trim();
    
    Serial.print(key);
    Serial.print("\t");
    Serial.print(value);
    Serial.println();
    
    if (key == "low") low = value;
    if (key == "high") high = value;
    if (key == "text") text = value;
    if (key == "date") date = value;
    if (key == "HTTP_CODE") code = value;
  }
  GetWeatherByAddressChoreo.close();
  
  Serial.print(date);
  Serial.print(": low ");
  Serial.print(low);
  Serial.print(", high ");
  Serial.print(high);
  Serial.println();
  Serial.print("Conditions: ");
  Serial.println(text);
  Serial.print("HTTP code: ");
  Serial.println(code);
  
  tft.println("Weather!");
  
  // if successful, print the new forecast.
  if (code.equals("200")) {
    Serial.println("Success!");
    tft.fillScreen(ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextWrap(true);
    tft.setCursor(0, 0);  
    tft.println(date.substring(0, date.lastIndexOf(" ")));
    tft.print(low); tft.print("-"); tft.print(high); tft.print(" F"); tft.println();
    tft.println(text);
    bmpDraw("snow.bmp", 0, 80);
    delay(4L * 60 * 60 * 1000); // on success, wait four hours
  } else {
    tft.setCursor(0, 100);
    tft.print(code);
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

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print("File not found");
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
