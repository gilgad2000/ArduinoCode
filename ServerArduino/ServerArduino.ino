/**
This code will turn on the USB when a signal is received through the parallel port.
**/

// demo: CAN-BUS Shield, receive data
#include <SPI.h>
#include "mcp_can.h"


#include <stdlib.h>
#include <stdarg.h>

#define DEVICE_ADDRESS 0x03
#define BLOCK 0xFFFFFFFF

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

//  Used to clear the settings on a device
#define CLEAR_SETTINGS 'c'

#define RESET_PIN 9

#define SEND_ALL 0X02

void analyseCANMessage(unsigned char* message);
unsigned char analyseString(String readFromSerial, unsigned char* sendPacket);
void stripAtDelimiter(String* full, String* beforedelimiter);

int i;
byte byteRead;
unsigned char len=16;

void setup()
{
  //  initialise pin 13 as output
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, LOW);
  delay(100);
  digitalWrite(RESET_PIN, HIGH);
  
  Serial.begin(115200);
  if(CAN.begin(CAN_500KBPS) ==CAN_OK) Serial.print("can init ok!!\r\n");
  else Serial.print("Can init fail!!\r\n");
  //   Set up the filters.  Configure to use both readread buffers
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,SEND_ALL);
  CAN.init_Filt(2,0,DEVICE_ADDRESS);
  CAN.init_Filt(3,0,SEND_ALL);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);  
}

void loop()
{
  String readFromSerial, receivedString;
  unsigned char sendPacket[8], received[8];
  unsigned char address;
  if(Serial.available())
  {
    readFromSerial = Serial.readStringUntil('\n');
    Serial.flush();
    address = analyseString(readFromSerial,sendPacket);
  }
  if((CAN.readStatus()&0x40)==0x40||(CAN.readStatus()&0x80)==0x80)
  {
    //  Retrieve data from register.  
    CAN.readMsgBuf(&len,received);
    /**    Retrieve Infromation from string.    **/
    analyseCANMessage(received);
    //  Reset interrupts.
    CAN.clearRX0Status();
    CAN.clearRX1Status();
  }
}

