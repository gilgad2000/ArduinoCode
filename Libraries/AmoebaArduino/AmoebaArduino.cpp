#include "AmoebaArduino.h"
#include <SPI.h>
#include "mcp_can.h"
#include "ArduinoVector.h"

//   These methods handle the receiving of messages.
//   Read the message buffer.
int AmoebaController::readBuffer()
{
	CAN.readMsgBuf(8, buffer);
	return SUCCESS;
}

//   Process the message buffer.
int AmoebaController::processBuffer()
{
	return SUCCESS;
}

//   Process the buffer if it wishes to directly control a parameter.
int AmoebaController::controlParameters()
{
	return SUCCESS;
}

//   This handles the buffer if it concerns programming a channel.
int AmoebaController::programChannel()
{
	return SUCCESS;
}

//   This method handles the packet if it is an experiment control packet.
int AmoebaController::experimentControl()
{
	return SUCCESS;
}

//   This method handles the packet if it is a data request packet.
int AmoebaController::dataRequest()
{
	return SUCCESS;
}

//   This method handles the packet if it is a data packet.
int AmoebaController::dataPacket()
{
	return SUCCESS;
}
		
//  Class constructor.
AmoebaController::AmoebaController()
{
	CAN.begin(CAN_500KBPS);                   // init can bus : baudrate = 500k
	Serial.begin(115200);
}

//  Class destructor.
AmoebaController::~AmoebaController()
{
}

//  Add Channel.
int AmoebaController::addChannel(AmoebaChannel chan);
{
	return SUCCESS;
}

//  Set address filter.
int AmoebaController::setAddress(unsigned char address)
{
	//  Set up the filter so give it an address.
	CAN.init_Mask(0,0,0xFFFFFFFF);  //  Setup mask 0 to filter on all bits.
	CAN.init_Mask(1,0,0xFFFFFFFF);  //  Setup mask 1 to filter on all bits.
	CAN.init_Filt(0,0,address);
	CAN.init_Filt(1,0,BLOCK);
	CAN.init_Filt(2,0,BLOCK);
	CAN.init_Filt(3,0,BLOCK);
	CAN.init_Filt(4,0,BLOCK);
	CAN.init_Filt(5,0,BLOCK);
  
	return SUCCESS;
}

//  Run the controller.
int AmoebaController::run()
{
	return SUCCESS;
}
