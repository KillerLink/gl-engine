#ifndef __SPACENAVIGATOR_HPP
#define __SPACENAVIGATOR_HPP

class SpaceNavigator {
	private:
		int fd=0; //Device File Descriptor
		int ready = 0;
		int led_state = 0;	
	public:
		int axes[6];
		int buttons[2];
		SpaceNavigator();
		~SpaceNavigator();
		void init(void);
		void init(char* dev_path);
		void poll(void);
		int set_led (int led_state);
		void reset_state(void);

};


#endif
