#include <stdio.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <errno.h>


#include <libevdev-1.0/libevdev/libevdev.h>

#include <linux/input.h>
//#include <stdarg.h>

//#include <string.h>

#include "eventprint.h"

void
print_abs_bits(struct libevdev *dev, int axis)
{
	const struct input_absinfo *abs;

	if (!libevdev_has_event_code(dev, EV_ABS, axis))
		return;

	abs = libevdev_get_abs_info(dev, axis);

	printf("	Value	%6d\n", abs->value);
	printf("	Min	%6d\n", abs->minimum);
	printf("	Max	%6d\n", abs->maximum);
	if (abs->fuzz)
		printf("	Fuzz	%6d\n", abs->fuzz);
	if (abs->flat)
		printf("	Flat	%6d\n", abs->flat);
	if (abs->resolution)
		printf("	Resolution	%6d\n", abs->resolution);
}

void
print_code_bits(struct libevdev *dev, unsigned int type, unsigned int max)
{
	unsigned int i;
	for (i = 0; i <= max; i++) {
		if (!libevdev_has_event_code(dev, type, i))
			continue;

		printf("    Event code %i (%s)\n", i, libevdev_event_code_get_name(type, i));
		if (type == EV_ABS)
			print_abs_bits(dev, i);
	}
}

void
print_bits(struct libevdev *dev)
{
	unsigned int i;
	printf("Supported events:\n");

	for (i = 0; i <= EV_MAX; i++) {
		if (libevdev_has_event_type(dev, i))
			printf("  Event type %d (%s)\n", i, libevdev_event_type_get_name(i));
		switch(i) {
		case EV_KEY:
			print_code_bits(dev, EV_KEY, KEY_MAX);
			break;
		case EV_REL:
			print_code_bits(dev, EV_REL, REL_MAX);
			break;
		case EV_ABS:
			print_code_bits(dev, EV_ABS, ABS_MAX);
			break;
		case EV_LED:
			print_code_bits(dev, EV_LED, LED_MAX);
			break;
		}
	}
}

void
print_props(struct libevdev *dev)
{
	unsigned int i;
	if (libevdev_has_property(dev, 0))
		printf("Properties:\n");

	for (i = 0; i <= INPUT_PROP_MAX; i++) {
		if (libevdev_has_property(dev, i))
			printf("  Property type %d (%s)\n", i,
				   libevdev_property_get_name(i));
	}
}

int
print_event(struct input_event *ev)
{
	if (ev->type == EV_SYN)
		printf("Event: time %ld.%06ld, ++++++++++++++++++++ %s +++++++++++++++\n",
			   ev->time.tv_sec,
			   ev->time.tv_usec,
			   libevdev_event_type_get_name(ev->type));
	else
		printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
			   ev->time.tv_sec,
			   ev->time.tv_usec,
			   ev->type,
			   libevdev_event_type_get_name(ev->type),
			   ev->code,
			   libevdev_event_code_get_name(ev->type, ev->code),
			   ev->value);
	return 0;
}

int
print_sync_event(struct input_event *ev)
{
	printf("SYNC: ");
	print_event(ev);
	return 0;
}
