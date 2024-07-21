#include <uefi_mbr.h>

bool writeMBR(FILE *image)
{
    uint64_t mbrSizeLBAs = imageSizeLBAs;
    if(mbrSizeLBAs > 0xFFFFFFFF) mbrSizeLBAs = 0x100000000;

    Mbr mbr = {

        .BootCode = {0},
        .UniqueMBRDiskSignature = 0,
        .Unknown = 0,
        .PartitionRecord[0] = {

            .BootIndicator = 0,
            .StartingCHS = {0x00, 0x02, 0x00},
            .OSType = 0xEE,                     //Protective GPT
            .EndingCHS = {0xFF, 0xFF, 0xFF},
            .StartingLBA = 0x00000001,
            .SizeInLBA = mbrSizeLBAs - 1,
        },
        .Signature = 0xAA55,

    };

    // Write to file
    if(fwrite(&mbr, 1, sizeof mbr, image) != sizeof mbr) return false;

    writeFullLBASize(image);

    return true;
}
