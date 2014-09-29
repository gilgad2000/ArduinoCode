// demo: CAN-BUS Shield, send data
#include <mcp_can.h>
#include <SPI.h>

#define DEVICE_ADDRESS 0x0000000B
#define BLOCK 0xFFFFFFFF

int i;
unsigned char stmp[8];
char command[11];

void setup()
{
  Serial.begin(115200);
  // init can bus, baudrate: 500k
  if(CAN.begin(CAN_500KBPS) ==CAN_OK) Serial.print("can init ok!!\r\n");
  else Serial.print("Can init fail!!\r\n");
  /**  Set up the filter so give it an address.  **/
  CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
  CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
  CAN.init_Filt(0,0,DEVICE_ADDRESS);
  CAN.init_Filt(1,0,BLOCK);
  CAN.init_Filt(2,0,BLOCK);
  CAN.init_Filt(3,0,BLOCK);
  CAN.init_Filt(4,0,BLOCK);
  CAN.init_Filt(5,0,BLOCK);
  for(i=0;i!=8;i++)
    stmp[i]=0;
}


void loop()
{
  Serial.readBytesUntil('\n',&command[0],10);
  Serial.println(command);
  if(command[0]=='1')
  {
    Serial.println("Turn LED on.");
    /**  Turn channel 0 on.  **/
    stmp[0] = 'C';
    stmp[1] = 0;
    stmp[2] = 1; //  Control bit.
    stmp[3] = 0;
    stmp[4] = 0;
    stmp[5] = 0;
    stmp[6] = 0;
    stmp[7] = 0;
    CAN.sendMsgBuf(0x0A, 0, 8, stmp);
    for(i=0;i!=8;i++)
      Serial.print(stmp[i]);
  }
  if(command[0]=='0')
  {
    Serial.println("Turn LED off.");
    /**  Turn channel 0 on.  **/
    stmp[0] = 'C';
    stmp[1] = 0;
    stmp[2] = 0;  //   Control bit.
    stmp[3] = 0;
    stmp[4] = 0;
    stmp[5] = 0;
    stmp[6] = 0;
    stmp[7] = 0;
    CAN.sendMsgBuf(0x0A, 0, 8, stmp);
  }
  for(i=0;i!=10;i++)
      command[i] = 0;
}

