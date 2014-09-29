// demo: CAN-BUS Shield, receive data
#include <SPI.h>
#include "mcp_can.h"
#include <wiring_private.h>
#include <pins_arduino.h>

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
#define BEGIN_EXPERIMENT 'b'

//  This define converts 4 chars in an array into a float.  It does this by taking a pointer to the first character, casting that as a 
//  float pointer and the dereferencing the float pointer.
//#define CHARSTOFLOAT *(float*)&

#define INT8U unsigned char

#define DEVICE_ADDRESS 0x23  //  Decimal 35
#define COMMON_ADDRESS 0x02  //  Decimal 2
#define BLOCK 0xFFFFFFFF
#define NUM_CHANNELS 3

#define RESET_PIN 9

#define VALVE0 3
#define VALVE1 4
#define VALVE2 5

typedef struct channel
{
  INT8U sensor_address;
  INT8U sensor_channel;
  INT8U controller_channel;
  INT8U inverse;
  float value;
  float error;
  INT8U outputpin;
  float state;
};

typedef struct instrument
{
  int currentChan;
  INT8U len;
  INT8U i;
  boolean experimentRunning;
  channel chan[NUM_CHANNELS];
  unsigned char sendBuf[8];
  unsigned char receiveBuf[8];
  unsigned long timeLastChecked;
  unsigned long timePeriod;
  INT8U itter;
};

instrument inst;

void control(unsigned char* buf);

void program(unsigned char* buf);

void programb(unsigned char* buf);

void experiment(unsigned char* buf);

void check();

void start();

void checkChannel(channel Channel);

void dataFromSensor(unsigned char* buf);

void adjustController(channel* Chan,float currentValue);

void updateOutput();

int digitalReadOutputPin(uint8_t pin);

void setup()
{ 
  
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, HIGH);
  
  CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
  /**  Set up the filter so give it an address.  **/
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,COMMON_ADDRESS);
  CAN.init_Filt(2,0,DEVICE_ADDRESS);
  CAN.init_Filt(3,0,COMMON_ADDRESS);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  inst.currentChan = 0;
  inst.len = 8;
  inst.timeLastChecked = 0;
  inst.timePeriod = 30;
  inst.itter = 0;
  
  for(inst.i=0;inst.i!=NUM_CHANNELS;inst.i++)
  {
    inst.chan[inst.i].sensor_address = 0;
    inst.chan[inst.i].sensor_channel = 0;
    inst.chan[inst.i].value = 0;
    inst.chan[inst.i].error = 0;
    inst.chan[inst.i].state = 0;
    inst.chan[inst.i].inverse = 0;
    inst.chan[inst.i].controller_channel = inst.i;
    //  This line will need to be changed for different controllers.
    inst.chan[inst.i].outputpin = VALVE0+inst.i;
    pinMode(VALVE0+inst.i,OUTPUT);
    digitalWrite(VALVE0+inst.i,LOW);
  }
  for(inst.i=0;inst.i!=inst.len;inst.i++)
  { 
    inst.sendBuf[inst.i] = 0;
    inst.receiveBuf[inst.i] = 0;
  }
  inst.experimentRunning = false;
}

void loop()
{
   //  Wait for new message.
   if((CAN.readStatus()&0x40)==0x40||(CAN.readStatus()&0x80)==0x80)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&(inst.len), inst.receiveBuf);
     //  Reset interrupts.
     CAN.clearRX0Status();
     CAN.clearRX1Status();
     if(inst.receiveBuf[0]==CONTROL_CONTROLLER)
        control(inst.receiveBuf);
     if(inst.receiveBuf[0]==PROGRAM_CONTROLLER)
        program(inst.receiveBuf); 
     if(inst.receiveBuf[0]==EXPERIMENT_TIMING)
        experiment(inst.receiveBuf);
     if(inst.receiveBuf[0]==PROGRAM_CONTROLLER_B)
        programb(inst.receiveBuf);
     if(inst.receiveBuf[0]==BEGIN_EXPERIMENT)
        experiment(inst.receiveBuf);
     if(inst.receiveBuf[0]==DATA_PACKET)
        dataFromSensor(inst.receiveBuf);
     if(inst.receiveBuf[0]==DATA_REQUEST)
        request(inst.receiveBuf);
   }
   if(((millis()-inst.timeLastChecked)>=inst.timePeriod)&&inst.experimentRunning==true)
   {
     check();
     inst.timeLastChecked = millis();
   }
   updateOutput();
}

