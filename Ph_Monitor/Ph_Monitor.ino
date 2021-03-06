// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x00000022
#define BLOCK 0xFFFFFFFF
#define NUM_CHANNELS 1
#define RESET_PIN 9
#define INT_PIN 2
#define NUM_READINGS 8

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

INT8U len = 8;
INT8U buf[8];
INT8U sendbuf[8];


float readings[1];

int i;

void control(unsigned char* buf);

float convertToPh(float reading);

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
  CAN.init_Filt(2,0,BLOCK);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  
  CAN.clearRX0Status();
  CAN.clearRX1Status();
  
  
  for(i=0;i!=8;i++)
  {
    sendbuf[i]=0;
    buf[i]=0;
  }
}

void loop()
{
   //  Wait for new message.
   if((CAN.readStatus()&0x40)==0x40||(CAN.readStatus()&0x80)==0x80)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&len, buf);
     if(buf[0]==DATA_REQUEST)
        request(buf);
     for(i=0;i!=8;i++)
       buf[i]=0;
     //  Reset interrupts.
     CAN.clearRX0Status();
     CAN.clearRX1Status();
   }  
}

void request(unsigned char* buf)
{
    readings[0] = 0;
    for(i=0;i!=NUM_READINGS;i++)
    {
       readings[0] = readings[0] + (float)analogRead(A1);
       delayMicroseconds(500);   
    }
    readings[0] = readings[0]/NUM_READINGS;  
    readings[0] = convertToPh(readings[0]);
    sendbuf[0] = DATA_PACKET;
    //  Data channel.
    sendbuf[1] = buf[1];
    sendbuf[2] = DEVICE_ADDRESS;
    memcpy((float*)&sendbuf[3],(float*)&(readings[0]),sizeof(float));
    sendbuf[7] = 0;
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}

float convertToPh(float reading)
{
  //return reading;
  return 0.009*reading + 1.7013;
}
