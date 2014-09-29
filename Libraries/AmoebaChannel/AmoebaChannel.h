
class AmoebaChannel
{
	private:
		unsigned char sensorAddress;
		unsigned char sensorChannel;
		float value;
		float error;
	public:
		AmoebaChannel();
		~AmoebaChannel();
		void setSensorAddress(unsigned char address);
		void setSensorChannel(unsigned char channel);
		void setValue(float value);
		void setError(float error);
		
		void getSensorAddress(unsigned char address);
		void getSensorChannel(unsigned char channel);
		void getValue(float value);
		void getError(float error);
		
		virtual controlParameter();
}
