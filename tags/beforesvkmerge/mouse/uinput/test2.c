#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/time.h>
#include <unistd.h>

// Test the uinput module

struct uinput_user_dev   uinp;
struct input_event       event;

int main(void) {

            int ufile, retcode, i;

            ufile = open("/dev/input/uinput", O_WRONLY | O_NDELAY );
            printf("open /dev/input/uinput returned %d.\n", ufile);
            if (ufile == 0) {
                        printf("Could not open uinput.\n");
                        return -1;
            }

            memset(&uinp, 0, sizeof(uinp));
            strncpy(uinp.name, "simulated mouse", 20);
            uinp.id.version = 4;
            uinp.id.bustype = BUS_USB;

            ioctl(ufile, UI_SET_EVBIT, EV_KEY);
            ioctl(ufile, UI_SET_EVBIT, EV_REL);
            ioctl(ufile, UI_SET_RELBIT, REL_X);
            ioctl(ufile, UI_SET_RELBIT, REL_Y);

            for (i=0; i<256; i++) {
                        ioctl(ufile, UI_SET_KEYBIT, i);
            }

            ioctl(ufile, UI_SET_KEYBIT, BTN_MOUSE);

            // create input device in input subsystem
            retcode = write(ufile, &uinp, sizeof(uinp));
            printf("First write returned %d.\n", retcode);

            retcode = (ioctl(ufile, UI_DEV_CREATE));
            printf("ioctl UI_DEV_CREATE returned %d.\n", retcode);
            if (retcode) {
                        printf("Error create uinput device %d.\n", retcode);
                        return -1;
            }

            // NOW DO STUFF !!!!

            for (i=0; i<1; i++) {

                        struct timeval tv1;

                        // move pointer upleft by 5 pixels
                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_REL;
                        event.code = REL_X;
                        event.value = -1e5;
                        write(ufile, &event, sizeof(event));
            
                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_REL;
                        event.code = REL_Y;
                        event.value = -1e5;
                        write(ufile, &event, sizeof(event));
            
                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_SYN;
                        event.code = SYN_REPORT;
                        event.value = 0;
                        write(ufile, &event, sizeof(event));

                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_REL;
                        event.code = REL_X;
                        event.value = 50;
                        write(ufile, &event, sizeof(event));
            
                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_REL;
                        event.code = REL_Y;
                        event.value = 50;
                        write(ufile, &event, sizeof(event));
            
                        memset(&event, 0, sizeof(event));
                        gettimeofday(&event.time, NULL);
                        event.type = EV_SYN;
                        event.code = SYN_REPORT;
                        event.value = 0;
                        write(ufile, &event, sizeof(event));

                        // wait just a moment
                        do { gettimeofday(&tv1, NULL); } while ((tv1.tv_usec & 
0x3FFF) != 0);
                        do { gettimeofday(&tv1, NULL); } while ((tv1.tv_usec & 
0x3FFF) == 0);
            }

            // destroy the device
            ioctl(ufile, UI_DEV_DESTROY);

            close(ufile);

}
