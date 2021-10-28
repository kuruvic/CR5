/*
 * IRremote: IRsendDemo - demonstrates sending IR codes with IRsend
 * An IR LED must be connected to Arduino PWM pin 3.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>

IRsend irsend;

char index;

unsigned long tData = 0x20DF10EF;
unsigned long tData_2 = 0x122430CF;

unsigned int irSignal[] = { 8600,4050, 500,1550, 450,550, 500,500, 450,550, 500,1500, 500,550, 
    450,550, 450,550, 500,1500, 450,1550, 500,550, 450,550, 450,550, 500,500, 450,550, 450,550, 450,600, 
    400,600, 500,500, 500,500, 450,550, 500,1550, 500,500, 400,1600, 500,500, 500,500, 450,550, 500,1550, 450};  // UNKNOWN 1035C9DA

unsigned int irSignalP[] = { 8600,4000, 500,1550, 450,550, 500,500, 500,500, 500,1550, 
    400,600, 500,500, 500,500, 450,550, 500,500, 450,600, 450,550, 450,550, 500,500, 450,550, 500,500,
    450,1600, 400,600, 500,1500, 450,550, 450,550, 500,500, 450,600, 450,550, 500,1500, 450,550, 450,1600, 500,500, 450};  // UNKNOWN 7BF485E5
    
int khz = 38; // 38kHz carrier frequency for the NEC protocol

// On the Zero and others we switch explicitly to SerialUSB
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__));
    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_SEND_PIN);
}

void loop() {

        if (Serial.available() >= 1)
        {
          index = Serial.read();
        }
        
    if(index == 'A')
    {
        irsend.sendNEC(tData, 32);
        Serial.print(F("sendNEC(0x"));
        Serial.print(tData,HEX);
        Serial.println(F(", 32)"));
        delay(2000);

        irsend.sendNEC(tData_2, 32);
        Serial.print(F("sendNEC(0x"));
        Serial.print(tData,HEX);
        Serial.println(F(", 32)"));
        delay(2000); //5 second delay between each signal burst
    }    

    if(index == 'S')
    {
        irsend.sendNEC(tData, 32);
        Serial.print(F("sendNEC(0x"));
        Serial.print(tData,HEX);
        Serial.println(F(", 32)"));
        delay(2000);

        irsend.sendNEC(tData_2, 32);
        Serial.print(F("sendNEC(0x"));
        Serial.print(tData,HEX);
        Serial.println(F(", 32)"));
        delay(2000); //5 second delay between each signal burst
    }

    if(index == 'F')
    {
        irsend.sendRaw(irSignal, sizeof(irSignal) / sizeof(irSignal[0]), khz); // Note the approach used to automatically calculate the size of the array.

        delay(2000);
    }

    if(index == 'D')
    {
        irsend.sendRaw(irSignalP, sizeof(irSignalP) / sizeof(irSignalP[0]), khz); // Note the approach used to automatically calculate the size of the array.

        delay(2000); 
    }
}
