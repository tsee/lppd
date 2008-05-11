void destroy_uinput()
{
  fprintf(stderr,"\njoyLOOQ says: \"BYE!\" and winks knowingly ;-D\n");
  if(uinp_fd > 0)
    {
      
      struct input_event event;
      event.type = SYN_CONFIG;
      event.code = 0;
      event.value = 0;
      write(uinp_fd,event,sizeof(event)); // Voodoo?
  
      ioctl (uinp_fd, UI_DEV_DESTROY);
      close (uinp_fd);
    }
  exit(0);
}

int create_uinput()
{
  static struct uinput_user_dev uinp;
  int fd = -1;
  char buf[256];

  sigset(SIGINT,destroy_uinput);
  sigset(SIGTERM,destroy_uinput);
  
  int i=0;
  system("modprobe -v uinput");
  
  while(++i < 3)
    {
      fprintf(stderr,"uinput? ... ");
      fd = open ("/dev/input/uinput", O_WRONLY);
      if (fd <= 0)
        fd = open ("/dev/uinput", O_WRONLY);
      if (fd <= 0)
          sleep(i);
      else
        break;
    }
  if(fd < 0)
    {
      fprintf(stderr," Say, are you really root? \n");
      exit(EXIT_FAILURE);
    }
  else
    fprintf(stderr,"OK! ");

  memset (&uinp, 0, sizeof (struct uinput_user_dev));

  strncpy (uinp.name, "joyLOOQ from <ja@email-addr-hidden>",
UINPUT_MAX_NAME_SIZE);
  
  uinp.id.version = 4;
  uinp.id.bustype = BUS_USB;
  uinp.id.vendor = 0x00046d; // Logitech
  uinp.id.product = 0;

  ioctl (fd, UI_SET_EVBIT, EV_SYN);
  ioctl (fd, UI_SET_EVBIT, EV_ABS);
  ioctl (fd, UI_SET_EVBIT, EV_KEY);
  

  ioctl (fd, UI_SET_ABSBIT, ABS_X);
  ioctl (fd, UI_SET_ABSBIT, ABS_Y);
  ioctl (fd, UI_SET_ABSBIT, ABS_Z);
  ioctl (fd, UI_SET_ABSBIT, ABS_RUDDER);
  
  ioctl (fd, UI_SET_KEYBIT, BTN_TOP);
  ioctl (fd, UI_SET_KEYBIT, BTN_TOP2);
  ioctl (fd, UI_SET_KEYBIT, BTN_BASE);
  ioctl (fd, UI_SET_KEYBIT, BTN_BASE2);
  ioctl (fd, UI_SET_KEYBIT, BTN_BASE3);
  ioctl (fd, UI_SET_KEYBIT, BTN_BASE4);
  
  for(i = 0; i < ABS_MAX;++i)
    {
      uinp.absmax[i] = 511;
      uinp.absmin[i] = -512;
    }
  // The jogwheel is different
  uinp.absmax[ABS_Z] = 2047;
  uinp.absmin[ABS_Z] =-2048;
  // .. and rudder as well
  uinp.absmax[ABS_RUDDER] = 255;
  uinp.absmin[ABS_RUDDER] =-256;

  // Create device
  write (fd, &uinp, sizeof (uinp));

  if (ioctl (fd, UI_DEV_CREATE))
    {
      fprintf (stderr,"Unable to create UINPUT device.\n");
      exit(EXIT_FAILURE);
    }

  return fd;
} 
