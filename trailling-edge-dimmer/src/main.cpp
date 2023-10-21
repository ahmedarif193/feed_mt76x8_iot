#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

#include <gpiod.h> 

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

#define MMAP_PATH  "/dev/mem"

#define RALINK_GPIO_DIR_IN    0
#define RALINK_GPIO_DIR_OUT   1

#define RALINK_REG_PIODATA    0x620
#define RALINK_REG_PIODIR     0x600
#define RALINK_REG_PIOSET     0x630
#define RALINK_REG_PIORESET   0x640

#define SYNC_PIN 3
#define REG_LOCK_PIN 2
#define REG_CLK_PIN 1
#define REG_DATA_PIN 0

#define O0_PIN 42
#define O1_PIN 41
#define O2_PIN 16
#define O3_PIN 17
#define O4_PIN 20
#define O5_PIN 21
#define O6_PIN 22
#define O7_PIN 23

#define LOW 0
#define HIGH 1
#define DEBUG 1

#define OUTPUT_MULTIPLIER (6000 / 255)
#define OUTPUT_RISEFALL_MS 5

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

struct gpiod_line *sync_line;

struct gpiod_chip *chip;
std::atomic<bool> run_monitoring(true);

enum InputType { Rise = 1,
                 Fall = 2,
                 Mirror = 3,
                 Multiway = 4,
                 None = 5 };

struct ContextOutput {
  unsigned long value = 0;
  int pin;
  bool state;
};
struct ContextInput {
  unsigned long lastchange = 0;
  bool lastValue = false;
  bool value = false;
  int map = 0;
  InputType type = InputType::None;
};
ContextOutput contextOutput[8];
ContextInput contextInput[8];
volatile unsigned long pastSyncUs = 0;

unsigned long micros() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000L + ts.tv_nsec / 1000L; // Convert seconds to microseconds and nanoseconds to microseconds
}
unsigned long millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L; // Convert seconds to milliseconds and nanoseconds to milliseconds
}

