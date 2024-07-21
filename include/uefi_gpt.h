#ifndef __UEFI_SPEC_2_10__GPT_H__
#define __UEFI_SPEC_2_10__GPT_H__

#include <uchar.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// global and consts.
#include <config.h>
#include <uefi_lba.h>

// -------------------------------------
// Global Typedefs
// -------------------------------------

/**
 * @brief Структура для хранения данных GUID.
 *
 * @note Эта структура представляет Global Unique Identifier (GUID) версии 4, варианта 2,
 * используемый для уникальной идентификации разделов и других объектов.
 *
 * @param TimeLow Младшие 32 бита временной метки.
 * @param TimeMid Средние 16 бит временной метки.
 * @param TimeHighAndVersion Старшие 16 бит временной метки, включая номер версии.
 * @param ClockSeqHighAndReversed Высокий байт последовательного номера, включая номер варианта.
 * @param ClockSeqLow Низкий байт последовательного номера.
 * @param Node Массив из 6 байтов, представляющий узел (обычно MAC-адрес).
 * @note (doc) UEFI_Spec_2_10.pdf
 */
typedef struct {

    uint32_t TimeLow;
    uint16_t TimeMid;
    uint16_t TimeHighAndVersion;        // Последние 4 бита - номер версии.
    uint8_t ClockSeqHighAndReversed;    // Последние 4 бита - нормер варианта.
    uint8_t ClockSeqLow;
    uint8_t Node[6];

} __attribute__((packed)) Guid;


/**
 * @brief Структура для хранения данных GPT Header.
 * 
 * @note Эта структура представляет GUID Partition Table (GPT) Header и содержит информацию о 
 * разделе диска, такую как размер таблицы разделов, размер записей разделов, CRC и GUID диска.
 * 
 * @param Signature Уникальная сигнатура заголовка GPT (обычно "EFI PART").
 * @param Revision Ревизия заголовка GPT.
 * @param HeaderSize Размер заголовка GPT в байтах.
 * @param HeaderCRC32 Контрольная сумма CRC-32 заголовка GPT.
 * @param Reversed Зарезервировано.
 * @param MyLBA Логический блок адреса (LBA), где расположен заголовок GPT.
 * @param AlternateLBA Логический блок адреса, где расположен запасной заголовок GPT.
 * @param FirstUsableLBA Логический блок адреса, где начинаются данные раздела.
 * @param LastUsableLBA Логический блок адреса, где заканчиваются данные раздела.
 * @param DiskGuid Уникальный идентификатор диска в формате GUID.
 * @param PartitionEntryLBA Логический блок адреса, где начинается таблица разделов.
 * @param NumberOfPartitionEntries Число записей в таблице разделов.
 * @param SizeOfPartition Размер каждой записи в таблице разделов в байтах.
 * @param PartitionEntryArrayCRC32 Контрольная сумма CRC-32 таблицы разделов.
 * @param ReversedSecond Зарезервировано.
 * @note (doc) UEFI_Spec_2_10.pdf
 */
typedef struct {

    uint8_t     Signature[8];
    uint32_t    Revision;
    uint32_t    HeaderSize;
    uint32_t    HeaderCRC32;
    uint32_t    Reversed;
    uint64_t    MyLBA;
    uint64_t    AlternateLBA;
    uint64_t    FirstUsableLBA;
    uint64_t    LastUsableLBA;
    Guid        DiskGuid;
    uint64_t    PartitionEntryLBA;
    uint32_t    NumberOfPartitionEntries;
    uint32_t    SizeOfPartition;
    uint32_t    PartitionEntryArrayCRC32;
    uint8_t     ReversedSecond[512-92];

} __attribute__((packed)) GptHeader;


/**
 * @brief Структура для хранения данных записи раздела GPT.
 * 
 * @note Эта структура представляет запись раздела GPT и содержит информацию о разделе,
 * такую как тип раздела, уникальный идентификатор раздела, начальный и конечный логические
 * блоки адреса (LBA), атрибуты раздела и имя раздела.
 * 
 * @param PartitionTypeGUID Тип раздела в формате GUID.
 * @param UniquePartitionGUID Уникальный идентификатор раздела в формате GUID.
 * @param StartingLBA Начальный логический блок адреса (LBA) раздела.
 * @param EndingLBA Конечный логический блок адреса (LBA) раздела.
 * @param Attributes Атрибуты раздела.
 * @param PartitionName Имя раздела в формате Unicode.
 * @note (doc) UEFI_Spec_2_10.pdf
*/
typedef struct {

    Guid    PartitionTypeGUID;
    Guid    UniquePartitionGUID;
    uint64_t StartingLBA;
    uint64_t EndingLBA;
    uint64_t Attributes;
    char16_t PartitionName[36];

} __attribute__((packed)) GptPartitionEntry;


// ==========
// Functions
// ==========

/**
 * @brief Создает новый GUID версии 4, варианта 2.
 *
 * @return Новый GUID.
 *
 * @note Функция генерирует новый GUID версии 4, варианта 2, который можно использовать
 * для уникальной идентификации объектов. GUID генерируется с использованием случайных данных.
 */
Guid new_guid();

extern uint32_t crcTable[256];
/**
 * @brief Создает таблицу CRC32.
 *
 * @note Функция инициализирует глобальную таблицу crcTable[256], которая используется для
 * вычисления контрольных сумм CRC32. Таблица содержит 256 предвычисленных значений,
 * которые ускоряют процесс вычисления CRC32.
 */
void createCRC32Table(void);

/**
 * @brief Вычисляет значение CRC32 для диапазона данных.
 *
 * @param buf Указатель на буфер с данными.
 * @param len Длина буфера в байтах.
 *
 * @return Значение CRC32 для предоставленных данных.
 *
 * @note Функция принимает указатель на буфер и его длину, и возвращает значение CRC32, 
 * вычисленное для этих данных. Использует предвычисленную таблицу CRC32 для ускорения вычислений.
 */
uint32_t calculateCRC32(void *buf, int32_t len);

/**
 * @brief EFI GUID.
 *
 * @note Этот GUID представляет EFI System Partition. 
 * Он используется для идентификации раздела EFI в GPT.
 * @note Wiki EFI GUID - C12A7328-F81F-11D2-BA4B-00A0C93EC93B.
 */
extern const Guid EFI_GUID;

/**
 * @brief Microsoft Basic Data GUID.
 *
 * @note Этот GUID представляет базовый раздел данных Microsoft. 
 * Он используется для идентификации стандартного раздела данных в GPT.
 * @note (Microsoft) Basic Data Guid EBD0A0A2-B9E5-4433-87C0-68B6B72699C7.
 */
extern const Guid BASIC_DATA_GUID;

/**
 * @brief Записывает заголовки и таблицы GPT в файл образа.
 *
 * @param image Указатель на файл образа, в который будут записаны заголовки и таблицы GPT.
 *
 * @return true, если запись прошла успешно, иначе false.
 *
 * @note Функция создает и записывает заголовки и таблицы GPT в указанный файл образа.
 * Эта функция должна быть вызвана после того, как файл образа был открыт для записи.
 */
bool writeGPTs(FILE *image);

#endif