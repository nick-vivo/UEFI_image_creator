#!/bin/sh

# Sendin' Out a TEST O S

# Version 0.0.1
qemu-system-x86_64 -bios bios64.bin -machine q35 -net none -drive file=test.img,format=raw

# qemu-system-x86_64 \: Запускает QEMU для эмуляции 64-битной архитектуры x86.
# -bios ./bios/bios64.bin \: Указывает файл BIOS для использования (в данном случае bios64.bin из директории ./bios).
# -m 256M \: Задает количество оперативной памяти для виртуальной машины (256 МБ).
# -vga std \: Устанавливает стандартный VGA-графический адаптер.
# -name TESTOS \: Задает имя виртуальной машины (TESTOS).
# -machine q35 \: Определяет тип машины как Q35, что указывает на использование современной чипсеты Intel.
# -usb \: Включает поддержку USB.
# -device usb-mouse \: Добавляет USB-мышь к виртуальной машине.
# -rtc base=localtime \: Устанавливает время реального времени (RTC) в виртуальной машине на локальное время хоста.
# -net none: Отключает сетевое подключение для виртуальной машины.



# For testing other drive physical/logical sizes. Although this did not work for me for lba size > 512
#qemu-system-x86_64 \
#-bios bios64.bin \
#-vga std \
#-boot d -device virtio-scsi-pci,id=scsi1,bus=pci.0 \
#-drive file=test.img,if=none,id=drive-virtio-disk1 \
#-device scsi-hd,bus=scsi1.0,drive=drive-virtio-disk1,id=virtio-scsi-pci,physical_block_size=1024,logical_block_size=1024 \
#-net none
