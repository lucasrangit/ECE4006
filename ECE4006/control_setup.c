/*
 * This code takes in four parameters from the command line pan, tilt,
 * panspeed and tiltspeed and moves the camera.
 *
 * Installation
 * 	1. Download libvisca from http://sourceforge.net/projects/libvisca
 * 	2. Extract the file
 * 	3. Run ./configure
 * 	4. Run make
 *
 * I did this on the computers in COC309 and everything worked fine. sample code
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "../src/libvisca.h"

#define EVI_D30


int main(int argc, char **argv)
{

  VISCAInterface_t interface;
  VISCACamera_t camera;
  unsigned char packet[3000];
  int i, bytes, camera_num;
  UInt8_t value;
  UInt16_t zoom;
  int pan_pos, tilt_pos;

  if(argc < 6)
    {
      fprintf(stderr, "%s usage: %s ./test serial_device pan tilt pan_speed tilt_speed \n", argv[0]);
    }
  if(VISCA_open_serial(&interface, argv[1]) != VISCA_SUCCESS)
    {
      fprintf(stderr,"%s: unable to open serial device %s\n", argv[1]);
    }
  pan_pos = atoi(argv[2]);
  tilt_pos = atoi(argv[3]);
  ps = atoi(argv[4]);
  ts = atoi(argv[5]);

  interface.broadcast = 0;
  VISCA_set_address(&interface, &camera_num);
  camera.address = 1;
  VISCA_clear(&interface, &camera);

  if(VISCA_set_pantilt_absoulte_position(&interface, &camera, ps, ts, pan_pos, titl_pos) != VISCA_SUCCESS)
    fprintf(stderr, "error setting absolute position\n");

  return 0;

}
