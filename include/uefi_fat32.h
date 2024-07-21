#ifndef __UEFI_SPEC_2_10__FAT32_H__
#define __UEFI_SPEC_2_10__FAT32_H__

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <config.h>
#include <uefi_lba.h>

/**
 * @brief Структура для хранения данных FAT32 Volume Boot Record (VBR).
 *
 * @note Эта структура представляет собой загрузочный сектор и блок параметров BIOS (BPB) для файловой системы FAT32,
 * необходимый для инициализации и управления файловой системой.
 *
 * @param BS_jmpBoot Jump инструкция для загрузочного кода.
 * @param BS_OEMName Идентификатор OEM.
 * @param BPB_BytsPerSec Количество байт на сектор.
 * @param BPB_SecPerClus Количество секторов на кластер.
 * @param BPB_RsvdSecCnt Количество зарезервированных секторов.
 * 
 * @param BPB_NumFATs Количество файловых таблиц (FAT).
 * @param BPB_RootEntCnt Количество записей в корневом каталоге (только для FAT12/16).
 * @param BPB_TotSec16 Общее количество секторов (только для FAT12/16).
 * @param BPB_Media Описание носителя.
 * @param BPB_FATSz16 Количество секторов на FAT (только для FAT12/16).
 * @param BPB_SecPerTrk Количество секторов на трек.
 * @param BPB_NumHeads Количество головок.
 * @param BPB_HiddSec Количество скрытых секторов.
 *
 * @param BPB_TotSec32 Общее количество секторов (для FAT32).
 * @param BPB_FATSz32 Количество секторов на FAT (для FAT32).
 * @param BPB_ExtFlags Расширенные флаги.
 * @param BPB_FSVer Версия файловой системы.
 * @param BPB_RootClus Номер корневого кластера.
 * @param BPB_FSInfo Сектор с информацией о файловой системе.
 * @param BPB_BkBootSec Сектор резервного загрузочного кода.
 * @param BPB_Reserved Зарезервированные байты.
 * @param BS_DrvNum Номер диска.
 * @param BS_Reserved1 Зарезервированный байт.
 * @param BS_BootSig Подпись загрузочного сектора.
 * @param BS_VolID Идентификатор тома.
 * @param BS_VolLab Метка тома.
 * @param BS_FilSysType Тип файловой системы.
 * @param BootCode Загрузочный код.
 * @param BootSecSig Подпись загрузочного сектора (0xAA55).
 * 
 * @note (doc) MEFI_FAT32_FileSystem.pdf
 */
typedef struct  {

    // Boot Sector and BPB Structure
    uint8_t         BS_jmpBoot[3];  // {0xEB, 0x??, 0x90} or {0xE9, 0x??, 0x??} (spec)
    uint8_t         BS_OEMName[8];  // "MSWIN4.1"                               (spec)
    uint16_t        BPB_BytsPerSec; // 512, 1024, 2048 or 4096                  (spec)
    uint8_t         BPB_SecPerClus; //  1, 2, 4, 8, 16, 32, 64 or 128           (spec)
    uint16_t        BPB_RsvdSecCnt; // < 1 for FAT12, FAT16. 32 for FAT32       (spec)

    uint8_t         BPB_NumFATs;    // recommended 2                            (spec)
    uint16_t        BPB_RootEntCnt; // FAT12/16 - contains 32, FAT32 = 0, FAT16 = 512(recommended) (spec)
    uint16_t        BPB_TotSec16;
    uint8_t         BPB_Media;
    uint16_t        BPB_FATSz16;
    uint16_t        BPB_SecPerTrk;
    uint16_t        BPB_NumHeads;
    uint32_t        BPB_HiddSec;
    uint32_t        BPB_TotSec32;

    // FAT32 Structure Starting at Offset 36
    uint32_t        BPB_FATSz32;
    uint16_t        BPB_ExtFlags;
    uint16_t        BPB_FSVer;
    uint32_t        BPB_RootClus;
    uint16_t        BPB_FSInfo;
    uint16_t        BPB_BkBootSec;
    uint8_t         BPB_Reserved[12];
    uint8_t         BS_DrvNum;
    uint8_t         BS_Reserved1;


    // Fat12 and Fat16 Structure Starting at Offset 36
    uint8_t         BS_BootSig;
    uint8_t         BS_VolID[4];
    uint8_t         BS_VolLab[11];
    uint8_t         BS_FilSysType[8];

    // Not in fatspec.pdf tables
    uint8_t BootCode[510-90];
    uint16_t BootSecT_Sig;      //0xAA55

} __attribute__((packed)) Vbr;


