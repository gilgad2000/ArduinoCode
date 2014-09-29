#include <ArduinoVector.h>
#include "AmoebaChannel.h"

//   Used to request Data from a sensor
#define DATA_REQUST  'R'

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

#define SUCCESS 0

class AmoebaController
{
	private:
		//   Data read off CAN Bus.
		unsigned char buffer[8];
		//   This property is true if the experiment is running.
		boolean running;
		//   This property is the channels.
		Vector<AmoebaChannel> channels;
		
		//   These methods handle the receiving of messages.
		//   Read the message buffer.
		int readBuffer();
		//   Process the message buffer.
		int processBuffer();
		//   Process the buffer if it wishes to directly control a parameter.
		int controlParameters();
		//   This handles the buffer if it concerns programming a channel.
		int programChannel();
		//   This method handles the packet if it is an experiment control packet.
		int experimentControl();
		//   This method handles the packet if it is a data request packet.
		int dataRequest();
		//   This method handles the packet if it is a data packet.
		int dataPacket();
		
	public:
		//  Class constructor.
		AmoebaController();
		//  Class destructor.
		~AmoebaController();
		//  Add Channel.
		int addChannel(AmoebaChannel chan);
		//  Set address filter.
		int setAddress(unsigned char address);
		//  Run the controller.
		int run();
		
}