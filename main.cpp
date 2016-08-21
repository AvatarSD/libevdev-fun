
#include <stdio.h>
#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>   /* For open(), creat() */
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include <iostream>
#include <algorithm>
#include <list>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/input.h>

extern "C" {
#include "eventprint.h"
}

int main(int argv, char **argc)
{

	if(argv < 2)
	{
		std::string buff = "/dev/input/";

		DIR *pDIR;
		if( !(pDIR=opendir(buff.c_str())))
			return 1;

		std::list<dirent> files;
		while(dirent * entry = readdir(pDIR))
			files.push_back(*entry);

		files.remove_if([](auto & file)->bool{return !(strcmp(file.d_name, ".") && strcmp(file.d_name, "..") && file.d_type == DT_CHR);});

		std::list<std::string> filesName;
		for (auto & file : files)
			filesName.push_back(file.d_name);

		filesName.sort([](auto& lhs, auto& rhs){return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;});

		for (auto file : filesName)
		{
			std::cout << file << " ";

			file.insert(0, buff);
			int	fd = open(file.c_str(), O_RDONLY|O_NONBLOCK);
			if (fd < 0) {
				perror("Failed to open device\n");
				continue;
			}

			struct libevdev *dev = NULL;
			int rc = libevdev_new_from_fd(fd, &dev);
			if (rc < 0) {
				printf("Failed to init libevdev (%s)\n", strerror(-rc));
				continue;
			}

			printf(" \tbus: %#x   \tvnd: %#x   \tprd: %#x   \tver: %x   \tname: \"%s\"\n",
				   libevdev_get_id_bustype(dev),
				   libevdev_get_id_vendor(dev),
				   libevdev_get_id_product(dev),
				   libevdev_get_driver_version(dev),
				   libevdev_get_name(dev));

			libevdev_free(dev);
			close(fd);
		}
		closedir(pDIR);
		return 0;
	}


	struct libevdev *dev = NULL;

	int	fd = open(argc[1], O_RDWR|O_NONBLOCK);
	if (fd < 0) {
		perror("Failed to open device");
		return 1;
	}

	int rc = libevdev_new_from_fd(fd, &dev);
	if (rc < 0) {
		printf("Failed to init libevdev (%s)\n", strerror(-rc));
		return 1;
	}

	printf("Input device ID: bus %#x vendor %#x product %#x\n",
		   libevdev_get_id_bustype(dev),
		   libevdev_get_id_vendor(dev),
		   libevdev_get_id_product(dev));
	printf("Evdev version: %x\n", libevdev_get_driver_version(dev));
	printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
	printf("Phys location: %s\n", libevdev_get_phys(dev));
	printf("Uniq identifier: %s\n", libevdev_get_uniq(dev));
	print_bits(dev);
	print_props(dev);

	//static bool led = 0;
	//libevdev_kernel_set_led_value(dev, LED_SCROLLL, LIBEVDEV_LED_OFF);


	do {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL|LIBEVDEV_READ_FLAG_BLOCKING, &ev);
		if (rc == LIBEVDEV_READ_STATUS_SYNC) {
			printf("rr\n");//"::::::::::::::::::::: dropped ::::::::::::::::::::::\n");
			while (rc == LIBEVDEV_READ_STATUS_SYNC) {
				print_sync_event(&ev);
				rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
			}
			printf("::::::::::::::::::::: re-synced ::::::::::::::::::::::\n");
		} else if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
			print_event(&ev);

		{
			static int counter = 0;
			if(ev.type == EV_KEY && ev.value == 1)
				switch (ev.code) {
				case KEY_RIGHT:
					counter++;
					break;
				case KEY_LEFT:
					counter--;
					break;
				default:
					break;
				}
			if(counter > 2)
				counter = 0;
			if(counter < 0)
				counter = 2;
			libevdev_kernel_set_led_value(dev, LED_NUML, counter==0 ? LIBEVDEV_LED_ON:LIBEVDEV_LED_OFF);
			libevdev_kernel_set_led_value(dev, LED_CAPSL, counter==1 ? LIBEVDEV_LED_ON:LIBEVDEV_LED_OFF);
			libevdev_kernel_set_led_value(dev, LED_SCROLLL, counter==2 ? LIBEVDEV_LED_ON:LIBEVDEV_LED_OFF);
		}

	} while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS || rc == -EAGAIN);

	return 0;
}

