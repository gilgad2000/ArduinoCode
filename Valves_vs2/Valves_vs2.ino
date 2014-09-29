/***  Minor error:  After running an experiment, checking a channel will turn it on.  If no experiment has been run or a clear_settings command has been sent
then all is well.  This only happens when an experiment is not being run.  I've put a lot of hours into finding this error to no avail.  Since it makes no difference
then I'll turn back to look for this error.   ***/

// demo: CAN-BUS Shield, receive data
#include <SPI.h>
#include "mcp_can.h"
#include <wiring_private.h>
#include <pins_arduino.h>

//#define SERIAL

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
#define BEGIN_EXPERIMENT 'E'

//  Used to clear the settings on a device
#define CLEAR_SETTINGS 'c'

//  This define converts 4 chars in an array into a float.  It does this by taking a pointer to the first character, casting that as a 
//  float pointer and the dereferencing the float pointer.
//#define CHARSTOFLOAT *(float*)&

#define INT8U unsigned char

#define DEVICE_ADDRESS 0x23  //  Decimal 35
#define BLOCK 0xFFFFFFFF
#define SHARED_ADDRESS 0X02
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
  boolean enabled;
  boolean checkedRecently;
};

typedef struct instrument
{
  channel channels[NUM_CHANNELS];
  int currentChan;
  INT8U len;
  INT8U itter;
  boolean experimentRunning;
  channel chan[NUM_CHANNELS];
  unsigned long timeLastChecked;
  unsigned long currentTime;
  unsigned char sendBuf[8];
  unsigned char receiveBuf[8];
};

instrument inst;

unsigned char pins[3];

unsigned long timePeriod = 90;

void control(unsigned char* buf);

void program(unsigned char* buf);

void programb(unsigned char* buf);

void experiment(unsigned char* buf);

void check();

void start_stop(unsigned char* buf);

void checkChannel(channel Channel);

void dataFromSensor(unsigned char* buf);

void adjustController(unsigned char chan_num,float currentValue);

int digitalReadOutputPin(uint8_t pin);

void adjustoutputaccordingtostate();

void all_off();

void clearAll();

void setup()
{ 
  int i;
  /**  Set up output pins.  **/
  pins[0]=VALVE0; pins[1] = VALVE1; pins[2]=VALVE2;
  
  /**  Put CAN Reset pin hight.  This pin shouldn't need to every be put low.  **/
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, HIGH);
  
  /**  Begin the CAN bus.  **/  
  CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
  /**  If available start the serial connection.  **/
  #ifdef SERIAL
    Serial.begin(115200);
  #endif
  /**  Set up the filter so give it an address.  **/
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,SHARED_ADDRESS);
  CAN.init_Filt(2,0,BLOCK);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  /**    Set the digital outout pins.  **/
  pinMode(VALVE0, OUTPUT); 
  pinMode(VALVE1, OUTPUT); 
  pinMode(VALVE2, OUTPUT); 
  
  inst.currentChan = 0;
  inst.len = 8;
  
  /**  Populate the channel array.  **/
  clearAll();
  
  for(i=0;i!=inst.len;i++)
  { 
    inst.sendBuf[i] = 0;
    inst.receiveBuf[i] = 0;
  }
  /**  Stop the experiment running on start up.  **/
  inst.experimentRunning = false;
  inst.timeLastChecked = 0;
  inst.itter = 0;
  
}

void loop()
{
   /**  Has a new message been received?  **/
   if((CAN.readStatus()&0x03)!=0)
   {
     #ifdef SERIAL
       Serial.println("Message received,");
     #endif
     /**  Once new message arrives retrieve it.  **/
     CAN.readMsgBuf(&(inst.len), inst.receiveBuf);
     /**  Reset interrupts.  **/
     CAN.clearRX0Status();
     CAN.clearRX1Status();
     /**  Check the first byte of the message to see what its purpose was.  **/
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
     if(inst.receiveBuf[0]==CLEAR_SETTINGS)
        clearAll();
   }
   /**  If 90 miliseconds has elapsed since it was last checked and an experiment is running.  **/
   if(((millis()-inst.timeLastChecked)>=timePeriod)&&inst.experimentRunning==true)
   {
     check();
     inst.timeLastChecked = millis();
   }
   adjustoutputaccordingtostate();
}

void adjustoutputaccordingtostate()
{
  int i;
  for(i=0;i!=NUM_CHANNELS;i++)
  {
    if(inst.chan[i].state==1)
      digitalWrite(inst.chan[i].outputpin,HIGH);
     else
      digitalWrite(inst.chan[i].outputpin,LOW); 
  }
}

