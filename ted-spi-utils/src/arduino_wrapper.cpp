#include "arduino_wrapper.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>

static const uint32_t RALINK_REG_DATA_OFFSETS[] = { RALINK_REG_PIODATA, RALINK_REG_PIODATA + 0x4, RALINK_REG_PIODATA + 0x8 };
static const uint32_t RALINK_REG_DIR_OFFSETS[] = { RALINK_REG_PIODIR, RALINK_REG_PIODIR + 0x4, RALINK_REG_PIODIR + 0x8 };
static const uint32_t RALINK_REG_SET_OFFSETS[] = { RALINK_REG_PIOSET, RALINK_REG_PIOSET + 0x4, RALINK_REG_PIOSET + 0x8 };
static const uint32_t RALINK_REG_RESET_OFFSETS[] = { RALINK_REG_PIORESET, RALINK_REG_PIORESET + 0x4, RALINK_REG_PIORESET + 0x8 };

static uint8_t* gpio_mmap_reg = NULL;
static int gpio_mmap_fd = 0;

int wrapperSetup() {
    if ((gpio_mmap_fd = open(MMAP_PATH, O_RDWR)) < 0) {
        perror("Failed to open mmap file");
        return -1;
    }

    gpio_mmap_reg = (uint8_t*) mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10000000);
    if (gpio_mmap_reg == MAP_FAILED) {
        perror("Failed to mmap");
        gpio_mmap_reg = NULL;
        close(gpio_mmap_fd);
        return -1;
    }

    return 0;
}

static inline uint32_t get_register_offset(const uint32_t pin, const uint32_t offsets[3]) {
    return offsets[pin / 32];
}

int digitalRead(int pin) {
    uint32_t tmp = *(volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_DATA_OFFSETS));
    return (tmp >> (pin % 32)) & 1u;
}

void pinMode(int pin, int is_output) {
    uint32_t mask = (1u << (pin % 32));
    volatile uint32_t *reg = (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_DIR_OFFSETS));
    if (is_output) {
        *reg |= mask;
    } else {
        *reg &= ~mask;
    }
}

void digitalWrite(int pin, int value) {
    uint32_t mask = (1u << (pin % 32));
    volatile uint32_t *reg = value ? (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_SET_OFFSETS))
                                   : (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_RESET_OFFSETS));
    *reg = mask;
}

void delayMicroseconds(unsigned int us) {
    usleep(us);
}

/**The constrain function is commonly used in the Arduino ecosystem to 
 * constrain a number between a minimum and maximum value. The error message 
 * you're seeing suggests that the constrain function hasn't been declared 
 * in the scope of the file or the environment you're compiling against.*/
double constrain(double x, double a, double b) {
    if (x < a) return a;
    else if (x > b) return b;
    else return x;
}