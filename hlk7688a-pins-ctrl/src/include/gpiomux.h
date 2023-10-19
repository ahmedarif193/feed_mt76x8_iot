#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <mt76x8_ctrl_types.h>

// Type Definitions
typedef struct {
    char *groupName;
    char *functionality[4];
    unsigned int bitShift;
    unsigned int bitmask;
} GPIOMuxConfig;

// Function Prototypes
int configure_gpiomux(char *group, char *func);
int display_gpiomux();
int open_gpio_mapping();
void close_gpio_mapping();

// GPIOMux configurations for mt76x8
static GPIOMuxConfig mt76x8GpioConfig[] = {
    {"i2c",    {"i2c", "gpio", NULL, NULL}, 20, 0x3},
    {"uart0",  {"uart", "gpio", NULL, NULL}, 8, 0x3},
    {"uart1",  {"uart", "gpio", "pwm01", NULL}, 24, 0x3},
    {"uart2",  {"uart", "gpio", "pwm23", NULL}, 26, 0x3},
    {"pwm0",   {"pwm", "gpio", NULL, NULL}, 28, 0x3},
    {"pwm1",   {"pwm", "gpio", NULL, NULL}, 30, 0x3},
    {"refclk", {"refclk", "gpio", NULL, NULL}, 18, 0x1},
    {"spi_s",  {"spi_s", "gpio", NULL, "pwm01_uart2"}, 2, 0x3},
    {"spi_cs1",{"spi_cs1", "gpio", "refclk", NULL}, 4, 0x3},
    {"i2s",    {"i2s", "gpio", "pcm", NULL}, 6, 0x3},
    {"ephy",   {"ephy", "gpio", NULL, NULL}, 34, 0x3},
    {"wled",   {"wled", "gpio", NULL, NULL}, 32, 0x3}
};

#endif // GPIO_CONFIG_H
