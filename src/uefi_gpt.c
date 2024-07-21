#include <uefi_gpt.h>

Guid new_guid(void) {
    uint8_t rand_arr[16] = { 0 };

    for (uint8_t i = 0; i < sizeof rand_arr; i++)
        rand_arr[i] = rand() % (UINT8_MAX + 1);

    // Fill out GUID
    Guid result = {
        .TimeLow         = *(uint32_t *)&rand_arr[0],
        .TimeMid        = *(uint16_t *)&rand_arr[4],
        .TimeHighAndVersion = *(uint16_t *)&rand_arr[6],
        .ClockSeqHighAndReversed = rand_arr[8],
        .ClockSeqLow = rand_arr[9],
        .Node = { rand_arr[10], rand_arr[11], rand_arr[12], rand_arr[13],
                  rand_arr[14], rand_arr[15] },
    };

    // Fill out version bits - version 4
    result.TimeHighAndVersion &= ~(1 << 15); // 0b0111 1111
    result.TimeHighAndVersion |= (1 << 14);  // 0b0100 0000
    result.TimeHighAndVersion &= ~(1 << 13); // 0b1101 1111
    result.TimeHighAndVersion &= ~(1 << 12); // 0b1110 1111

    // Fill out variant bits
    result.ClockSeqHighAndReversed |= (1 << 7);    // 0b1000 0000
    result.ClockSeqHighAndReversed |= (1 << 6);    // 0b0100 0000
    result.ClockSeqHighAndReversed &= ~(1 << 5);   // 0b1101 1111

    return result;
}

uint32_t crcTable[256];
void createCRC32Table(void) {
    uint32_t c = 0;

    for (int32_t n = 0; n < 256; n++) {
        c = (uint32_t)n;
        for (uint8_t k = 0; k < 8; k++) {
            if (c & 1) 
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crcTable[n] = c;
    }
}

uint32_t calculateCRC32(void *buf, int32_t len) {
    static bool made_crc_table = false;

    uint8_t *bufp = buf;
    uint32_t c = 0xFFFFFFFFL;

    if (!made_crc_table) {
        createCRC32Table();
        made_crc_table = true;
    }

    for (int32_t n = 0; n < len; n++) 
        c = crcTable[(c ^ bufp[n]) & 0xFF] ^ (c >> 8);

    // Invert bits for return value
    return c ^ 0xFFFFFFFFL;
}

const Guid EFI_GUID = { 0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, 
                        { 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B } };


const Guid BASIC_DATA_GUID = { 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0,
                                { 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 } };

bool writeGPTs(FILE *image) {
    // Fill out primary GPT header
    GptHeader primary_gpt = {
        .Signature = { "EFI PART" },
        .Revision = 0x00010000,   // Version 1.0
        .HeaderSize = 92,
        .HeaderCRC32 = 0,      // Will calculate later
        .Reversed = 0,
        .MyLBA = 1,            // LBA 1 is right after MBR
        .AlternateLBA = imageSizeLBAs - 1,
        .FirstUsableLBA = 1 + 1 + gptTableLBAs, // MBR + GPT header + primary gpt table
        .LastUsableLBA = imageSizeLBAs - 1 - gptTableLBAs - 1, // 2nd GPT header + table
        .DiskGuid = new_guid(),
        .PartitionEntryLBA = 2,   // After MBR + GPT header
        .NumberOfPartitionEntries = 128,   
        .SizeOfPartition = 128,
        .PartitionEntryArrayCRC32 = 0, // Will calculate later
        .ReversedSecond = { 0 },
    };

    // Fill out primary table partition entries
    GptPartitionEntry gpt_table[NUMBER_OF_GPT_TABLE_ENTRIES] = {
        // EFI System Paritition
        {
            .PartitionTypeGUID = EFI_GUID,
            .UniquePartitionGUID = new_guid(),
            .StartingLBA = espLBA,
            .EndingLBA = espLBA + espSizeLBAs,
            .Attributes = 0,
            .PartitionName = u"EFI SYSTEM",
        },

        // Basic Data Paritition
        {
            .PartitionTypeGUID = BASIC_DATA_GUID,
            .UniquePartitionGUID = new_guid(),
            .StartingLBA = dataLBA,
            .EndingLBA = dataLBA + dataSizeLBAs,
            .Attributes = 0,
            .PartitionName = u"BASIC DATA",
        },
    };

    // Fill out primary header CRC values
    primary_gpt.PartitionEntryArrayCRC32 = calculateCRC32(gpt_table, sizeof gpt_table);
    primary_gpt.HeaderCRC32 = calculateCRC32(&primary_gpt, primary_gpt.HeaderSize);

    // Write primary gpt header to file
    if (fwrite(&primary_gpt, 1, sizeof primary_gpt, image) != sizeof primary_gpt)
        return false;
    writeFullLBASize(image);

    // Write primary gpt table to file
    if (fwrite(&gpt_table, 1, sizeof gpt_table, image) != sizeof gpt_table)
        return false;

    // Fill out secondary GPT header
    GptHeader secondary_gpt = primary_gpt;

    secondary_gpt.HeaderCRC32 = 0;
    secondary_gpt.PartitionEntryArrayCRC32 = 0;
    secondary_gpt.MyLBA = primary_gpt.AlternateLBA;
    secondary_gpt.AlternateLBA = primary_gpt.MyLBA;
    secondary_gpt.PartitionEntryLBA = imageSizeLBAs - 1 - gptTableLBAs;

    // Fill out secondary header CRC values
    secondary_gpt.PartitionEntryArrayCRC32 = calculateCRC32(gpt_table, sizeof gpt_table);
    secondary_gpt.HeaderCRC32 = calculateCRC32(&secondary_gpt, secondary_gpt.HeaderCRC32);

    // Go to position of secondary table
    fseek(image, secondary_gpt.PartitionEntryLBA * lbaSize, SEEK_SET);

    // Write secondary gpt table to file
    if (fwrite(&gpt_table, 1, sizeof gpt_table, image) != sizeof gpt_table)
        return false;

    // Write secondary gpt header to file
    if (fwrite(&secondary_gpt, 1, sizeof secondary_gpt, image) != sizeof secondary_gpt)
        return false;
    writeFullLBASize(image);

    return true;
}