void program(unsigned char* buf)
{
  inst.chan[buf[3]].sensor_address = buf[1];
  inst.chan[buf[3]].sensor_channel = buf[2];
  /**  Convert form char* to float.  **/
  inst.chan[buf[3]].value = *(float*)&buf[4];
}

void programb(unsigned char* buf)
{
  inst.chan[buf[1]].inverse = buf[2];
  /**  Convert form char* to float.  **/
  inst.chan[buf[1]].error = *(float*)&buf[3];
}

void control(unsigned char* buf)
{
  float tmp;
  int i;
  memcpy((float*)&tmp,(float*)&buf[2],4);
  i = int(tmp);
  if(i==1.0)
  {
      digitalWrite(VALVE0, HIGH);  
      inst.chan[buf[1]].state=1;
  }
  if(i==0.0)
  {
     digitalWrite(VALVE0, LOW); 
     inst.chan[buf[1]].state=0; 
  }
}

void experiment(unsigned char* buf)
{
  if(buf[1]==1)
    inst.experimentRunning = true;
  else
  {
    inst.experimentRunning = false;
    //  Turn off all the channels and update them.
    for(inst.i=0;inst.i!=NUM_CHANNELS;inst.i++)
        inst.chan[inst.i].state=0;
    updateOutput();
  }
}

void check()
{
  if(inst.chan[inst.currentChan].sensor_address!=0)
      checkChannel(inst.chan[inst.currentChan]);
  inst.currentChan++;
  if(inst.currentChan>NUM_CHANNELS)
    inst.currentChan = 0;
}

void checkChannel(channel Channel)
{
  //  Create message packet.
  inst.sendBuf[0] = DATA_REQUEST;
  //  Sensor channel.
  inst.sendBuf[1] = Channel.sensor_channel;
  //  Return address.
  inst.sendBuf[2] = DEVICE_ADDRESS;
  inst.sendBuf[3] = Channel.controller_channel;
  inst.sendBuf[4] = 0;
  inst.sendBuf[5] = 0;
  inst.sendBuf[6] = 0;
  inst.sendBuf[7] = 0;
  //  Send out request packet. 
  CAN.sendMsgBuf(Channel.sensor_address,0,8,inst.sendBuf);
}

void dataFromSensor(unsigned char* buf)
{
  float value;
  int i = 0;
  memcpy((char*)&value,(char*)&buf[3],4);
  for(i=0;i!=NUM_CHANNELS;i++)
  {
    if(inst.chan[i].sensor_address==buf[2] && inst.chan[i].sensor_channel==buf[1])
      adjustController(&inst.chan[i],value);
  }
}

void adjustController(channel* Chan,float currentValue)
{
  //   If too high.  
  if(currentValue>(Chan->value+Chan->error))
  {
    //  Check to see if the channel is inverted.
    if(Chan->inverse==0)
      Chan->state=0;
    else
      Chan->state=1;
  }
  //   If too low.
  if(currentValue<(Chan->value-Chan->error))
  {
    //  Check to see if the channel is inverted.
    if(Chan->inverse==0)
      Chan->state=1;
    else
      Chan->state=0;
  }
}

void request(unsigned char* buf)
{
  float value = 0.0;
  unsigned char message[8];
  value = inst.chan[buf[1]].state;
  message[0] = DATA_PACKET;
  message[1] = buf[1];
  message[2] = DEVICE_ADDRESS;
  memcpy((float*)&message[3],(float*)&value,4);
  message[7] = 0;
  CAN.sendMsgBuf(buf[2],0,8,message);
}

void updateOutput()
{
  int i=0;
  for(i=0;i!=NUM_CHANNELS;i++)
  {
    if(inst.chan[i].state==1)
      digitalWrite(inst.chan[i].outputpin,HIGH);
    else
      digitalWrite(inst.chan[i].outputpin,LOW);
  }
}
