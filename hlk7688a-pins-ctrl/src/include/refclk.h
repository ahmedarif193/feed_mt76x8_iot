#ifndef REFCLK_CONFIG_H
#define REFCLK_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mt76x8_ctrl_types.h>

// Function Prototypes
int set_reference_clock(unsigned int rate);
int get_reference_clock(void);

int open_reference_clock_mapping(void);
void close_reference_clock_mapping(void);

#endif // REFCLK_CONFIG_H
