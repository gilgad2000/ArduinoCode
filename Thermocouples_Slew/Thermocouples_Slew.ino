// demo: CAN-BUS Shield, receive data
/***  This code is still a work in progress.  Although a very interesting one.  ***/

#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x20
#define BLOCK 0xFFFFFFFF
#define ACCEPT_ALL 0x00000000

#define THERMOCOUPLE_INPUT A0

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

#define UNIT_PIN 4
#define TWO_PIN 5
#define FOUR_PIN 6

#define NUMBER_OF_SAMPLES 1
#define NUMBER_OF_CHANNELS 8

#define SMOOTHING_VAL 0.9

INT8U len = 8;
INT8U buf[8];
INT8U sendbuf[8];

int j;

unsigned long lastChecked;

float readings[NUMBER_OF_CHANNELS];

void control(unsigned char* buf);

float convertSignalToReading(float reading);

void test(unsigned char* buf);

float readfromChannel(signed char chan);

float checkForNoise();

void changeMultiplexer(unsigned char chan);

float smooth(float data, float filterVal, float smoothedVal);

void setup()
{
  int i=0;

  j=0;

  lastChecked = 0;
 
  pinMode(RESET_PIN, OUTPUT);
 
  /**  These pins are used to control the multiplexer.   **/
  pinMode(UNIT_PIN, OUTPUT);
  pinMode(TWO_PIN, OUTPUT);
  pinMode(FOUR_PIN, OUTPUT);
  
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
    readings[i] = readfromChannel(i);
  }
}

void loop()
{
   int i=0;
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
     {
        buf[i] = 0;
     }
   }
   if(millis()>=(lastChecked+(1000/NUMBER_OF_CHANNELS)))
   {
     readings[j]=smooth(readfromChannel(j),SMOOTHING_VAL,readings[j]);
     j++;
     if(j==NUMBER_OF_CHANNELS)
       j=0;
     changeMultiplexer(j);
   }
}

void request(unsigned char* buf)
{
    float reading;
    reading = 0;
    reading = readings[buf[1]];
    sendbuf[0] = DATA_PACKET;
    //  Data channel.
    sendbuf[1] = buf[1];
    sendbuf[2] = DEVICE_ADDRESS;
    memcpy((float*)&sendbuf[3],(float*)&(reading),4);
    sendbuf[7] = 0;
    CAN.sendMsgBuf(buf[2],0,8,sendbuf);
}

float convertSignalToReading(float reading)
{
  return 0.5105*reading - 48.718;
}

void test(unsigned char* buf)
{
    unsigned char received_from;
    received_from = buf[1];
    buf[0] = TEST_COMMAND;
    buf[1] = DEVICE_ADDRESS;
    buf[2] = RECEIVED;
    CAN.sendMsgBuf(received_from,0,8,sendbuf);
}

float readfromChannel(signed char chan)
{
  float reading;
  int i;
  reading = 0;
  reading = (float)analogRead(A0);
  reading = convertSignalToReading(reading);
  return reading;
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


void changeMultiplexer(unsigned char chan)
{
  /** Set the output pins to read from the multiplexer.  **/
  switch(chan)
  {
    case 0:
      digitalWrite(UNIT_PIN, LOW);
      digitalWrite(TWO_PIN, LOW);
      digitalWrite(FOUR_PIN, LOW);
      break;
    case 1:
      digitalWrite(UNIT_PIN, HIGH);
      digitalWrite(TWO_PIN, LOW);
      digitalWrite(FOUR_PIN, LOW);
      break;
    case 2:
      digitalWrite(UNIT_PIN, LOW);
      digitalWrite(TWO_PIN, HIGH);
      digitalWrite(FOUR_PIN, LOW);
      break;
    case 3:
      digitalWrite(UNIT_PIN, HIGH);
      digitalWrite(TWO_PIN, HIGH);
      digitalWrite(FOUR_PIN, LOW);
      break;
    case 4:
      digitalWrite(UNIT_PIN, LOW);
      digitalWrite(TWO_PIN, LOW);
      digitalWrite(FOUR_PIN, HIGH);
      break;
    case 5:
      digitalWrite(UNIT_PIN, HIGH);
      digitalWrite(TWO_PIN, LOW);
      digitalWrite(FOUR_PIN, HIGH);
      break;
    case 6:
      digitalWrite(UNIT_PIN, LOW);
      digitalWrite(TWO_PIN, HIGH);
      digitalWrite(FOUR_PIN, HIGH);
      break;
    case 7:
      digitalWrite(UNIT_PIN, HIGH);
      digitalWrite(TWO_PIN, HIGH);
      digitalWrite(FOUR_PIN, HIGH);
      break;
    default:
      digitalWrite(UNIT_PIN, LOW);
      digitalWrite(TWO_PIN, LOW);
      digitalWrite(FOUR_PIN, LOW);
      break;
  }
  //delayMicroseconds(50);
}