unsigned char analyseString(String readFromSerial, unsigned char* sendPacket)
{
  String command, parameter, reply, parameterA, parameterB;
  char toClient[256];
  float value;
  double tmp;
  int delimiter;
  int recognised = 0;
  unsigned char sensor_address, sensor_channel, controller_channel, controller_address, inverse, address=0x00;
  
  delimiter = readFromSerial.indexOf(':');
  if(delimiter!=-1)
  {
    for(i=0;i!=8;i++)
      sendPacket[i]=0;
    command = readFromSerial.substring(0,delimiter);
    parameter = readFromSerial.substring(delimiter+1,readFromSerial.length());
    /************** Test command.  ***************/
    if(command=="Test")
    {
      Serial.println("Test successful");
      recognised = 1;
    }
    /************** Request data.  **************/
    if(command=="RequestData")
    {
      //  FORMAT:  Request:address:channel
      delimiter = parameter.indexOf(':');
      stripAtDelimiter(&parameter,&parameterA);
      //  In this case device channel
      sensor_address = atoi(&parameterA[0]);
      sensor_channel = atoi(&parameter[0]);
      //  Assemble the packet.
      sendPacket[0] = DATA_REQUEST;
      sendPacket[1] = sensor_channel;
      sendPacket[2] = DEVICE_ADDRESS;
     
      recognised = 1;
      controller_address = sensor_address;
      //  Send Packet.
      CAN.sendMsgBuf(controller_address,0,8,sendPacket);
    }
    if(command=="ProgramA")
    {
      //  FORMAT:  ProgramA:Sensor_Address:Sensor_Channel:Value:Controller_channel:Controller_address
      //  Sensor_Address.
      stripAtDelimiter(&parameter, &parameterA);
      sensor_address = atoi(&parameterA[0]);
      //   Sensor_Channel.
      stripAtDelimiter(&parameter, &parameterA);
      sensor_channel = atoi(&parameterA[0]);
      //    Value.
      stripAtDelimiter(&parameter, &parameterA);
      tmp = atof(&parameterA[0]);
      value = float(tmp);
      //    Controller Channel.
      stripAtDelimiter(&parameter, &parameterA);
      controller_channel = atoi(&parameterA[0]);      
      //    Controller Address.
      stripAtDelimiter(&parameter, &parameterA);
      controller_address = atoi(&parameterA[0]);
      //  Assemble the packet.  NOTE:
      sendPacket[0] = PROGRAM_CONTROLLER;
      sendPacket[1] = sensor_address;
      sendPacket[2] = sensor_channel;
      sendPacket[3] = controller_channel;
      memcpy((char*)&sendPacket[4],(char*)&value,4);
      recognised = 1;
      
      //  Send Packet.
      CAN.sendMsgBuf(controller_address,0,8,sendPacket);
    }
    if(command=="ProgramB")
    {
      //  FORMAT:  ProgramB:Controller_Address:Controller_Channel:Inverse:Error
      //  Controller Address
      stripAtDelimiter(&parameter, &parameterA);
      controller_address = atoi(&parameterA[0]);
      //  Controller Address
      stripAtDelimiter(&parameter, &parameterA);
      controller_channel = atoi(&parameterA[0]);
      //  Inverse
      stripAtDelimiter(&parameter, &parameterA);
      inverse = atoi(&parameterA[0]);
      //  Error
      stripAtDelimiter(&parameter, &parameterA);
      value = atof(&parameterA[0]);  
      
      //  Create packet.   
      sendPacket[0] = PROGRAM_CONTROLLER_B;
      sendPacket[1] = controller_channel;
      sendPacket[2] = inverse;
      memcpy((char*)&sendPacket[3],(char*)&value,4);
      recognised = 1;         
      //  Send Packet.
      CAN.sendMsgBuf(controller_address,0,8,sendPacket);
    }
    if(command=="Start")
    {
      controller_address = atoi(&parameter[0]);
      sendPacket[0] = EXPERIMENT_TIMING;
      sendPacket[1] = 1;
      recognised = 1;
      
      address = SEND_ALL;
      //  Send Packet.
      CAN.sendMsgBuf(SEND_ALL,0,8,sendPacket);
    }    
    if(command=="Stop")
    {
      controller_address = atoi(&parameter[0]);
      sendPacket[0] = EXPERIMENT_TIMING;
      sendPacket[1] = 0;
      recognised = 1;
      
      address = SEND_ALL;
      //  Send Packet.
      CAN.sendMsgBuf(SEND_ALL,0,8,sendPacket);
    }
    if(command=="Control")
    {
      //   Control:controller_address:controller_channel:value
      
      controller_address = atoi(&parameter[0]);
      //  Inverse
      stripAtDelimiter(&parameter, &parameterA);
      controller_address = atoi(&parameterA[0]);
      //  Controller Channel
      stripAtDelimiter(&parameter, &parameterA);
      controller_channel = atoi(&parameterA[0]);
      value = atof(&parameter[0]);
      sendPacket[0] = CONTROL_CONTROLLER;
      sendPacket[1] = controller_channel;
      memcpy((char*)&sendPacket[2],(char*)&value,4);

      recognised = 1;
      
      //  Send Packet.
      CAN.sendMsgBuf(controller_address,0,8,sendPacket);
    }
    if(command=="ClearAll")
    {
      sendPacket[0] = CLEAR_SETTINGS;
      recognised = 1;
      
      address = SEND_ALL;
      //  Send Packet.
      CAN.sendMsgBuf(SEND_ALL,0,8,sendPacket);
    }
    if(recognised ==0)
    {  
       reply = "Not a recognised commad = |" + readFromSerial + "|\n";
       Serial.print(reply);
    }   
  }
  else
  {
    reply = "Not a recognised string = |" + readFromSerial + "|\n";
    Serial.print(reply);  
  }
  return controller_address;
}

//  This function works by call by reference, it stips up to the delimiter from the first string and then returns that in full without
//  the delimiter.
void stripAtDelimiter(String* full, String* beforedelimiter)
{
  int delimiter;
  delimiter = full->indexOf(':');
  *beforedelimiter = full->substring(0,delimiter);
  *full = full->substring(delimiter+1,full->length());
}

void analyseCANMessage(unsigned char* message)
{
   float value;
   char toClient[256],tmp[10];
   switch(message[0])
   {
     case DATA_PACKET:
         memcpy((char*)&value,(char*)&message[3],4);
         sprintf(&toClient[0],"DataFrom:%i:Channel:%i:Value:",message[2],message[1]);
         Serial.print(toClient);
         dtostrf(value,1,6,&tmp[0]);
         Serial.print(tmp);
         Serial.print('\n');
         break;
     default:
         //Serial.println("Not a recognised packet.");
         value = 0;
   }
}
