#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <stdbool.h>
#define MMAP_PATH  "/dev/mem"

#define RALINK_GPIO_DIR_IN    0
#define RALINK_GPIO_DIR_OUT   1

#define RALINK_REG_PIODATA    0x620
#define RALINK_REG_PIODIR     0x600
#define RALINK_REG_PIOSET     0x630
#define RALINK_REG_PIORESET   0x640

static const uint32_t RALINK_REG_DATA_OFFSETS[] = { RALINK_REG_PIODATA, RALINK_REG_PIODATA + 0x4, RALINK_REG_PIODATA + 0x8 };
static const uint32_t RALINK_REG_DIR_OFFSETS[] = { RALINK_REG_PIODIR, RALINK_REG_PIODIR + 0x4, RALINK_REG_PIODIR + 0x8 };
static const uint32_t RALINK_REG_SET_OFFSETS[] = { RALINK_REG_PIOSET, RALINK_REG_PIOSET + 0x4, RALINK_REG_PIOSET + 0x8 };
static const uint32_t RALINK_REG_RESET_OFFSETS[] = { RALINK_REG_PIORESET, RALINK_REG_PIORESET + 0x4, RALINK_REG_PIORESET + 0x8 };

static uint8_t* gpio_mmap_reg = NULL;
static int gpio_mmap_fd = 0;

static int gpio_mmap(void) {
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

int mt76x8_gpio_get_pin(int pin) {
    uint32_t tmp = *(volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_DATA_OFFSETS));
    return (tmp >> (pin % 32)) & 1u;
}

void mt76x8_gpio_set_pin_direction(int pin, int is_output) {
    uint32_t mask = (1u << (pin % 32));
    volatile uint32_t *reg = (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_DIR_OFFSETS));
    if (is_output) {
        *reg |= mask;
    } else {
        *reg &= ~mask;
    }
}

void mt76x8_gpio_set_pin_value(int pin, int value) {
    uint32_t mask = (1u << (pin % 32));
    volatile uint32_t *reg = value ? (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_SET_OFFSETS))
                                   : (volatile uint32_t *)(gpio_mmap_reg + get_register_offset(pin, RALINK_REG_RESET_OFFSETS));
    *reg = mask;
}

static bool running = true;

typedef struct {
    int pin;
    int interval; // polling interval in milliseconds
    int lastState;
} PinMonitor;

// Global state and flags.
struct timespec prev_timestamp;
bool first_interrupt = true;

static
void getinfo ()
{
    struct sched_param param;
    int policy;

    sched_getparam(0, &param);
    printf("Priority of this process: %d", param.sched_priority);

    pthread_getschedparam(pthread_self(), &policy, &param);

    printf("Priority of the thread: %d, current policy is: %d and should be %d \n",
              param.sched_priority, policy, SCHED_FIFO);
}

// IRQ Handler function.
void gpio_irq_handler(int pin) {
    struct timespec curr_timestamp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &curr_timestamp);

    // Calculate the time difference and then subtract 1 second (in nanoseconds).
    long diff_in_ns = (curr_timestamp.tv_sec - prev_timestamp.tv_sec) * 1e9 + 
                      (curr_timestamp.tv_nsec - prev_timestamp.tv_nsec) - 1e9; // Subtracting 1 second.

    prev_timestamp = curr_timestamp; // Update the previous timestamp to the current one before displaying.

    printf("Latency from expected 1 second: %ld ns\n", diff_in_ns);
}

// Thread function to monitor GPIO pin state.
void* pin_monitor_thread(void* arg) {
    struct sched_param param;

    // Lock memory to ensure no swapping is done.
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
        perror("mlockall failed");
        return -1;
    }

    // Set real-time priority to the main thread
    param.sched_priority = 50; // Mid range priority, adjust as needed
    if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) == -1) {
        perror("pthread_setschedparam failed");
        return -1;
    }
	getinfo ();
    PinMonitor* monitor = (PinMonitor*)arg;

    while (running) {
        int currentState = mt76x8_gpio_get_pin(monitor->pin);
        if (currentState != monitor->lastState) {
            gpio_irq_handler(monitor->pin);
            monitor->lastState = currentState;
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (gpio_mmap()) {
        return -1;
    }
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
        perror("mlockall failed");
        return -1;
    }
    // Initialize pin monitor data.
    PinMonitor monitor = {
        .pin = 1, // This can be changed as required.
        .interval = 100, 
        .lastState = mt76x8_gpio_get_pin(1)
    };

    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
	getinfo ();
    // Starting the pin monitor thread.
    pthread_t thread;
    pthread_create(&thread, &attr, pin_monitor_thread, &monitor);

    // Printing GPIO states for pins 39 to 42.
    for (int i = 39; i <= 42; i++) {
        printf("get pin %d input %d\n", i, mt76x8_gpio_get_pin(i));
    }

    // Configuring pin 2 for output and setting initial value.
    mt76x8_gpio_set_pin_direction(3, RALINK_GPIO_DIR_OUT);
    mt76x8_gpio_set_pin_value(3, 0);

    // Loop to toggle GPIO state with a delay.
    while (1) {
    }

    // Cleanup and exit.
    close(gpio_mmap_fd);
    pthread_attr_destroy(&attr);
    return 0;  // Return value is changed to 0 for successful exit.
}