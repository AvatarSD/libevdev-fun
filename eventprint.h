#ifndef EVENTPRINT_H
#define EVENTPRINT_H


int print_sync_event(struct input_event *ev);
int print_event(struct input_event *ev);
void print_bits(struct libevdev *dev);
void print_props(struct libevdev *dev);

#endif // EVENTPRINT_H
