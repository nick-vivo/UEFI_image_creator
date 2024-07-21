#include <time.h>

#include <stdio.h>   // fopen, fprintf
#include <stdlib.h>  // EXIT_SUCCESS, EXIT_FAILURE

#include <uefi_mbr.h>
#include <uefi_gpt.h>
#include <uefi_lba.h>
#include <uefi_fat32.h>

// =============================
// MAIN
// =============================
int main(void)
{
    FILE *image = fopen(image_name, "wb+");

    if(!image)
    {
        fprintf(stderr, "Error: could not open file: %s", image_name);
        return EXIT_FAILURE;
    }

    // Set sizes & LBA values

    // Add extra padding for:
    //   2 aligned partitions
    //   2 GPT tables
    //   MBR
    //   GPT headers
    gptTableLBAs = GPT_TABLE_SIZE / lbaSize;

    const uint64_t padding = (ALIGNMENT*2 + (lbaSize * ((gptTableLBAs*2) + 1 + 2)));    // extra padding for GPTs/MBR

    imageSize = espSize + dataSize + padding;
    imageSizeLBAs = bytesToLBAs(imageSize);

    alignLBA = ALIGNMENT / lbaSize;

    espLBA = alignLBA;
    espSizeLBAs = bytesToLBAs(espSize);

    dataSizeLBAs = bytesToLBAs(dataSize);
    // Get next aligned lba value:
    dataLBA = nextAlignedLBA(espLBA + espSizeLBAs);

    // Seed random generator
    srand(time(NULL));

    // Write protective MBR
    if(!writeMBR(image))
    {
        fprintf(stderr, "Error: could not write protective MBR for file %s\n", image_name);
        return EXIT_FAILURE;
    }

    // Write GPT headers & tables
    if(!writeGPTs(image))
    {
        fprintf(stderr, "Error: could not write GPT headers & tables for file %s\n", image_name);
        return EXIT_FAILURE;
    }

    // Write EFI System Partition w/FAT32 filesystem
    if(!writeESP(image))
    {
        fprintf(stderr, "Error: could not write ESP for file %s\n", image_name);
        return EXIT_FAILURE;
    }


    fclose(image);

    return EXIT_SUCCESS;
}