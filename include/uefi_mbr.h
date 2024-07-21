#ifndef __UEFI_SPEC_2_10__MBR_H__
#define __UEFI_SPEC_2_10__MBR_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// global and consts.
#include <config.h>
#include <uefi_lba.h>

// -------------------------------------
// Global Typedefs
// -------------------------------------

/**
 * @brief Структура для хранения данных раздела MBR.
 *
 * @note Представляет запись раздела в MBR. Она используется для
 * описания расположения и размера разделов на диске.
 *
 * @param BootIndicator Определяет, является ли раздел загрузочным. Значение true указывает, что раздел загрузочный.
 * @param StartingCHS Начальная CHS-адресация. Адресация Cylinders, Heads и Sectors для начала раздела.
 * @param OSType Тип операционной системы. Определяет тип файловой системы или операционной системы, которая будет использовать этот раздел.
 * @param EndingCHS Конечная CHS-адресация. Адресация Cylinders, Heads и Sectors для конца раздела.
 * @param StartingLBA Начальный LBA. Начальное значение LBA для раздела.
 * @param SizeInLBA Размер в LBA. Размер раздела в логических блоках.
 */
typedef struct {

    bool        BootIndicator;
    uint8_t     StartingCHS[3];
    uint8_t     OSType;
    uint8_t     EndingCHS[3];
    uint32_t    StartingLBA;
    uint32_t    SizeInLBA;

} __attribute__((packed)) Mbr_Partition;

/**
 * @brief Структура для хранения данных MBR.
 *
 * @note Эта структура представляет Master Boot Record (MBR) и содержит 
 * информацию о разделе диска и коде загрузчика.
 *
 * @param BootCode Загрузочный код. Начальный код, который выполняется при загрузке системы. 
 * Обычно содержит небольшой загрузчик или инструкции по переходу к основному загрузчику.
 * @param UniqueMBRDiskSignature Уникальная сигнатура диска MBR. Используется для уникальной идентификации диска.
 * @param Unknown Неизвестное поле. Резервное или неиспользуемое поле.
 * @param PartitionRecord Записи разделов MBR. Содержит информацию о четырех разделах диска.
 * @param Signature Подпись MBR. Завершающая подпись MBR (обычно 0xAA55), указывающая на правильность MBR.
 */
typedef struct {

    uint8_t         BootCode[440];
    uint32_t        UniqueMBRDiskSignature;
    uint16_t        Unknown;
    Mbr_Partition   PartitionRecord[4];
    uint16_t        Signature;

} __attribute__((packed)) Mbr;

// ==========
// Functions
// ==========

/**
 * @brief Записывает Master Boot Record (MBR) в файл образа.
 * 
 * @param image Указатель на файл образа, в который будет записан MBR.
 * 
 * @return true, если запись прошла успешно, иначе false.
 * 
 * @note Функция создает структуру MBR с защитным GPT-разделом и записывает её в файл образа.
 * Если размер образа в LBA превышает 0xFFFFFFFF, то он ограничивается этим значением.
 * Затем функция заполняет оставшееся пространство до конца LBA нулевыми байтами, вызывая `writeFullLBASize`.
 * 
 * @note Перед вызовом этой функции необходимо открыть файл образа для записи.
 * 
 */
bool writeMBR(FILE *image);

#endif