// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x0B
#define BLOCK 0xFFFFFFFF
#define NUM_CHANNELS 1;

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

int led = 7;

void control(unsigned char* buf);

void setup()
{
  float value = 512.0;
  
  CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
  Serial.begin(115200);
  
  /**  Set up the filter so give it an address.  **/
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,BLOCK);
  CAN.init_Filt(2,0,BLOCK);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  pinMode(led, OUTPUT); 
  digitalWrite(led, LOW);
  
  for(i=0;i!=8;i++)
  {
    sendbuf[i]=0;
    buf[i]=0;
  }
  Serial.println("Started");
  //*************************   Create and send the first program packet.      ***********************************
  //  Create Setup Packet.
  sendbuf[0] = PROGRAM_CONTROLLER;
  sendbuf[1] = DEVICE_ADDRESS;
  //  Channel Number.
  sendbuf[3] = 0;
  memcpy(&sendbuf[4],&value,4);
  //  Send first program packet.
  CAN.sendMsgBuf(0x23,0,8,sendbuf);
  Serial.println("Send first program packet.");
  //*************************   Create and send the second program packet.      ***********************************
  //  Error value.
  value = 90;
  //  Create second program packet.
  //  Channel 
  sendbuf[0] = PROGRAM_CONTROLLER_B;
  sendbuf[1] = 1;
  //  Inverted?  1 if inverted, 0 if not inverted.
  sendbuf[2] = 0;
  memcpy(&sendbuf[3],&value,4);
  sendbuf[7] = 0;
  CAN.sendMsgBuf(0x23,0,8,sendbuf);
  Serial.println("Send second program packet.");
  //*************************   Create and start experiment packet.      ***********************************
  //  Error value.
  sendbuf[0] = EXPERIMENT_CONTROL_PACKET;
  sendbuf[1] = 1;
  sendbuf[2] = 0;
  sendbuf[3] = 0;
  sendbuf[4] = 0;
  sendbuf[5] = 0;
  sendbuf[6] = 0;
  sendbuf[7] = 0;
  CAN.sendMsgBuf(0x23,0,8,sendbuf);
  //Send the initialising packet.
  readings[0] = (float)analogRead(A0);
  CAN.clearRX0Status();
  CAN.clearRX1Status();
  Serial.println("Start experiment. packet.");
}

// (CAN.readStatus()&0x80)!=0x80)||((CAN.readStatus()&0x40)!=0x40)   Line preserved for possible future use.

void loop()
{

}

void request(unsigned char* buf)
{
    Serial.println("Request received.");  
    sendbuf[0] = DATA_PACKET;
    //  Data channel.
    sendbuf[1] = buf[3];
    sendbuf[2] = DEVICE_ADDRESS;
    Serial.println(readings[0]);
    memcpy((float*)&sendbuf[3],(float*)&(readings[0]),4);
    Serial.println((float)buf[3]);
    sendbuf[7] = 0;
    //  Possible delay here.
    /*Serial.println("Returning packet:");
    for(i=0;i!=8;i++)
     {
       Serial.print(sendbuf[i]); Serial.print('\t');
     }
     Serial.println();*/
    Serial.println("Sending to:");
    Serial.println(buf[2]);
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}
