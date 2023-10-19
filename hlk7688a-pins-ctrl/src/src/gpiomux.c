#include <gpiomux.h>

static uint8_t* gpioMemReg = NULL;
static int gpioFileDesc = 0;

static void applyGpioMuxSettings(unsigned int mask, unsigned int shift, unsigned int value) {
	volatile uint32_t *targetReg = (volatile uint32_t*)(gpioMemReg + 0x60);

	if (shift > 31) {
		shift -= 32;
		targetReg++;
	}

	uint32_t updatedValue = *targetReg;
	updatedValue &= ~(mask << shift);
	updatedValue |= (value << shift);
	*targetReg = updatedValue;
}

int configure_gpiomux(char *group, char *func) {
	int configIndex;
	for (configIndex = 0; configIndex < _MT76X8_NUM_GPIO_MUX; configIndex++) {
		if (!strcmp(mt76x8GpioConfig[configIndex].groupName, group)) {
			break;
		}
	}

	if (configIndex < _MT76X8_NUM_GPIO_MUX) {
		for (int i = 0; i < 4; i++) {
			if (!mt76x8GpioConfig[configIndex].functionality[i] || strcmp(mt76x8GpioConfig[configIndex].functionality[i], func)) {
				continue;
			}
			applyGpioMuxSettings(mt76x8GpioConfig[configIndex].bitmask, mt76x8GpioConfig[configIndex].bitShift, i);
			fprintf(stderr, "Configured gpiomux %s -> %s\n", mt76x8GpioConfig[configIndex].groupName, func);
			return EXIT_SUCCESS;
		}
	}

	fprintf(stderr, "Invalid group/function combination.\n");
	return EXIT_FAILURE;
}

int display_gpiomux(void) {
	uint32_t regVal1 = *(volatile uint32_t*)(gpioMemReg + 0x60);
	uint32_t regVal2 = *(volatile uint32_t*)(gpioMemReg + 0x64);

	for (int index = 0; index < _MT76X8_NUM_GPIO_MUX; index++) {
		uint32_t extractedVal = (mt76x8GpioConfig[index].bitShift < 32) ?
								(regVal1 >> mt76x8GpioConfig[index].bitShift) & mt76x8GpioConfig[index].bitmask :
								(regVal2 >> (mt76x8GpioConfig[index].bitShift - 32)) & mt76x8GpioConfig[index].bitmask;

		fprintf(stderr, "Group %s - ", mt76x8GpioConfig[index].groupName);
		for (int i = 0; i < 4; i++) {
			if (!mt76x8GpioConfig[index].functionality[i]) {
				continue;
			}
			fprintf(stderr, (i == extractedVal) ? "[%s] " : "%s ", mt76x8GpioConfig[index].functionality[i]);
		}
		fprintf(stderr, "\n");
	}

	return EXIT_SUCCESS;
}

int open_gpio_mapping(void) {
	gpioFileDesc = open(MMAP_PATH, O_RDWR);
	if (gpioFileDesc < 0) {
		fprintf(stderr, "Failed to open memory map file.\n");
		return EXIT_FAILURE;
	}

	gpioMemReg = (uint8_t*)mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, gpioFileDesc, 0x10000000);
	if (gpioMemReg == MAP_FAILED) {
		perror("Memory mapping failed");
		gpioMemReg = NULL;
		close(gpioFileDesc);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void close_gpio_mapping(void) {
	if (gpioFileDesc) {
		close(gpioFileDesc);
	}
}
