#ifndef __CONFIG__UEFI_GPT_IMAGE_CREATOR__
#define __CONFIG__UEFI_GPT_IMAGE_CREATOR__

#include <stdint.h>

// -----------------------------------------
// Глобальные константы, перечисления и тд.
// -----------------------------------------

enum {
    GPT_TABLE_ENTRY_SIZE = 128,         // Размер одной записи таблицы GPT в байтах.
    NUMBER_OF_GPT_TABLE_ENTRIES = 128,  // Количество записей в таблице GPT.
    GPT_TABLE_SIZE = 16384,             // 1024 * 16 Общий размер таблицы GPT в байтах (16 KiB).
    ALIGNMENT = 1048576,                // 1024 * 1024 * 1 Размер одного физического кластера в байтах (1 MiB или больше).
};


// -------------------------------------
// Глобальные переменные
// -------------------------------------

extern char *image_name;          // Название выходного файла образа диска.

extern uint64_t lbaSize;          // Размер одного логического блока данных. (512, 1024, 2048, 4096)
extern uint64_t espSize;          // Размер раздела EFI System Partition (ESP) в байтах. (33 MiB)
extern uint64_t dataSize;         // Размер раздела данных в байтах. (1 MiB)
extern uint64_t imageSize;        // Общий размер образа диска в байтах. Первоначально инициализируется как 0 и будет рассчитан позже.


// Размер разделов в LBA.
extern uint64_t espSizeLBAs;      // Размер раздела ESP в логических блоках (LBA).
extern uint64_t dataSizeLBAs;     // Размер раздела данных в логических блоках (LBA).
extern uint64_t imageSizeLBAs;    // Общий размер образа диска в логических блоках (LBA).
extern uint64_t gptTableLBAs;     // Размер таблицы GPT в логических блоках (LBA).

extern uint64_t alignLBA;         // Выравнивание в логических блоках (LBA).
extern uint64_t espLBA;           // Начало раздела ESP в логических блоках (LBA).
extern uint64_t dataLBA;          // Начало раздела данных в логических блоках (LBA).


#endif