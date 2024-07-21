#include <uefi_lba.h>

void writeFullLBASize(FILE *image) {
    uint8_t zero_sector[512];
    for (uint8_t i = 0; i < (lbaSize - sizeof zero_sector) / sizeof zero_sector; i++)
        fwrite(zero_sector, sizeof zero_sector, 1, image);
}
