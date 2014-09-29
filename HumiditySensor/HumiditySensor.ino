// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x25   //  37 in decimal   25 in hexadecimal.
#define BLOCK 0xFFFFFFFF
#define NUM_CHANNELS 2
#define RESET_PIN 9
#define INT_PIN 2

//   Used to request Data from a sensor
#define DATA_REQUEST  'R'

//   Used to program a controller.
#define PROGRAM_CONTROLLER 'P'

//   Used to force a parameter on a controller.
#define CONTROL_CONTROLLER 'C'

//  Used to set timing details when setting up an experiment.
#define EXPERIMENT_TIMING 'E'

//  Used to  set more control parameter details.
#define PROGRAM_CONTROLLER_B  'B'

//   Used to return requested data from a sensor.
#define DATA_PACKET 'D'

//  Used to being an experiment.
#define EXPERIMENT_CONTROL_PACKET 'b'

typedef struct instrument
{
  INT8U len;
  INT8U buf[8];
  INT8U sendbuf[8];
  INT8U i;
};

instrument inst;

void request();

float readFromChannel(unsigned char num);

float readingToHumidity(float reading);

float readingToTempterature(float reading);

void setup()
{ 
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
  delay(20); 
  digitalWrite(RESET_PIN, HIGH);
  
  CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
  
  /**  Set up the filter so give it an address.  **/
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,BLOCK);
  CAN.init_Filt(2,0,DEVICE_ADDRESS);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  CAN.clearRX0Status();
  CAN.clearRX1Status();
  
  inst.len = 8;
  
  for(inst.i=0;inst.i!=inst.len;inst.i++)
  {
    inst.sendbuf[inst.i]=0;
    inst.buf[inst.i]=0;
  }
  
}

void loop()
{
   //  Wait for new message.
   if((CAN.readStatus()&0x40)==0x40||(CAN.readStatus()&0x80)==0x80)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&inst.len, inst.buf);
     if(inst.buf[0]==DATA_REQUEST)
        request();
     //  Clear the received message.
     for(inst.i=0;inst.i!=inst.len;inst.i++)
        inst.buf[inst.i]=0;
     //  Reset interrupts.
     CAN.clearRX0Status();
     CAN.clearRX1Status();
     delay(4);
   }
}

void request()
{
    float reading;
    inst.sendbuf[0] = DATA_PACKET;
    //  Data channel.
    inst.sendbuf[1] = inst.buf[1];
    inst.sendbuf[2] = DEVICE_ADDRESS;
    reading = readFromChannel(inst.buf[1]);
    memcpy((float*)&inst.sendbuf[3],(float*)&(reading),sizeof(float));
    inst.sendbuf[7] = 0;
    CAN.sendMsgBuf(inst.buf[2],0,inst.len,inst.sendbuf);
}

float readFromChannel(unsigned char num)
{
  float reading;
  switch(num)
  {
    case 0:
      reading = (float)analogRead(A0);
      reading = readingToHumidity(reading);
      break;
    case 1:
      reading = (float)analogRead(A1);
      reading = readingToTempterature(reading);
      break;
  }
  return reading;
}

float readingToHumidity(float reading)
{
  //reading = -0.00007*reading*reading + 0.1917*reading - 31.622;
  return reading;
}

float readingToTempterature(float reading)
{
  //reading = 0.193*reading - 71.447;
  return reading;
}
