



#include <errno.h>
#include <fcntl.h>
// #include <i2c/smbus.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

#include "romimot_hw.h"


const int romiaddr = 0x14; /* The I2C address */

const int readDelay = 100; /* Delay in uS. Needed for the slightly broken SMBUS-ish implemetation on the Romi 32u4 */

const int i2cBusNumber = 1; /* pi's bus connected to the Romi */

const uint8_t romiCmdMotor = 6;
const uint8_t romiCmdEncoder = 39;


// Opens the specified I2C device.  Returns a non-negative file descriptor
// on success, or -1 on failure.
int open_i2c_device(const char * device)
{
  int fd = open(device, O_RDWR);
  if (fd == -1)
  {
    perror(device);
    return -1;
  }
  return fd;
}

/*  implements the slightly-janky SMBUS-ish implemetation on the Romi 32u4
    i2cfd - a file descriptor for the I2C bus
    addr - register address we want to write into
    len - number of bytes to read from the remote memory of the Romi
    buf is the pointer to the buf that we'll read into.
    returns -1 on failure, 0 on success */
int romiRead(int i2cfd, uint8_t addr, uint8_t len, uint8_t* buf) {
  uint8_t wbuf[1] = {addr};
  if (write(i2cfd, wbuf, 1) != 1) {
      printf("failed an I2C setup write");
      return -1;
    }
    usleep(readDelay);
    if (read(i2cfd, buf, len) != len) {
      printf("failed an I2C data read");
      return -1;
    }
    return 0;
}

struct MotorPair romiEncoderRead(int i2cfd) {
  struct MotorPair encVals;

  romiRead(i2cfd, romiCmdEncoder, 4, (uint8_t *)&encVals);
  //printf(" enc was %d %d\n", (int16_t)((buf[1]<<8) + buf[0]), (int16_t)((buf[3]<<8) + buf[2]));
  //printf("enc was %d %d\n", encVals.left, encVals.right);
  return encVals;
}


int romiMotorWrite(int i2cfd, int16_t left, int16_t right) {

  //printf("motor set %d %d\n",left, right);
  uint8_t buf[5];
  buf[0] = romiCmdMotor;
  buf[1] = left;
  buf[2] = left >> 8;
  buf[3] = right;
  buf[4] = right >> 8;
  if (write(i2cfd, buf, 5) != 5) {
      printf("failed send motor write\n");
      return -1;
    }
  // usleep(readDelay);
  //int ret = i2c_smbus_write_block_data(i2cfd, romiCmdMotor, 4, (uint8_t *)&mVals);
  //int ret = i2c_smbus_write_block_data(i2cfd, romiCmdMotor, 4, buf);
  //int ret = i2c_smbus_write_byte_data(i2cfd, romiCmdMotor, 70);
  //ret = i2c_smbus_write_byte_data(i2cfd, romiCmdMotor+1, 0);
  return 0;

}
