// LinuxGPIO.h
#ifndef LINUXGPIO_H
#define LINUXGPIO_H

#include <stdint.h>
#include <stdio.h>

// Defines
#define MMAP_PATH  "/dev/mem"
#define INPUT    0
#define OUTPUT   1
#define LOW 0
#define HIGH 1

#define RALINK_REG_PIODATA    0x620
#define RALINK_REG_PIODIR     0x600
#define RALINK_REG_PIOSET     0x630
#define RALINK_REG_PIORESET   0x640

// Function prototypes
int wrapperSetup();
int digitalRead(int pin);
void pinMode(int pin, int is_output);
void digitalWrite(int pin, int value);
void delayMicroseconds(unsigned int us);
double constrain(double x, double a, double b);

#endif // LINUXGPIO_H
