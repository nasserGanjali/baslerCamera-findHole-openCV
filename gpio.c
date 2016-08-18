// I2C test program for a PCA9555

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

// I2C Linux device handle
int g_i2cFile;
int isGPIOClosed = 0;
// open the Linux device
void i2cOpen()
{
    g_i2cFile = open("/dev/i2c-0", O_RDWR);
    if (g_i2cFile < 0) {
        perror("i2cOpen");
        exit(1);
    }
}

// close the Linux device
void i2cClose()
{
    close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
void i2cSetAddress(int address)
{
    if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {
        perror("i2cSetAddress");
        exit(1);
    }
}

// write a 16 bit value to a register pair
// write low byte of value to register reg,
// and high byte of value to register reg+1
void pca9555WriteRegisterPair(u_int8_t reg, uint16_t value)
{
    u_int8_t data[3];
    data[0] = reg;
    data[1] = value & 0xff;
    data[2] = (value >> 8) & 0xff;
    if (write(g_i2cFile, data, 3) != 3) {
        perror("pca9555SetRegisterPair");
    }
}

// read a 16 bit value from a register pair
u_int16_t pca9555ReadRegisterPair(uint8_t reg)
{
    uint8_t data[3];
    data[0] = reg;
    if (write(g_i2cFile, data, 1) != 1) {
        perror("pca9555ReadRegisterPair set register");
    }
    if (read(g_i2cFile, data, 2) != 2) {
        perror("pca9555ReadRegisterPair read value");
    }
    return data[0] | (data[1] << 8);
}

// set IO ports to input, if the corresponding direction bit is 1,
// otherwise set it to output
void pca9555SetInputDirection(uint16_t direction)
{
    pca9555WriteRegisterPair(6, direction);
}

// set the IO port outputs
void pca9555SetOutput(uint16_t value)
{
    pca9555WriteRegisterPair(2, value);
}

// read the IO port inputs
uint16_t pca9555GetInput()
{
    return pca9555ReadRegisterPair(0);
}

int GPIO_start()
{
    // test output value
    int v = 2;

    // direction of the LED animation
    int directionLeft = 1;

    // open Linux I2C device
    i2cOpen();

    // set address of the PCA9555
    i2cSetAddress(0x20);

    // set input for IO pin 3,4
    pca9555SetInputDirection(12);

    // LED animation loop
    /*while (1) {
        // if button is pressed, invert output
        int xor;
        //        if (pca9555GetInput() & 0x8000) {
        //            xor = 0;
        //        } else {
        //            xor = 0xffff;
        //        }

        //			xor = 0xffff;
        // set current output

        //		pca9555SetOutput(v ^ xor);

        // animate LED position
        if (directionLeft) {
            v <<= 1;
        } else {
            v >>= 1;
        }
        if (v == 0x6000) {
            directionLeft = 0;
        }
        if (v == 3) {
            directionLeft = 1;
        }

        // wait 100 ms for next animation step
        //	printf("3\n");
        usleep(1000);
    }*/

    // close Linux I2C device
    return 1;
}

int closeGPIO()
{
    i2cClose();

    return 1;
}
/*
 *  pin 1 is LED            (Out)
 *  pin 2 is Camera Trigge  (Out)
 *  pin 3 is metal detector (IN)
 *  pin 4 is wire           (IN)
 */
int getGPIO()
{
    static int pin3LastStatus = 0, pin4LastStatus = 0;
    while( !isGPIOClosed )
    {
        usleep(20000);
        u_int16_t input = pca9555GetInput();
        if (!(input & 0x8))
        {
            if( !pin4LastStatus )
            {
                pin4LastStatus = 1;
                return 4;
            }
        }else
             pin4LastStatus = 0;

        if(!(input & 0x4))
        {
            if( !pin3LastStatus )
            {
                pin3LastStatus = 1;
                return 3;
            }
        }else
             pin3LastStatus = 0;
    }
}

void setGPIO(int value)
{
    pca9555SetOutput(value);
    usleep(100000);
    pca9555SetOutput(0);
}

int pcaTest ;
void testTrigge()
{
    pcaTest = 1;
    while(pcaTest)
    {
        pca9555SetOutput(2);
        usleep(10000);
        pca9555SetOutput(0);
        usleep(300000);
    }
}

void stopTriggeTest(){
    pcaTest = 0;
}

void singleShot(int value)
{
    pca9555SetOutput(value);
    usleep(50000);
    pca9555SetOutput(0);
    printf("singleShot !!! \n");
}








