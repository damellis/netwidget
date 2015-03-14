#include <SPI.h>

#include <Adafruit_CC3000.h>
#include <CapacitiveSensor.h>

#define WLAN_SSID "MIT GUEST"
#define WLAN_PASS ""
#define WLAN_SECURITY WLAN_SEC_UNSEC

#define SPARKFUN_PUBLIC_KEY1 ""
#define SPARKFUN_PRIVATE_KEY1 ""

#define SPARKFUN_PUBLIC_KEY2 ""
#define SPARKFUN_PRIVATE_KEY2 ""

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   22  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  23
#define ADAFRUIT_CC3000_CS    21

Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);

#define IDLE_TIMEOUT_MS  300       // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "data.sparkfun.com"

CapacitiveSensor   cs = CapacitiveSensor(26,27);     // load, sense

int redpin = 5, greenpin = 6, bluepin = 10;
int ledpin = 8; // the small led

int touch;

uint32_t ip;

void setup(void)
{
  cs.set_CS_AutocaL_Millis(15000); // auto-calibrate touch sensor every 15 seconds
  
  analogWrite(redpin, 0);
  analogWrite(greenpin, 0);
  analogWrite(bluepin, 0);
  
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 

  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
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

void loop(void)
{
  long maxval = 0;
  long start = millis();
  
  // find the maximum reading over 10 seconds.
  // if we're in the middle of showing a glow, finish it before connecting to the internet
  while (millis() - start < 10000 || touch > 0) {
    long val = cs.capacitiveSensor(30);
    val = val - 20;
    val = constrain(val, 0, 255);
    if (val > maxval) maxval = val;
    analogWrite(ledpin, val);
    Serial.println(val);
    delay(10);
    
    analogWrite(redpin, constrain(touch, 0, 255));
    analogWrite(greenpin, constrain(touch, 0, 255));
    analogWrite(bluepin, constrain(touch, 0, 255));
    
    if (touch > 0) touch--;
  }
  
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("GET /input/"));
    www.fastrprint(SPARKFUN_PUBLIC_KEY1);
    www.fastrprint(F("?private_key="));
    www.fastrprint(SPARKFUN_PRIVATE_KEY1);
    www.fastrprint(F("&touch="));
    www.print(maxval);
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  Serial.println(F("-------------------------------------"));
  
  www.connect(ip, 80);
  if (www.connected()) {
    www.fastrprint(F("GET /output/"));
    www.fastrprint(SPARKFUN_PUBLIC_KEY2);
    www.fastrprint(F(".json?gt[timestamp]=now-1minute"));
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  lastRead = millis();
  www.setTimeout(IDLE_TIMEOUT_MS);
  www.find("\"touch\":\"");
  touch = www.parseInt();
  Serial.print("touch = "); Serial.println(touch);
  www.close();
  Serial.println(F("-------------------------------------"));

//  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
//  /* the next time your try to connect ... */
//  Serial.println(F("\n\nDisconnecting"));
//  cc3000.disconnect();  
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
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
