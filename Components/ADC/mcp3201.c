#include <stdio.h>
#include <stdlib.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <time.h>

#define CHANNEL 0        // SPI channel (CE0)
#define SPEED   1000000  // 1 MHz SPI clock

// Helper to get current time in milliseconds
long long current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000);
}

int main(void) {
    int fd;
    unsigned char buffer[2];
    long long start, now;
    FILE *file;

    // Initialize SPI
    fd = wiringPiSPISetup(CHANNEL, SPEED);
    if (fd < 0) {
        perror("SPI setup failed");
        return 1;
    }

    // Open CSV file
    file = fopen("data.csv", "w");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    fprintf(file, "time_ms,value\n");
    printf("Starting 10-second data capture...\n");

    start = current_time_ms();
    now = start;

    while ((now - start) < 10000) {  // 10 seconds
        buffer[0] = 0x00;
        buffer[1] = 0x00;

        wiringPiSPIDataRW(CHANNEL, buffer, 2);

        // Extract 12-bit value from MCP3201
        int value = ((buffer[0] & 0x1F) << 7) | (buffer[1] >> 1);

        long long t = now - start;
        fprintf(file, "%lld,%d\n", t, value);

        usleep(100); // ~10 kHz sampling rate (adjust as needed)
        now = current_time_ms();
    }

    fclose(file);
    printf("Done. Data saved to data.csv\n");
    return 0;
}
