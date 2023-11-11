#ifndef LINUXSPI_H
#define LINUXSPI_H

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <iostream>
#include <sys/ioctl.h> 

enum BitOrder {
	LSBFIRST = 0,
	MSBFIRST = 1
};

#define SPI_MODE0 0x00  // Place other modes here if needed

#define DATA_SIZE_8BIT 8
class SPISettings {
public:
    SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t dataMode)
        : clock(clock), bitOrder(bitOrder), dataMode(dataMode), dataSize(DATA_SIZE_8BIT) {}

    SPISettings(uint32_t clock, BitOrder bitOrder, uint8_t dataMode, uint32_t dataSize)
        : clock(clock), bitOrder(bitOrder), dataMode(dataMode), dataSize(dataSize) {}

    SPISettings(uint32_t clock)
        : clock(clock), bitOrder(MSBFIRST), dataMode(SPI_MODE0), dataSize(DATA_SIZE_8BIT) {}

    SPISettings()
        : clock(4000000), bitOrder(MSBFIRST), dataMode(SPI_MODE0), dataSize(DATA_SIZE_8BIT) {}

    uint32_t clock;
    BitOrder bitOrder;
    uint8_t dataMode;
    uint32_t dataSize;
private:


    // Friend declaration, assuming there's a corresponding SPIClass somewhere
    friend class SPIClass;
};
class LinuxSPI {
private:
    int fd; // File descriptor for SPI device
    const char* spiDevicePath;
    int cs=-1;
public:
    void begin(){}
    LinuxSPI(const char* devicePath = "/dev/spidev0.1") : spiDevicePath(devicePath), fd(-1) {}
    
    ~LinuxSPI() {
        if (fd >= 0) {
            close(fd);
        }
    }

    void beginTransaction() {
        fd = open(spiDevicePath, O_RDWR);
        if (fd < 0) {
            std::cerr << "Failed to open SPI device." << std::endl;
            return;
        }
    }

    void configureSpi(SPISettings settings) {
        uint8_t mode;
        switch (settings.dataMode) {
            case SPI_MODE0:
                mode = SPI_MODE_0;
                break;
            // TODO: handle other modes if necessary
        }

        uint8_t bits = settings.dataSize;
        uint32_t speed = settings.clock;
        // Assuming MSB is either 0 or 1. Adjust if necessary.
        uint8_t order = (settings.bitOrder == MSBFIRST) ? 0 : 1;

        // Setting SPI parameters
        if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1)
            perror("Can't set SPI mode");
        if (ioctl(fd, SPI_IOC_RD_MODE, &mode) == -1)
            perror("Can't get SPI mode");
        
        if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1)
            perror("Can't set bits per word");
        if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) == -1)
            perror("Can't get bits per word");
        
        if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1)
            perror("Can't set max speed HZ");
        if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) == -1)
            perror("Can't get max speed HZ");

        if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &order) == -1)
            perror("Can't set order");
        if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &order) == -1)
            perror("Can't get order");
    }
    
    void beginTransaction(SPISettings settings) {
        fd = open(spiDevicePath, O_RDWR);
        if (fd < 0) {
            std::cerr << "Failed to open SPI device." << std::endl;
            return;
        }
        configureSpi(settings);
    }

    void beginTransaction(uint8_t pin, SPISettings settings) {
        // Assuming the pin might be used to select a specific SPI device.
        // Construct the SPI device path based on the pin if needed.
        char path[20];
        snprintf(path, sizeof(path), "/dev/spidev0.%d", pin);
        spiDevicePath = path;

        beginTransaction(settings);
    }

    void endTransaction() {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }

    uint8_t transfer(uint8_t data) {
        if (fd < 0) {
            std::cerr << "SPI device not open." << std::endl;
            return 0;
        }

        uint8_t rxBuffer;
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)&data,
            .rx_buf = (unsigned long)&rxBuffer,
            .len = sizeof(uint8_t),
            // Set other parameters if needed...
        };

        int status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (status < 0) {
            std::cerr << "SPI transfer error." << std::endl;
            return 0;
        }

        return rxBuffer;
    }
};

#endif // LINUXSPI_H
