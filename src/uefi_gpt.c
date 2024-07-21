#include <uefi_gpt.h>

Guid new_guid()
{
    uint8_t randArray[16] = {0};

    for(uint8_t i = 0; i < sizeof randArray; ++i) randArray[i] = rand() & 0xFF;

    // Fill out Guid
    Guid result = {
        .TimeLow                 = *(uint32_t *) &randArray[0],
        .TimeMid                 = *(uint16_t *) &randArray[4],
        .TimeHighAndVersion      = *(uint16_t *) &randArray[6],
        .ClockSeqHighAndReversed = *(uint8_t *) &randArray[8],
        .ClockSeqLow             = *(uint8_t *) &randArray[9],
        .Node                    = {randArray[10], randArray[11], randArray[12], 
                                    randArray[13], randArray[14], randArray[15]}
    };

    // Fill out version
    result.TimeHighAndVersion &= ~(1 << 15);    //0b 0111 1111...
    result.TimeHighAndVersion |= (1 << 14);     //0b 0100 0000...
    result.TimeHighAndVersion &= ~(1 << 13);    //0b 1101 1111...
    result.TimeHighAndVersion &= ~(1 << 12);    //0b 1110 1111...

    // Fill out variant
    result.ClockSeqHighAndReversed |= 1 << 7;   //0b 1000 0000
    result.ClockSeqHighAndReversed |= 1 << 6;   //0b 0100 0000
    result.ClockSeqHighAndReversed &= ~(1 << 5);   //0b 0010 0000

    return result;
}

uint32_t crcTable[256];
void createCRC32Table(void)
{
    uint32_t c;

    for(int32_t n = 0; n < 256; n++)
    {
        c = (uint32_t) n;
        for(uint8_t k = 0; k < 8; k++) {

            if(c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;

            crcTable[n] = c;
        }

    } 
}

uint32_t calculateCRC32(void *buf, int32_t len)
{
    static bool madeCRCTable = false;

    uint8_t *bufp = buf;
    uint32_t c = 0xFFFFFFFFL;

    if(!madeCRCTable){
        createCRC32Table();
        madeCRCTable = true;
    }

    for(int32_t n = 0; n < len; ++n)
        c = crcTable[(c ^ bufp[n]) & 0xFF] ^ (c >> 8);

    return c ^ 0xFFFFFFFFL;
}

const Guid EFI_GUID = {
    0xC12A7328,
    0xF81F,
    0x11D2,
    0xBA,
    0x4B,
    {
        0x00,
        0xA0,
        0xC9,
        0x3E,
        0xC9,
        0x3B
    }
};

const Guid BASIC_DATA_GUID = {
    0xEBD0A0A2,
    0xB9E5,
    0x4433,
    0x87,
    0xC0,
    {
        0x68,
        0xB6,
        0xB7,
        0x26,
        0x99,
        0xC7,
    },
};

bool writeGPTs(FILE *image)
{
    // Fill out primary GPT header
    GptHeader primaryHeader = {
        .Signature = { "EFI PART" },
        .Revision = 0x00010000, // Version 1.0
        .HeaderSize = 92,
        .HeaderCRC32 = 0,   // Will calculate later
        .Reversed = 0,
        .MyLBA = 1,     // LBA 1 is right later MBR
        .AlternateLBA = imageSizeLBAs - 1,
        .FirstUsableLBA = 1 + 1 + gptTableLBAs,   // MBR + GPT header + PrimaryGptTable
        .LastUsableLBA = imageSizeLBAs - 1 - gptTableLBAs - 1,    //2nd GptHeader + table
        .DiskGuid = new_guid(),
        .PartitionEntryLBA = 2,     // After MBR + GPT header
        .NumberOfPartitionEntries = 128,
        .SizeOfPartition = 128,     // 128, 256, 512, etc.. Partition Table (GPT) Disk Layout ,Specification Release 2.10
        .PartitionEntryArrayCRC32 = 0,  // Will calculate later
        .ReversedSecond = {0},
    };

    // Fill out primary table partition entries
    GptPartitionEntry gptTable[NUMBER_OF_GPT_TABLE_ENTRIES] = {

        // EFI SYSTEM Partition
        {
            .PartitionTypeGUID = EFI_GUID,
            .UniquePartitionGUID = new_guid(),
            .StartingLBA = espLBA,
            .EndingLBA = espLBA + espSizeLBAs,
            .Attributes = 0,
            .PartitionName = u"EFI SYSTEM",
        },
        // Basic Data Partition
        {
            .PartitionTypeGUID = BASIC_DATA_GUID,
            .UniquePartitionGUID = new_guid(),
            .StartingLBA = dataLBA,
            .EndingLBA = dataLBA +  dataSizeLBAs,
            .Attributes = 0,
            .PartitionName = u"BASIC DATA",
        },
        // Остальные будут 0
    };
    // Fill out primary header CRC values
    primaryHeader.PartitionEntryArrayCRC32 = calculateCRC32(gptTable, sizeof gptTable);
    primaryHeader.HeaderCRC32 = calculateCRC32(&primaryHeader, primaryHeader.HeaderSize);

    // Write primary gpt header to file
    if(fwrite(&primaryHeader, 1, sizeof primaryHeader, image) != sizeof primaryHeader)
        return false;
    writeFullLBASize(image);

    // Write primary gpt table to file
    if(fwrite(&gptTable, 1, sizeof gptTable, image) != sizeof gptTable)
        return false;

    // Fill out secondary GPT header
    GptHeader secondaryGPT = primaryHeader;

    secondaryGPT.HeaderCRC32 = 0;
    secondaryGPT.PartitionEntryArrayCRC32 = 0;

    secondaryGPT.MyLBA = primaryHeader.AlternateLBA;
    secondaryGPT.AlternateLBA = primaryHeader.MyLBA;
    secondaryGPT.PartitionEntryLBA = imageSizeLBAs - 1 - gptTableLBAs;

    // Fill out secondary header CRC header
    secondaryGPT.PartitionEntryArrayCRC32 = calculateCRC32(gptTable, sizeof gptTable);
    secondaryGPT.HeaderCRC32 = calculateCRC32(&secondaryGPT, secondaryGPT.HeaderSize);

    // Go to position of secondary table
    fseek(image, secondaryGPT.PartitionEntryLBA * lbaSize, SEEK_SET);

    // Write secondary gpt table to file
    if(fwrite(&gptTable, 1, sizeof gptTable, image) != sizeof gptTable)
        return false;

    // Write secondary gpt header to file
    if(fwrite(&secondaryGPT, 1, sizeof secondaryGPT, image) != sizeof secondaryGPT)
        return false;
    writeFullLBASize(image);

    return true;
}
