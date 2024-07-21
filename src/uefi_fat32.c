#include <uefi_fat32.h>

void getFATDirEntTimeDate(uint16_t *inTime, uint16_t *inDate)
{
    time_t curr_time = time(NULL);
    struct tm tm = *localtime(&curr_time);

    // FAT32 needs # of years since 1980, localtime returns tm_year as # years since 1900,
    //   subtract 80 years for correct year value. Also convert month of year from 0-11 to 1-12
    //   by adding 1
    *inDate = ((tm.tm_year - 80) << 9) | ((tm.tm_mon + 1) << 5) | tm.tm_mday;

    // Seconds is # 2-second count, 0-29
    if (tm.tm_sec == 60) tm.tm_sec = 59;
    *inTime = tm.tm_hour << 11 | tm.tm_min << 5 | (tm.tm_sec / 2);
}

bool writeESP(FILE *image)
{
    // Reserved sectors region ----------------
    // Fill out Volume Boot Record(VBR)
    const uint8_t reservedSectors = 32;      // FAT32 Count

    Vbr vbr =
    {

        .BS_jmpBoot = {   0xEB, 0x00, 0x90 },
        .BS_OEMName = {   "THISDISK"       },
        .BPB_BytsPerSec = lbaSize,
        .BPB_SecPerClus = 1,
        .BPB_RsvdSecCnt = reservedSectors,

        .BPB_NumFATs = 2,
        .BPB_RootEntCnt = 0,
        .BPB_TotSec16 = 0,
        .BPB_Media = 0xF8,              // "Fixed" non-removable media; Could also be 0xF0 for e.g. flash driver
        .BPB_FATSz16 = 0,
        .BPB_SecPerTrk = 0,
        .BPB_NumHeads = 0,
        .BPB_HiddSec = espLBA - 1,      // of sectors before this partition/volume
        .BPB_TotSec32 = espSizeLBAs,    // Size of this volume

        .BPB_FATSz32 = (alignLBA - reservedSectors) / 2,     // Align data region: on aligment value
        .BPB_ExtFlags = 0,              // Mirrored FATs
        .BPB_FSVer = 0,
        .BPB_RootClus = 2,              // Cluster 0 & 1 are reserved; root dir cluster starts at 2
        .BPB_FSInfo = 1,                // Sector 0 = this Vbr, FS info sector follow it
        .BPB_BkBootSec = 6,             // 
        .BPB_Reserved = { 0 },
        .BS_DrvNum = 0x80,              // 1st hard drive
        .BS_Reserved1 = 0,

        .BS_BootSig = 0x29,
        .BS_VolID = {0},
        .BS_VolLab = {"NO NAME    "},
        .BS_FilSysType = {"FAT32   "},

        .BootCode = {0},
        .BootSecT_Sig = 0xAA55              //0xAA55
    };

    // Fill out file system info sector
    FSInfo fsinfo = {

        .FSI_LeadSigOffset = 0x41615252,
        .FSI_Reserved1 = {0},
        .FSI_StrucSig = 0x61417272,
        .FSI_Free_Count = 0xFFFFFFFF,
        .FSI_Nxt_Free = 0xFFFFFFFF,
        .FSI_Reserved2 = {0},
        .FSI_TrailSig = 0xAA550000
    };

    // Write VBR
    fseek(image, espLBA * lbaSize, SEEK_SET);
    if(fwrite(&vbr, 1, sizeof vbr, image) != sizeof vbr)
    {
        fprintf(stderr, "Error: Could not write ESP Volume Boot Record to image\n");
        return false;
    }
    writeFullLBASize(image);

    // Write FSInfo
    if(fwrite(&fsinfo, 1, sizeof fsinfo, image) != sizeof fsinfo)
    {
        fprintf(stderr, "Error: Could not write ESP File System Info Sector to image\n");
        return false;
    }
    writeFullLBASize(image);

    // Go to backup boot sector location
    fseek(image, (espLBA + vbr.BPB_BkBootSec) * lbaSize, SEEK_SET);

    // Write VBR and FSInfo
    fseek(image, espLBA * lbaSize, SEEK_SET);
    if(fwrite(&vbr, 1, sizeof vbr, image) != sizeof vbr)
    {
        fprintf(stderr, "Error: Could not write ESP Volume Boot Record to image\n");
        return false;
    }
    writeFullLBASize(image);

    // Write FSInfo
    if(fwrite(&fsinfo, 1, sizeof fsinfo, image) != sizeof fsinfo)
    {
        fprintf(stderr, "Error: Could not write ESP File System Info Sector to image\n");
        return false;
    }
    writeFullLBASize(image);

    // FAT32 region ---------------------------
    // Write FATs(NOTE: Fats will me mirrored)
    const uint32_t fatLBA = espLBA + vbr.BPB_RsvdSecCnt;
    for(uint8_t i = 0; i < vbr.BPB_NumFATs; ++i)
    {
        fseek(image, (fatLBA + i *vbr.BPB_FATSz32) * lbaSize, SEEK_SET);

        uint32_t cluster = 0; 

        // Cluster 0; FAT identifier, lowest 8 bits are the media type/byte
        cluster = 0xFFFFFF00 | vbr.BPB_Media;
        fwrite(&cluster, sizeof cluster, 1, image);

        // Cluster 1; End of Chain (EOC) marker
        cluster = 0xFFFFFFFF;
        fwrite(&cluster, sizeof cluster, 1, image);

        // Cluster 2; Root dir '/' cluster start, if end of file/dir data then write EOC marker
        cluster = 0xFFFFFFFF;
        fwrite(&cluster, sizeof cluster, 1, image);

        // Cluster 3; '/EFI' dir cluster
        cluster = 0xFFFFFFFF;
        fwrite(&cluster, sizeof cluster, 1, image);

        // Cluster 4; '/EFI/BOOT' dir cluster
        cluster = 0xFFFFFFFF;
        fwrite(&cluster, sizeof cluster, 1, image);

        // Cluster 5+; Other files/directories...
        // e.g. if adding a file with a size = 5 sectors/clusters
        //cluster = 6;    // Point to next cluster containing file data
        //cluster = 7;    // Point to next cluster containing file data
        //cluster = 8;    // Point to next cluster containing file data
        //cluster = 9;    // Point to next cluster containing file data
        //cluster = 0xFFFFFFFF; // EOC marker, no more file data after this cluster
    }

    // Data region ----------------------------
    // Write File/Dir Data...
    const int32_t dataLBA = fatLBA + (vbr.BPB_NumFATs * vbr.BPB_FATSz32);
    fseek(image, dataLBA * lbaSize, SEEK_SET);

    // Root '/' Directory.
    // /EFI Directory
    FAT32_DirEntryShort dirEnt = {
        .DIR_Name = { "EFI        " },
        .DIR_Attr = ATTR_DIRECTORY,
        .DIR_NTRes = 0,
        .DIR_CrtTimeTenth = 0,
        .DIR_CrtTime = 0,
        .DIR_CrtDate = 0,
        .DIR_LstAccDate = 0,
        .DIR_FstClusHI = 0,
        .DIR_WrtTime = 0,
        .DIR_WrtDate = 0,
        .DIR_FstClusLO = 3,
        .DIR_FileSize = 0,  // Directories have 0 file size
    };

    uint16_t createTime = 0, createDate = 0;
    getFATDirEntTimeDate(&createTime, &createDate);

    dirEnt.DIR_CrtTime = createTime;
    dirEnt.DIR_CrtDate = createDate;
    dirEnt.DIR_WrtTime = createTime;
    dirEnt.DIR_WrtDate = createDate;

    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    // EFI Directory entries

    fseek(image, (dataLBA + 1) * lbaSize, SEEK_SET);

    memcpy(dirEnt.DIR_Name, ".          ", 11);     //  "." entry, this directory itself 
    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    memcpy(dirEnt.DIR_Name, "..         ", 11);     // ".." dir entry, parent dir (ROOT)
    dirEnt.DIR_FstClusLO = 0;                       // Root directory does not have a cluster value
    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    memcpy(dirEnt.DIR_Name, "BOOT       ", 11); // /EFI/BOOT directory
    dirEnt.DIR_FstClusLO = 4;                   // /EFI/BOOT cluster
    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    // /EFI/BOOT Directory entries
    fseek(image, (dataLBA + 2) * lbaSize, SEEK_SET);

    memcpy(dirEnt.DIR_Name, ".          ", 11);
    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    memcpy(dirEnt.DIR_Name, "..         ", 11); // ".." dir entry, parent dir (/EFI dir)
    dirEnt.DIR_FstClusLO = 3;                  // /EFI directory cluster
    fwrite(&dirEnt, sizeof dirEnt, 1, image);

    return true;
}
