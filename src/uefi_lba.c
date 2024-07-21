#include <uefi_lba.h>

void writeFullLBASize(FILE *image)
{
    uint8_t zeroSector[512];

    for(uint8_t i = 0; i < (lbaSize - sizeof zeroSector) / sizeof zeroSector; ++i)
        fwrite(zeroSector, sizeof zeroSector, 1, image);
}
