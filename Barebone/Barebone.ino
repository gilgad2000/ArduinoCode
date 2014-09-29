// demo: CAN-BUS Shield, receive data
#include "mcp_can.h"
#include <SPI.h>
#include <stdio.h>
#define INT8U unsigned char

#define DEVICE_ADDRESS 0x0000000A
#define BLOCK 0xFFFFFFFF

INT8U Flag_Recv = 0;
INT8U len = 8;
INT8U buf[8];
char str[20];
int i;
INT8U state;

int led = 4;

void control(unsigned char* buf);

void setup()
{
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
    buf[i]=0;
  Serial.println("Started");
}

// (CAN.readStatus()&0x80)!=0x80)||((CAN.readStatus()&0x40)!=0x40)   Line preserved for possible future use.

void loop()
{
   //  Wait for new message.
   for(state=0;CAN.readStatus()==0;)
   {}
   //  Once new message arrives retrieve it.
   CAN.readMsgBuf(&len, buf);
   for(i=0;i!=8;i++)
   {
     Serial.print(buf[i]); Serial.print('\t');
   }
   Serial.println();
   //  Reset interrupts.
   CAN.clearRX0Status();
   CAN.clearRX1Status();
   if(buf[0]=='C')
      control(buf);     
   Serial.println();
}

void control(unsigned char* buf)
{  
   if(buf[2]==1)
   {
     Serial.println("Turn LED ON.");
     digitalWrite(led, HIGH);
   }
   if(buf[2]==0)
   {
     Serial.println("Turn LED OFF.");
     digitalWrite(led, LOW); 
   }     
}
