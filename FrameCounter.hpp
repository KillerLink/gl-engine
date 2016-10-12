#ifndef __FRAMECOUNTER_HPP
#define __FRAMECOUNTER_HPP

#include <stdint.h>

class FrameCounter {
	private:
		double tlast,tcurrent;
		uint16_t nframes;
		double tdiff;
	public:
		FrameCounter();
		double tick(void);
		double getTime(void);
		double getFPS(void);		
};

#endif

