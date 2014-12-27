#include <iostream>

#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>


using namespace std;

void handle_syn_dropped(struct libevdev *dev) {
    struct input_event ev;
    int rc = LIBEVDEV_READ_STATUS_SYNC;

    while (rc == LIBEVDEV_READ_STATUS_SYNC) {
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_SYNC, &ev);
        if (rc < 0) {
            if (rc != -EAGAIN)
                fprintf(stderr, "error %d (%s)\n", -rc, strerror(-rc));
            return;
        }

        printf("State change since SYN_DROPPED for %s %s value %d\n",
                libevdev_event_type_get_name(ev.type),
                libevdev_event_code_get_name(ev.type, ev.code),
                ev.value);
    }
}


void read_device_test() {
    cout << "read device test" << endl;

    struct libevdev *dev;
    int fd;
    int rc;

    fd = open("/dev/input/by-id/usb-093a_USB_OPTICAL_MOUSE-event-mouse", O_RDONLY|O_NONBLOCK);
    if (fd < 0)
        fprintf(stderr, "error fd: %d %s\n", errno, strerror(errno));
    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0)
        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));


    printf("Device: %s\n", libevdev_get_name(dev));
    printf("vendor: %x product: %x\n",
            libevdev_get_id_vendor(dev),
            libevdev_get_id_product(dev));

    if (libevdev_has_event_type(dev, EV_REL) &&
            libevdev_has_event_code(dev, EV_REL, REL_X) &&
            libevdev_has_event_code(dev, EV_REL, REL_Y) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_LEFT) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_MIDDLE) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_RIGHT))
        printf("Looks like we got ourselves a mouse\n");


    struct input_event ev;

    while (true) {


        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc < 0) {
            if (rc != -EAGAIN) {
                fprintf(stderr, "error: %d %s\n", -rc, strerror(-rc));
                break;
            }
        }
//        else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
//            handle_syn_dropped(dev);
//        }
        else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            printf("We have an event!\n%d (%s) %d (%s) value %d\n",
                    ev.type, libevdev_event_type_get_name(ev.type),
                    ev.code, libevdev_event_code_get_name(ev.type, ev.code),
                    ev.value);

        }

    }

    libevdev_free(dev);
    close(fd);
}

void uinput_test() {

    cout << "uinput device test" << endl;
//
//    struct libevdev *dev;
//    struct libevdev_uinput *uidev;
//
//    int fd;
//    int rc;
//    int input_wait;
//
//    dev = libevdev_new();
//    libevdev_set_name(dev, "advanced scroll mouse");
//
//    libevdev_enable_event_type(dev, EV_REL);
//    libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
//    libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
//
//    libevdev_enable_event_type(dev, EV_KEY);
//    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
//    libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, NULL);
//    libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);
//
//    fd = open("/dev/uinput", O_RDWR);
//
//    rc = libevdev_uinput_create_from_device(dev, fd, &uidev);
//    if (rc < 0)
//        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));
//
//    //    don't need the source device anymore
//    libevdev_free(dev);



    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    int rc, fd;

    fd = open("/dev/input/by-id/usb-093a_USB_OPTICAL_MOUSE-event-mouse", O_RDONLY|O_NONBLOCK);
    if (fd < 0)
        fprintf(stderr, "error fd: %d %s\n", errno, strerror(errno));

    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0)
        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));

    rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    if (rc < 0)
        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));

    cout << "before event" << endl;
    usleep(10000);

    libevdev_uinput_write_event(uidev, EV_REL, REL_X, -100);
    libevdev_uinput_write_event(uidev, EV_REL, REL_Y, 100);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

    cout << "after event" << endl;
    usleep(10000);

    libevdev_uinput_destroy(uidev);
}

void uinput_test2() {

    cout << "uinput device test2" << endl;

    int err, input_wait;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;

    dev = libevdev_new();
    libevdev_set_name(dev, "fake keyboard device");

    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, KEY_A, NULL);

    err = libevdev_uinput_create_from_device(dev,
            LIBEVDEV_UINPUT_OPEN_MANAGED,
            &uidev);

    if (err != 0)
        fprintf(stderr, "error rc: %d %s\n", -err, strerror(-err));

    input_wait = getchar();

    libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 1);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
    libevdev_uinput_write_event(uidev, EV_KEY, KEY_A, 0);
    libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

    input_wait = getchar();

    libevdev_uinput_destroy(uidev);
    printf("Complete\n");
}

void grab_test() {
    cout << "grab device test" << endl;

    struct libevdev *dev;
    struct libevdev_uinput *uidev;
    int fd;
    int rc;

    fd = open("/dev/input/by-id/usb-093a_USB_OPTICAL_MOUSE-event-mouse", O_RDONLY|O_NONBLOCK);
    if (fd < 0)
        fprintf(stderr, "error fd: %d %s\n", errno, strerror(errno));

    rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0)
        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));

    rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev);
    if (rc < 0)
        fprintf(stderr, "error rc: %d %s\n", -rc, strerror(-rc));


    printf("Device: %s\n", libevdev_get_name(dev));
    printf("vendor: %x product: %x\n",
            libevdev_get_id_vendor(dev),
            libevdev_get_id_product(dev));

    if (libevdev_has_event_type(dev, EV_REL) &&
            libevdev_has_event_code(dev, EV_REL, REL_X) &&
            libevdev_has_event_code(dev, EV_REL, REL_Y) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_LEFT) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_MIDDLE) &&
            libevdev_has_event_code(dev, EV_KEY, BTN_RIGHT))
        printf("Looks like we got ourselves a mouse\n");


    struct input_event ev;

    int caputred_events = 0;

    rc = libevdev_grab(dev, LIBEVDEV_GRAB);
    if(rc < 0)
        fprintf(stderr, "error grab: %d %s\n", -rc, strerror(-rc));

    while (caputred_events < 1000) {

        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc < 0) {
            if (rc != -EAGAIN) {
                fprintf(stderr, "error: %d %s\n", -rc, strerror(-rc));
                break;
            }
        }
//        else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
//            handle_syn_dropped(dev);
//        }
        else if (rc == LIBEVDEV_READ_STATUS_SUCCESS) {
            printf("We have an event!\n%d (%s) %d (%s) value %d\n",
                    ev.type, libevdev_event_type_get_name(ev.type),
                    ev.code, libevdev_event_code_get_name(ev.type, ev.code),
                    ev.value);

//            usleep(10000);

            if(ev.code == REL_WHEEL) {
                libevdev_uinput_write_event(uidev, ev.type, REL_MISC, ev.value);
            }
            else {
                libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
            }

            libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

//            cout << "after event" << endl;
//            usleep(10000);

            caputred_events++;
        }
    }

    rc = libevdev_grab(dev, LIBEVDEV_UNGRAB);
    if(rc < 0)
        fprintf(stderr, "error ungrab: %d %s\n", -rc, strerror(-rc));

    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
    close(fd);
}

int main() {

    grab_test();
//    uinput_test2();
//    uinput_test();
//    read_device_test();

    cout << "shutdown" << endl;
    return 0;
}