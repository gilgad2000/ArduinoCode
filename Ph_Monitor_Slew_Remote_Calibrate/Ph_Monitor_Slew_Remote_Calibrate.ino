// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <EEPROM.h>
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

#define NUMCALS 3

#define SMOOTHING_VAL 0.95

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

//  Request calibration
#define REQUEST_CALIBRATION 'a'

#define REMOTE_CALIBRATION 'A'

//  Returned Calibration
#define RETURNED_CALIBRATION 'r'

#define SIZEOFFLOAT 4

//  Used to being an experiment.
#define EXPERIMENT_CONTROL_PACKET 'b'

INT8U len = 8;
INT8U buf[8];
INT8U sendbuf[8];

unsigned long lastChecked;

float readings[1];

float calibration[NUMCALS];

int i;
int j;

void control(unsigned char* buf);

float convertToPh(float reading);

float smooth(float data, float filterVal, float smoothedVal);

void requestCalibration(unsigned char* buf);

void calibrate(unsigned char* buf);

void setup()
{
  unsigned char tmp[SIZEOFFLOAT];
  lastChecked = 0;
  
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
  lastChecked = 0;
  
  //  Retrieve calibration from EEPROM.
  for(i=0;i!=NUMCALS;i++)
  {
    for(j=0;j!=SIZEOFFLOAT;j++)
    {
       tmp[j] = EEPROM.read((i*SIZEOFFLOAT)+j);
    }
    memcpy((char*)&calibration[i],(char*)&tmp[0],sizeof(float));
  }
  
  readings[0] = convertToPh((float)analogRead(A1));
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
     if(buf[0]==REMOTE_CALIBRATION)
        calibrate(buf);
     if(buf[0]==REQUEST_CALIBRATION)
        requestCalibration(buf);
     for(i=0;i!=8;i++)
       buf[i]=0;
     //  Reset interrupts.
     CAN.clearRX0Status();
     CAN.clearRX1Status();
   }  
   if(millis()>=(lastChecked+200))
     readings[0]=smooth(convertToPh((float)analogRead(A1)),SMOOTHING_VAL,readings[0]);
}

void request(unsigned char* buf)
{
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
  return calibration[2]*reading*reading + calibration[1]*reading + calibration[0];
}

/**  Function downloaded from: http://playground.arduino.cc/Main/Smooth  **/
float smooth(float data, float filterVal, float smoothedVal)
{
  if (filterVal > 1){      // check to make sure param's are within range
    filterVal = .99;
  }
  else if (filterVal <= 0){
    filterVal = 0;
  }
  smoothedVal = (data * (1 - filterVal)) + (smoothedVal  *  filterVal);
  return smoothedVal;
}

//  This should allow for remote recalibration of modules.  Once calibrated the data is stored in the EEPROM and retrieved on start up.
void calibrate(unsigned char* buf)
{
    memcpy((float*)&calibration[buf[1]],(float*)&buf[2],SIZEOFFLOAT);
    for(i=0;i!=SIZEOFFLOAT;i++)
    {
      EEPROM.write((buf[1]*SIZEOFFLOAT)+i,buf[2+i]);
    }
}

void requestCalibration(unsigned char* buf)
{
    unsigned char tmp[SIZEOFFLOAT];
    for(j=0;j!=SIZEOFFLOAT;j++)
    {
       sendbuf[3+j] = EEPROM.read(buf[1]*SIZEOFFLOAT+j);
    }
    sendbuf[0] = RETURNED_CALIBRATION;
    sendbuf[1] = DEVICE_ADDRESS;
    sendbuf[2] = buf[1];
    //memcpy((char*)&sendbuf[3],(char*)&calibration[buf[1]],sizeof(float));
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}
