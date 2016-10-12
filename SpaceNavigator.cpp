#include "SpaceNavigator.hpp"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/input.h>
#include <glm/glm.hpp>


#define test_bit(bit, array)  (array [bit / 8] & (1 << (bit % 8)))
#define PRESCALER 0.017f
#define POSTSCALER 0.017f
#define THRESHOLD 180.0f

SpaceNavigator::SpaceNavigator() {
	fd=0;
	reset_state();
	ready=false;
}

SpaceNavigator::~SpaceNavigator() {
	ready=false;
	if (fd!=0) {
		close (fd);
	}
}

void SpaceNavigator::init(void) {
	char x[] = "/dev/input/by-id/usb-3Dconnexion_SpaceNavigator_for_Notebooks-event-if00";
	init(x);
}

void SpaceNavigator::init(char* dev_path) {
	if (ready) {
		if (fd!=0) {
			close(fd);
		}
		ready=false;
	}
	char name[256]= "SNAV::Unknown";
	u_int8_t evtype_bitmask[(EV_MAX + 7) / 8];
	u_int8_t rel_bitmask[(REL_MAX + 7) / 8];
	u_int8_t led_bitmask[(LED_MAX + 7) / 8];
	fd = open (dev_path, O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		perror ("SNAV::opening the device failed");
		ready=false;
		return;
	}
	if (ioctl (fd, EVIOCGNAME (sizeof (name)), name) < 0) {
		perror ("SNAV::EVIOCGNAME ioctl failed");
		ready=false;
		return;
	}
	printf ("SNAV::found \"%s\" on %s\n", name, dev_path);
	if (ioctl (fd, EVIOCGBIT (0, sizeof (evtype_bitmask)), evtype_bitmask) < 0) {
		perror ("EVIOCGBIT ioctl failed");
		ready=false;
		return;
	}
	printf ("SNAV::Supported event types:\n");
	for (int i = 0; i < EV_MAX; i++) {
		if (test_bit (i, evtype_bitmask)) {
			/* this means that the bit is set in the event types list */
			printf("  Event type 0x%02x ", i);
			switch (i) {
				case EV_SYN :
					printf(" (Sync?)\n");
					break;
				case EV_KEY :
					printf(" (Keys or Buttons)\n");
					break;
				case EV_REL :
					printf(" (Relative Axes)\n");
					break;
				case EV_ABS :
					printf(" (Absolute Axes)\n");
					break;
				case EV_LED :
					printf(" (LEDs)\n");
					break;
				case EV_REP :
					printf(" (Repeat)\n");
					break;
				default:
					printf(" (Unknown event type)\n");
			}
		}
	}
	memset (rel_bitmask, 0, sizeof (rel_bitmask));
	memset (led_bitmask, 0, sizeof (led_bitmask));
	if (ioctl (fd, EVIOCGBIT (EV_LED, sizeof (led_bitmask)), led_bitmask) < 0) {
		perror ("EVIOCGBIT ioctl failed");
		exit (1);
	}
	printf ("detected leds:\n  ");
	for (int i = 0; i < LED_MAX; i++) {
		if (test_bit (i, led_bitmask))
		printf ("%d, ", i);
	}
	printf ("\n");
	axes[0] = axes[1] = axes[2] = axes[3] = axes[4] = axes[5] = 0;
	buttons[0] = buttons[1] = 0;
	ready=true;
}

void SpaceNavigator::poll(void) {
	if (!ready) {
		return;
	}
	struct input_event event;
	for (int i=0; i<100; i++) {
		if (read (fd, &event, sizeof (struct input_event)) < 0) {
			//perror ("my read error");
			return;
		}
		switch (event.type) {
			case EV_REL:
				if (event.code <= REL_RZ)
				axes[event.code - REL_X] = event.value;
				break;
			case EV_KEY:
				if (event.code >= BTN_0 && event.code <= BTN_1) {
					buttons[event.code - BTN_0] = event.value;
				}
				if (event.code - BTN_0 == 1) {
					led_state = 1 - led_state;
					set_led (led_state);
				}
				break;
			case EV_SYN:
				/* if multiple axes change simultaneously the linux
				* input system sends multiple EV_REL events. EV_SYN
				* then indicates that all changes have been reported.
				*/
				fprintf (stderr,"\rState: %4d %4d %4d %4d %4d %4d - %3s %3s",
					axes[0], axes[1], axes[2], axes[3], axes[4], axes[5],
					buttons[0] ? "on" : "off",
					buttons[1] ? "on" : "off");
				//axes[0] = axes[1] = axes[2] = axes[3] = axes[4] = axes[5] = 0;
				break;
			case EV_LED:
				break;
			default:
				fprintf (stderr, "\nunknown event type %d\n", event.code);
		}
	}
}

bool SpaceNavigator::set_led (int led_state) {
	/* this does not seem to work with 2.6.19, it apparently
	* has worked with 2.6.8. Needs further investigation  */
	if (!ready) {
		return false;
	}
	struct input_event event;
	int ret;
	event.type  = EV_LED;
	event.code  = LED_MISC;
	event.value = led_state;
	ret = write (fd, &event, sizeof (struct input_event));
	if (ret < 0) {
		perror ("SNAV::setting led state failed");
	}
	return ret < sizeof (struct input_event);
}

bool SpaceNavigator::is_ready(void) {
	return ready;
}

void SpaceNavigator::reset_state(void) {
	translations[0]=0.1f;
	translations[1]=0.2f;
	translations[2]=-5.0f;
	rotations[0]=0.3f;
	rotations[1]=-0.3f;
	rotations[2]=0.0f;
}

//TODO: Refactor this into own general input orientated class;

void SpaceNavigator::compute(void) {
	for (int i=0; i<3; i++) {
		if (axes[i]>THRESHOLD) {
			translations[i]+= POSTSCALER*pow(PRESCALER * (axes[i] - THRESHOLD),3);
		}
		if (axes[i]<-THRESHOLD) {
			translations[i]-= POSTSCALER*pow(PRESCALER * (-axes[i] - THRESHOLD),3);
		}
	}
	for (int i=0; i<3; i++) {
		if (axes[i+3]>THRESHOLD) {
			rotations[i]+= POSTSCALER*pow(PRESCALER * (axes[i+3] - THRESHOLD),3);
		}
		if (axes[i+3]<-THRESHOLD) {
			rotations[i]-= POSTSCALER*pow(PRESCALER * (-axes[i+3] - THRESHOLD),3);
		}
	}
	if (buttons[0]) {
		reset_state();
	}
	
	if (buttons[1]) {
		set_led(1-led_state);
	}
}
