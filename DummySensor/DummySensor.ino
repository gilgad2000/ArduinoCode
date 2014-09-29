// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x0B
#define BLOCK 0xFFFFFFFF
#define NUM_CHANNELS 1
#define RESET_PIN 9

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
   float value = 512;
  
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, HIGH);
  
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
  
  CAN.clearRX0Status();
  CAN.clearRX1Status();
  
  pinMode(led, OUTPUT); 
  digitalWrite(led, LOW);
  
  for(i=0;i!=8;i++)
  {
    sendbuf[i]=0;
    buf[i]=0;
  }
  Serial.println("Started");
}

// (CAN.readStatus()&0x80)!=0x80)||((CAN.readStatus()&0x40)!=0x40)   Line preserved for possible future use.

void loop()
{
   //  Wait for new message.
   //Serial.println(CAN.readStatus());
   if((CAN.readStatus()&0x40)==0x40||(CAN.readStatus()&0x80)==0x80)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&len, buf);
     if(buf[0]==DATA_REQUEST)
        request(buf);
     //  Check other buffer.
     CAN.readMsgBuf(&len, buf);
     if(buf[0]==DATA_REQUEST)
        request(buf);     
     CAN.clearRX0Status();
     CAN.clearRX1Status();
   }
   readings[0] = (float)analogRead(A0);
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
    Serial.println("Returning packet:");
    for(i=0;i!=8;i++)
    {
      Serial.print(sendbuf[i]); Serial.print('\t');
    }
    Serial.println("Sending to :");
    Serial.println(buf[2]); 
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}