void HandlerInput_sub_routine() {
  static int pos = 0;
  pos = pos >= 7 ? 0 : pos + 1;
  switch (contextInput[pos].type) {
    case InputType::Rise:
      if (contextInput[pos].value == true) {
        if (contextInput[pos].lastchange + OUTPUT_RISEFALL_MS < millis()) {
          contextInput[pos].lastchange = millis();
          auto mmap = contextInput[pos].map;
          if (mmap > -1) {
            if (contextOutput[mmap].value < 254)
              contextOutput[mmap].value += 1;
          }
        }
      }
      break;
    case InputType::Fall:
      if (contextInput[pos].value == true) {
        if (contextInput[pos].lastchange + OUTPUT_RISEFALL_MS < millis()) {
          contextInput[pos].lastchange = millis();
          auto mmap = contextInput[pos].map;
          if (mmap > -1) {
            if (contextOutput[mmap].value > 1)
              contextOutput[mmap].value -= 1;
          }
        }
      }
      break;
    case InputType::Mirror:
      {
        auto mmap = contextInput[pos].map;
        contextOutput[mmap].value = contextInput[pos].value ? 255 : 0;
      }
      break;
    case InputType::Multiway:
      if (contextInput[pos].lastValue != contextInput[pos].value) {

        contextInput[pos].lastValue = contextInput[pos].value;

        auto mmap = contextInput[pos].map;
        if (contextOutput[mmap].value > 250)
          contextOutput[mmap].value = 0;
        else
          contextOutput[mmap].value = 255;
      }
      break;
  }
}
void HAL_input_sub_routine() {
    static int pos = 0;
    constexpr char HAL_INPUT_MIRROR[] = { 3, 2, 1, 0, 7, 6, 5, 4 };
    
    {
        contextInput[HAL_INPUT_MIRROR[pos]].value = mt76x8_gpio_get_pin(REG_DATA_PIN);

        pos = pos >= 7 ? 0 : pos + 1;
        
        mt76x8_gpio_set_pin_value(REG_CLK_PIN, 1);
        mt76x8_gpio_set_pin_value(REG_CLK_PIN, 0);
        if (pos == 0) {
            mt76x8_gpio_set_pin_value(REG_CLK_PIN, 0);
            mt76x8_gpio_set_pin_value(REG_CLK_PIN, 1);
        }
    }
}
void syncContext2Output() {
    auto currentUs = micros();
    for (int i = 0; i < 8; i++) {
        if (contextOutput[i].state == HIGH) {
            if (currentUs >= pastSyncUs + (contextOutput[i].value * OUTPUT_MULTIPLIER)) {
                contextOutput[i].state = LOW;

                // Set the contextOutput[i].pin's value using libgpiod
                mt76x8_gpio_set_pin_value(contextOutput[i].pin, contextOutput[i].state);
            }
        }
    }
}
void zeroCrossCb() {
    int val = gpiod_line_get_value(sync_line);

    if (val == 1) {
        for (int i = 0; i < 8; i++) {
            auto mmap = contextInput[i].map;
            if (mmap > -1) {
                contextOutput[mmap].state = HIGH;
                mt76x8_gpio_set_pin_value(contextOutput[mmap].pin, contextOutput[mmap].state);
            }
        }
    }
}
void sync_pin_monitoring_thread() {
    unsigned long lastRiseTimeMillis = 0;
    unsigned long lastRiseTimeMicros = 0;
    int previous_val = -1;

    while (run_monitoring) {
        int val = gpiod_line_get_value(sync_line);
        
        if (val == 1 && previous_val != 1) {
#if DEBUG
            unsigned long currentTimeMillis = millis();
            unsigned long currentTimeMicros = micros();

            if (lastRiseTimeMillis != 0) {
                unsigned long latencyMillis = currentTimeMillis - lastRiseTimeMillis;
                unsigned long latencyMicros = currentTimeMicros - lastRiseTimeMicros;

                float frequency = 1000.0f / latencyMillis;

                std::cout << "Pulse Frequency: " << frequency << " Hz" << std::endl;
                std::cout << "Latency in milliseconds: " << latencyMillis << " ms" << std::endl;
                std::cout << "Latency in microseconds: " << latencyMicros << " µs" << std::endl;
            }

            lastRiseTimeMillis = currentTimeMillis;
            lastRiseTimeMicros = currentTimeMicros;
#endif
        }
        else if (val == 0 && previous_val == 1) { // Detecting fall transition (1 to 0)
            zeroCrossCb();
        }

        previous_val = val; // Store the current value for the next iteration
        std::this_thread::sleep_for(std::chrono::microseconds(500));  // Sleep for a short while before checking again. Adjust sleep time as necessary.
    }
}
void setup_libgpiod() {
    mt76x8_gpio_set_pin_direction(REG_LOCK_PIN, RALINK_GPIO_DIR_OUT);
    mt76x8_gpio_set_pin_direction(REG_CLK_PIN, RALINK_GPIO_DIR_OUT);
    mt76x8_gpio_set_pin_direction(REG_DATA_PIN, RALINK_GPIO_DIR_IN);

    sync_line = gpiod_chip_get_line(chip, SYNC_PIN);
    if (!sync_line) {
        std::cerr << "Failed to get SYNC_PIN line" << std::endl;
        goto exit_error_setup;
    }
    gpiod_line_request_input(sync_line, "interrupt");

    // Initialize output lines
    for (int i = 0; i < 8; i++) {
        mt76x8_gpio_set_pin_direction(contextOutput[i].pin, RALINK_GPIO_DIR_OUT);
    }

    // Setting REG_CLK_PIN to LOW
    mt76x8_gpio_set_pin_value(REG_CLK_PIN, 0);

    // Setting REG_LOCK_PIN to HIGH
    mt76x8_gpio_set_pin_value(REG_LOCK_PIN, 1);

    return;  // Successfully initialized

exit_error_setup:
    gpiod_chip_close(chip);
    exit(1); // or handle error appropriately
}
void sigint_handler(int signo) {
    if (signo == SIGINT) {
        run_monitoring = false;
    }
}
void setThreadPriority(std::thread &thread, int priority) {
    sched_param sch_params;
    sch_params.sched_priority = priority;

    if(pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &sch_params)) {
        std::cerr << "Failed to set thread priority" << std::endl;
    }
}

int main() {
    gpio_mmap();
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        std::cerr << "Failed to set up signal handler" << std::endl;
        exit(1);
    }

    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        std::cerr << "Failed to open GPIO chip" << std::endl;
        exit(1); // or handle error appropriately
    }
    setup_libgpiod();

    contextOutput[0].pin = O0_PIN;
    contextOutput[1].pin = O1_PIN;
    contextOutput[2].pin = O2_PIN;
    contextOutput[3].pin = O3_PIN;
    contextOutput[4].pin = O4_PIN;
    contextOutput[5].pin = O5_PIN;
    contextOutput[6].pin = O6_PIN;
    contextOutput[7].pin = O7_PIN;

    contextInput[0].type = InputType::Rise;
    contextInput[0].map = 0;
    contextInput[0].value = 200;
    contextInput[1].type = InputType::Fall;
    contextInput[1].map = 0;
    contextInput[1].value = 200;

    contextInput[2].type = InputType::Multiway;
    contextInput[2].map = 1;
    contextInput[3].type = InputType::Multiway;
    contextInput[3].map = 1;
    
    std::thread sync_monitor_thread(sync_pin_monitoring_thread);
    setThreadPriority(sync_monitor_thread, 5);  
    
    while (run_monitoring) {
        HAL_input_sub_routine();
        HandlerInput_sub_routine();
        syncContext2Output();
        std::this_thread::sleep_for(std::chrono::nanoseconds(5)); 
    }

    sync_monitor_thread.join();
    gpiod_line_release(sync_line);
    gpiod_chip_close(chip);
    return 0;
}

