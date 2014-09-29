// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

#define DEVICE_ADDRESS 0x0000000B
#define BLOCK 0xFFFFFFFF
#define ACCEPT_ALL 0x00000000

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

#define RESET_PIN 9

#define CHARSTOFLOAT *(float*)&

int i;
unsigned char buf[8];
unsigned char recbuf[8];
char command[11];
unsigned char len = 8;

void setup()
{
  Serial.begin(115200);
  Serial.println("Start up");
  pinMode(RESET_PIN, OUTPUT); 
  digitalWrite(RESET_PIN, HIGH);
  
  // init can bus, baudrate: 500k
  if(CAN.begin(CAN_500KBPS) ==CAN_OK) Serial.print("can init ok!!\r\n");
  else Serial.print("Can init fail!!\r\n");
  /**  Set up the filter so give it an address.  **/
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,BLOCK);
  CAN.init_Filt(2,0,BLOCK);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  
  CAN.clearRX0Status();
  CAN.clearRX1Status();
  Serial.println("Started");
}


void loop()
{
    float val = 6.0;
    Serial.println("Send message.");  
    /**  Turn channel 0 on.  **/
    buf[0] = 'R';
    buf[1] = 0;
    buf[2] = 0x0A;  //   Control bit.
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
   if(CAN.readStatus()!=0)
   {
     //  Once new message arrives retrieve it.
     CAN.readMsgBuf(&len, recbuf);
     CAN.clearRX0Status();
     CAN.clearRX1Status();
     //  Reset interrupts.
     //  Print buffer
     Serial.println("Received message.");  
     for(i=0;i!=8;i++)
     {
       Serial.print(recbuf[i]); Serial.print('\t');
     }
     Serial.println();
     
     val = CHARSTOFLOAT recbuf[3];
     Serial.println(val);
     Serial.println();
   }
     
}

