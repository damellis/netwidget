#include <CapacitiveSensor.h>

CapacitiveSensor   cs = CapacitiveSensor(26,27);     // load, sense
int R = 5, G = 6, B = 10;
int fade = 0;

void setup()                    
{
   cs.set_CS_AutocaL_Millis(10000);
   Serial.begin(9600);
}

void loop()                    
{
    long start = millis();
    long total =  cs.capacitiveSensor(30);
    fade = max(fade, constrain(total - 20, 0, 512));
    if (fade > 0) fade--;

    Serial.print(millis() - start);
    Serial.print("\t");
    Serial.print(total);    
    Serial.print("\t");
    Serial.println(fade);
    
    analogWrite(R, constrain(fade, 0, 255));
    analogWrite(G, constrain(fade, 0, 255));
    analogWrite(B, constrain(fade, 0, 255));

    delay(10);
}
