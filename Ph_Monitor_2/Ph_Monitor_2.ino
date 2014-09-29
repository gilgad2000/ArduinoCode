// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x00000022
#define BLOCK 0xFFFFFFFF
#define ACCEPT_ALL 0x00000000
#define NUM_CHANNELS 1;

#define RECEIVED 0xFF

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

#define TEST_COMMAND 'T'

#define RESET_PIN 9

INT8U len = 8;
INT8U buf[8];
INT8U sendbuf[8];

float readings[1];

int i;

void control(unsigned char* buf);

float convertSignalToReading(float reading);

void test(unsigned char* buf);

void setup()
{
 
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, LOW);
  delay(20);
  digitalWrite(RESET_PIN, HIGH);
  
  CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
  Serial.begin(115200);
  
  
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
  
  
  for(i=0;i!=8;i++)
  {
    sendbuf[i]=0;
    buf[i]=0;
  }
}

void loop()
{
   //  Wait for new message.
   if(CAN.readStatus()!=0)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&len, buf);
     //  Reset interrupts.
     CAN.clearRX0Status();
     CAN.clearRX1Status();
     if(buf[0]==DATA_REQUEST)
        request(buf);
     if(buf[0]==TEST_COMMAND)
        test(buf);
     for(i=0;i!=8;i++)
        buf[i] = 0;
   }
}

void request(unsigned char* buf)
{
    readings[0] = (float)analogRead(A1);
    sendbuf[0] = DATA_PACKET;
    //  Data channel.
    sendbuf[1] = buf[3];
    sendbuf[2] = DEVICE_ADDRESS;
    memcpy((float*)&sendbuf[3],(float*)&(readings[0]),sizeof(float));
    sendbuf[7] = 0;
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}

float convertSignalToReading(float reading)
{
  return reading;
  //return -0.00004*reading*reading + 0.0925*reading + 0.1175;
}

void test(unsigned char* buf)
{
    buf[0] = TEST_COMMAND;
    buf[1] = DEVICE_ADDRESS;
    buf[2] = RECEIVED;
    CAN.sendMsgBuf(buf[1],0,8,sendbuf);
}
