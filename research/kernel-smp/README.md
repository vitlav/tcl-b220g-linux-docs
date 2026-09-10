# TCL B220G: отдельный SMP-тест

Сохраняет успешный пункт one CPU и добавляет eight CPUs. В новом пункте изменён только maxcpus=1 на maxcpus=8. DTB, Image, initramfs, earlycon, keep_bootcon и cpuidle.off неизменны.

Критерий: достижение того же REACHED USERSPACE и CPU online 0-7. Это проверка запуска вторичных CPU и userspace, не нагрузочный тест и не проверка cpuidle/частот/теплового режима. UFS/USB/GPU/клавиатура не включены.

Прошлый успешный запуск подтвердил CPU0 и framebuffer0x9bc00000; пользователь подтвердил отсутствие наложения текста на реальном экране, поэтому параметры консолей не менялись.

Сборка GRUB: matching Debian2.12 tools/modules, arm64-efi, встроенный boot/grub/grub.cfg; модули normal configfile echo sleep reboot part_gpt part_msdos fat lsefimmap videoinfo efi_gop linux fdt gzio search search_fs_file.
