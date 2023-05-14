

struct MotorPair
{
    int16_t left;
    int16_t right;
};

int              open_i2c_device(const char *device);
int              romiRead(int i2cfd, uint8_t addr, uint8_t len, uint8_t *buf);
struct MotorPair romiEncoderRead(int i2cfd);
int              romiMotorWrite(int i2cfd, int16_t left, int16_t right);

extern const int romiaddr;

extern const int readDelay;

extern const int i2cBusNumber;

extern const uint8_t romiCmdMotor;
extern const uint8_t romiCmdEncoder;
