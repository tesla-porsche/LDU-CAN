

#include <Arduino.h>
#include <FlexCAN.h> //https://github.com/teachop/FlexCAN_Library 


/////////// Variables //////////////

float fslipMin = 1.80;
float fslipMax = 3.50;





int rpm;
int pot;
int fweak;
float maxSlip;
float minSlip;
float fslip;
float throtRamp;
const int led = 13;




CAN_message_t msg;
CAN_message_t inMsg;
CAN_filter_t filter;


void setup() {

    pinMode(led, OUTPUT);
    Can0.begin(500000);

    //set filters for standard
    for (int i = 0; i < 8; i++)
    {
        Can0.setFilter(filter, i);
    }
    //set filters for extended
    for (int i = 9; i < 13; i++)
    {
        Can0.setFilter(filter, i);
    }

    digitalWrite(led, HIGH);
    Serial.begin(1152000);


}

void loop() {

    while (Can0.available())
    {
        Can0.read(inMsg);
        decodeCAN();
    }

    parameterMap();
    debug();
}




void decodeCAN() {

    if (inMsg.id == 0x135) {
        rpm = (((inMsg.buf[1] << 8) + inMsg.buf[0]));
    }


    else if (inMsg.id == 0x113) {
        pot = ((inMsg.buf[1] << 8) + inMsg.buf[0]);   
    }
}


void parameterMap() {

    //fweak
    if (pot > 1800 && pot < 3200) {
        fweak = map(pot, 1800, 3200, 400, 250);
    }
    else if (pot >= 3200) {

        fweak = 250;
    }
    else {
        fweak = 400;
    }
    canSet(1, fweak);



    //fslipmin
    canSet(4, fslipMin);


    //fslipmax

    if (pot >= 2800) {
        minSlip = map(pot, 2800, 4095, (fslipMin * 32), fslipMax * 32);

    }
    else { minSlip = (fslipMin * 32); }

    if (rpm <= 4500) {
        fslip = map(rpm, 0, 4200, minSlip, fslipMax * 32);
    }
    else { fslip = maxSlip; }

    canSet(5, fslip / 32);

    // throtramp

    if (pot < 1500) {
        throtRamp = .45;
    }
    else if (pot >= 1500 && pot < 3700) {
        throtRamp = map(pot, 1500, 3700, 1, 25);
    }
    else {
        throtRamp = 25;

    }
    canSet(49, throtRamp);
}



void canSet(int index, float value) {
    int val = (value * 32);  //scale value * 32 to make STM32 happy
    int byte1;
    int byte2;
    int byte3;
    int byte4;
    byte1 = val & 0xFF;  //bitshifting
    byte2 = (val >> 8) & 0xFF;
    byte3 = (val >> 16) & 0xFF;
    byte4 = (val >> 24) & 0xFF;

    msg.id = 0x601; //set parameter ID
    msg.len = 8;
    msg.buf[0] = 0x40;
    msg.buf[1] = 0x00;
    msg.buf[2] = 0x20;
    msg.buf[3] = index; //index value of parameter, boost = 0
    msg.buf[4] = byte1;
    msg.buf[5] = byte2;
    msg.buf[6] = byte3;
    msg.buf[7] = byte4;
    Can0.write(msg);
}


void debug() {

    Serial.print("Pot val: ");
    Serial.println(pot);
    Serial.print("RPM: ");
    Serial.println(rpm);
    Serial.print("FWEAK: ");
    Serial.println(fweak);
    Serial.print("Modified fslipmax: ");
    Serial.println(fslip/32);
    Serial.println("");
    Serial.println("");
    Serial.println("");
}
