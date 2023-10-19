#include <refclk.h>

#define MEMORY_MAP_PATH "/dev/mem"
#define REGISTER_OFFSET 0x2c
#define SHIFT_AMOUNT 9
#define MASK 0x7
#define CLK_MAX 5

static const unsigned int available_rates[] = { 40, 12, 25, 40, 48 };
static const char *rate_names[] = { "xtal", "12", "25", "40", "48" };

static uint8_t* mapped_refclk = NULL;
static int refclk_fd = 0;

static void internal_set_refclk(unsigned int value) {
    unsigned int register_value = *(volatile uint32_t*) (mapped_refclk + REGISTER_OFFSET);

    register_value &= ~(MASK << SHIFT_AMOUNT);
    register_value |= (value << SHIFT_AMOUNT);
    *(volatile uint32_t*) (mapped_refclk + REGISTER_OFFSET) = register_value;
}

int set_reference_clock(unsigned int rate) {
    int id;

    for (id = 1; id < CLK_MAX; id++) {
        if (available_rates[id] == rate) {
            internal_set_refclk(id);
            fprintf(stderr, "set refclk to %uMHz\n", rate);
            return 0;
        }
    }
    internal_set_refclk(CLK_XTAL);
    fprintf(stderr, "couldn't set refclk to %uMHz, using xtal instead\n", rate);
    return -1;
}

int get_reference_clock() {
    unsigned int register_value = *(volatile uint32_t*) (mapped_refclk + REGISTER_OFFSET);
    int id;

    register_value = (register_value >> SHIFT_AMOUNT) & MASK;

    fprintf(stderr, "refclk rates in MHz: ");
    for (id = 0; id < CLK_MAX; id++) {
        fprintf(stderr, (id == register_value) ? "[%s] " : "%s ", rate_names[id]);
    }
    fprintf(stderr, "\n");
    return 0;
}

int open_reference_clock_mapping() {
    refclk_fd = open(MEMORY_MAP_PATH, O_RDWR);
    if (refclk_fd < 0) {
        fprintf(stderr, "couldn't open mmap file");
        return -1;
    }

    mapped_refclk = (uint8_t*) mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, refclk_fd, 0x10000000);
    if (mapped_refclk == MAP_FAILED) {
        perror("mmap error");
        fprintf(stderr, "mmap failed");
        mapped_refclk = NULL;
        close(refclk_fd);
        return -1;
    }
    return 0;
}

void close_reference_clock_mapping() {
    close(refclk_fd);
}