/**
 * @brief FAT32 FSInfo Sector Structure and Backup Boot Sector
 *
 * @note Эта структура представляет собой сектор информации о файловой системе для FAT32,
 * включающий данные о свободных кластерах и следующем свободном кластере.
 *
 * @param FSI_LeadSigOffset Смещение ведущей подписи.
 * @param FSI_Reserved1 Зарезервированные байты.
 * @param FSI_StrucSig Структурная подпись.
 * @param FSI_Free_Count Количество свободных кластеров.
 * @param FSI_Nxt_Free Следующий свободный кластер.
 * @param FSI_Reserved2 Зарезервированные байты.
 * @param FSI_TrailSig Концевая подпись.
 * (doc) MEFI_FAT32_FileSystem.pdf
 */
typedef struct {

    uint32_t FSI_LeadSigOffset;
    uint8_t  FSI_Reserved1[480];
    uint32_t FSI_StrucSig;
    uint32_t FSI_Free_Count;
    uint32_t FSI_Nxt_Free;
    uint8_t  FSI_Reserved2[12];
    uint32_t FSI_TrailSig;

} __attribute__((packed)) FSInfo;

// FAT32 Directory Entry Attributes
typedef enum {
    ATTR_READ_ONLY = 0x01,
    ATTR_HIDDEN    = 0x02,
    ATTR_SYSTEM    = 0x04,
    ATTR_VOLUME_ID = 0x08,
    ATTR_DIRECTORY = 0x10,
    ATTR_ARCHIVE   = 0x20,
    ATTR_LONG_NAME = ATTR_READ_ONLY | ATTR_HIDDEN |
                     ATTR_SYSTEM    | ATTR_VOLUME_ID,
} FAT32_DirAttr;


/**
 * @brief FAT32 Byte Directory Entry Short Structure
 *
 * @note Эта структура представляет запись каталога в файловой системе FAT32.
 * Она содержит информацию о файлах и подкаталогах, включая их имена, атрибуты, временные метки,
 * и местоположения кластеров.
 *
 * @param DIR_Name Имя файла или подкаталога (11 байт, в формате 8.3).
 * @param DIR_Attr Атрибуты файла (например, только для чтения, скрытый, системный и т.д.).
 * @param DIR_NTRes Зарезервировано для использования системой NT.
 * @param DIR_CrtTimeTenth Десятые доли секунды создания файла.
 * 
 * @param DIR_CrtTime Время создания файла.
 * @param DIR_CrtDate Дата создания файла.
 * @param DIR_LstAccDate14 Дата последнего доступа к файлу.
 * @param DIR_FstClusHI202 Старшие 16 бит начального кластера файла.
 * @param DIR_WrtTime Время последнего изменения файла.
 * @param DIR_WrtDate Дата последнего изменения файла.
 * @param DIR_FstClusLO Младшие 16 бит начального кластера файла.
 * 
 * @param DIR_FileSize Размер файла в байтах.
 */
typedef struct
{

    uint8_t     DIR_Name[11];
    uint8_t     DIR_Attr;
    uint8_t     DIR_NTRes;
    uint8_t     DIR_CrtTimeTenth;

    uint16_t    DIR_CrtTime;
    uint16_t    DIR_CrtDate;
    uint16_t    DIR_LstAccDate;
    uint16_t    DIR_FstClusHI;
    uint16_t    DIR_WrtTime;
    uint16_t    DIR_WrtDate;
    uint16_t    DIR_FstClusLO;

    uint32_t    DIR_FileSize;

} __attribute__((packed)) FAT32_DirEntryShort;


// ==========
// Functions
// ==========

/**
 * @brief Получает текущее время и дату в формате FAT.
 *
 * @param inTime Указатель на переменную, в которую будет записано время.
 * @param inDate Указатель на переменную, в которую будет записана дата.
 *
 * @note Функция преобразует текущее локальное время в формат, совместимый с файловой системой FAT.
 * Год рассчитывается как количество лет с 1980 года, месяцы считаются с 1 до 12, а не с 0 до 11.
 * Время обрабатывается в формате FAT, где секунды считаются в 2-секундных интервалах.
 */
void getFATDirEntTimeDate(uint16_t *inTime, uint16_t *inDate);


/**
 * @brief Записывает таблицу разделов EFI System Partition (ESP) в файл образа.
 *
 * @param image Указатель на файл образа, в который будут записаны данные ESP.
 *
 * @return true, если запись успешно выполнена, иначе false.
 *
 * @note Функция заполняет раздел ESP данными, включая Volume Boot Record (VBR), File System Info (FSInfo), FAT и директории.
 * Используется для создания структуры файловой системы FAT32 на разделе ESP.
 * 
 * @details
 * 1. Заполняет и записывает Volume Boot Record (VBR) в зарезервированную область.
 * 2. Записывает File System Info (FSInfo) сектор.
 * 3. Создает и записывает резервную копию VBR и FSInfo.
 * 4. Заполняет FAT таблицы, зеркально записывая их.
 * 5. Записывает корневую директорию и директорию EFI.
 */
bool writeESP(FILE *image);

#endif