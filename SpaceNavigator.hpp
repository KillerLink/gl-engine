#ifndef __SPACENAVIGATOR_HPP
#define __SPACENAVIGATOR_HPP

class SpaceNavigator {
	private:
		int fd=0; //Device File Descriptor
		bool ready = 0;
		int led_state = 0;	
	public:
		int axes[6];
		int buttons[2];
		float translations[3];
		float rotations[3];
		SpaceNavigator();
		~SpaceNavigator();
		void init(void);
		void init(char* dev_path);
		void poll(void);
		bool set_led (int led_state);
		bool is_ready(void);
		void compute(void);
		void reset_state(void);

};


#endif
