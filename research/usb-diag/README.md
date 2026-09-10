# Диагностическая флешка TCL

GRUB 2.12-9+deb13u2, ARM64 EFI, конфигурация встроена в BOOTAA64.EFI. Linux ядра нет. Вывод только на экран, автоматического сохранения логов нет.

Исходные пакеты Debian из https://deb.debian.org/debian/pool/main/g/grub2/: grub-common_2.12-9+deb13u2_amd64.deb и grub-efi-arm64-bin_2.12-9+deb13u2_arm64.deb; распакованы без установки. Для инструмента использована распакованная libdevmapper1.02.1_1.02.205-2_amd64.deb. Команда: grub-mkstandalone -O arm64-efi -d PATH/usr/lib/grub/arm64-efi --locales= --fonts= --modules='normal configfile echo sleep reboot part_gpt part_msdos fat lsefimmap videoinfo efi_gop' -o BOOTAA64.EFI boot/grub/grub.cfg=grub.cfg.

prepare-usb.ps1 стирает Kingston с конкретным serial и размером, создаёт GPT/FAT32 и EFI/BOOT/BOOTAA64.EFI. Предназначен только для предоставленной пользователем флешки. Локальная версия читает EFI рядом со скриптом; при фактической передаче SSH использован встроенный base64 тех же байт.

PowerShell -Command - не исполнил многострочный try/catch из stdin (пустой результат при коде 0). Попытка прочитать stdin целиком через [Console]::In.ReadToEnd() тоже зависла: процесс 14216, подготовительный EFI в TEMP отсутствовал, разметка USB оставалась прежней. После проверки CommandLine процесс остановлен адресно, SSH завершился с кодом 255. Решение: передать BOOTAA64.EFI и prepare-usb.ps1 через scp/SFTP в домашний каталог и выполнить powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -File prepare-usb.ps1. Постоянная ExecutionPolicy не меняется.

После Clear-Disk Windows сохранила GPT; безусловный Initialize-Disk завершился ошибкой «disk has already been initialized», до создания нового раздела. Исправлено: Initialize-Disk только при RAW, затем обязательная проверка GPT. Повторный запуск допустим только для той же уже разрешённой и идентифицированной флешки.
