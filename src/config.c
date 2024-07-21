#include <config.h>

// Определение и инициализация глобальных переменных
char *image_name = "test.img";          // Название выходного файла образа диска.
uint64_t lbaSize = 512;                 // Размер одного логического блока данных.
uint64_t espSize = 1024 * 1024 * 33;    // Размер раздела EFI System Partition (ESP) в байтах. (33 MiB)
uint64_t dataSize = 1024 * 1024 * 1;    // Размер раздела данных в байтах. (1 MiB)
uint64_t imageSize = 0;                 // Общий размер образа диска в байтах. Первоначально инициализируется как 0 и будет рассчитан позже.

// Размер раздела в LBA.
uint64_t espSizeLBAs = 0, dataSizeLBAs = 0, imageSizeLBAs = 0, gptTableLBAs = 0;
uint64_t alignLBA = 0, espLBA = 0, dataLBA = 0;