void control(unsigned char* buf)
{
  float tmp;
  int i;
  memcpy((float*)&tmp,(float*)&buf[2],4);
  i = int(tmp);
  if(i==1)
    inst.chan[buf[1]].state = 1;
  if(i==0)
    inst.chan[buf[1]].state = 0;
}

void experiment(unsigned char* buf)
{
  if(buf[1]==0)
  {
    inst.experimentRunning = false;
    all_off();
    adjustoutputaccordingtostate();
  }
  if(buf[1]==1)
  {
    inst.experimentRunning = true;
  }
}

void check()
{
  int j;
  if(inst.chan[inst.currentChan].enabled==true)
  {
    checkChannel(inst.chan[inst.currentChan]);
    inst.chan[inst.currentChan].checkedRecently = true;
  }
  inst.currentChan++;
  if(inst.currentChan==NUM_CHANNELS)
  {
    inst.currentChan = 0;
    for(j=0;j!=NUM_CHANNELS;j++)
      inst.chan[j].checkedRecently = false;
  }
}

void dataFromSensor(unsigned char* buf)
{
  float value;
  int j,sensor_address,sensor_channel;
  memcpy((char*)&value,(char*)&buf[3],4);
  sensor_address = inst.chan[buf[1]].sensor_address;
  sensor_channel = inst.chan[buf[1]].sensor_channel;
  adjustController(buf[1],value);
  for(j=0;j!=NUM_CHANNELS;j++)
    /**  Is the channel enabled for the experiment.  **/
    if(inst.chan[j].enabled == true)
      /**  If the sensor address and sensor channel are the same as the channel that sent out the request.  **/
      if(inst.chan[j].sensor_address==sensor_address && inst.chan[j].sensor_channel==sensor_channel)
        adjustController(j,value);
}

void adjustController(unsigned char chan_num,float currentValue)
{
  /**  There's a bit in reuqest which will also require modifiying.  **/
  //   If too high.  
  if(currentValue>inst.chan[chan_num].value+inst.chan[chan_num].error)
  {
    //  Check to see if the channel is inverted.
    if(inst.chan[chan_num].inverse==0)
      inst.chan[chan_num].state=0;
    else
      inst.chan[chan_num].state=1;
  }
  //   If too low.
  if(currentValue<inst.chan[chan_num].value-inst.chan[chan_num].error)
  {
    //  Check to see if the channel is inverted.
    if(inst.chan[chan_num].inverse==0)
      inst.chan[chan_num].state=1;
    else
      inst.chan[chan_num].state=0;
  }
  /**  Set the checked variable to show that the channel has been checked.  **/
  inst.chan[chan_num].checkedRecently = true;
}

/**  This method turns off all the output.  **/
void all_off()
{
  int i;
  for(i=0;i!=3;i++)
    inst.chan[i].state = 0;
}

/**  Functions below here you should not modify.  **/
void program(unsigned char* buf)
{
  inst.chan[buf[3]].sensor_address = buf[1];
  inst.chan[buf[3]].sensor_channel = buf[2];
  /**  Convert form char* to float.  **/
  inst.chan[buf[3]].value = *(float*)&buf[4];
  inst.chan[buf[3]].enabled = true;
}

void programb(unsigned char* buf)
{
  inst.chan[buf[1]].inverse = buf[2];
  /**  Convert form char* to float.  **/
  memcpy(&inst.chan[buf[1]].error,&buf[3],4);
}

void clearAll()
{
  int i;
  /**  Populate the channel array.  **/
  for(i=0;i!=NUM_CHANNELS;i++)
  {
    inst.chan[i].sensor_address = 0;
    inst.chan[i].sensor_channel = 0;
    inst.chan[i].value = 0;
    inst.chan[i].error = 0;
    //  This line will need to be changed for different controllers.
    inst.chan[i].outputpin = pins[i];
    inst.chan[i].state = 0;
    inst.chan[i].inverse = 0;
    inst.chan[i].controller_channel = i;
    inst.chan[i].enabled = false;
    inst.chan[i].checkedRecently = false;
  }
  all_off();
  /**  Stop the experiment running on start up.  **/
  inst.experimentRunning = false;
}

void request(unsigned char* buf)
{
  //float value = 0.0;
  unsigned char message[8];
  message[0] = DATA_PACKET;
  message[1] = buf[1];
  message[2] = DEVICE_ADDRESS;
  memcpy((float*)&message[3],(float*)&inst.chan[buf[1]].state,4);
  CAN.sendMsgBuf(buf[2],0,8,message);
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
