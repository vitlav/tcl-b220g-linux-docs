# Живые ACPI-таблицы против дампа Windows

Boot 4e6555b5-375b-4855-8672-59a720177a4c; все прочитанные checksum=0. MSDMpayload исключён.

| Таблица | Адрес | Байт | Сравнение |
|---|---|---:|---|
| XSDT | 0xffffc000 | 140 | нет сохранённого .dat для сравнения |
| FACP | 0xfffc2000 | 276 | совпадает: FACP-1.dat |
| DSDT | 0xfffc3000 | 189125 | совпадает: DSDT.dat |
| BGRT | 0xffffb000 | 56 | отличаются только status и checksum; разбор ниже |
| CSRT | 0xffff3000 | 29717 | совпадает: CSRT-3.dat |
| DBG2 | 0xffff2000 | 348 | совпадает: DBG2-8.dat |
| GTDT | 0xfffc1000 | 156 | совпадает: GTDT-5.dat |
| IORT | 0xfffc0000 | 3616 | совпадает: IORT-9.dat |
| APIC | 0xfffbf000 | 749 | совпадает: APIC-2.dat |
| MCFG | 0xfffbe000 | 108 | совпадает: MCFG-0.dat |
| PPTT | 0xfffbd000 | 414 | совпадает: PPTT-4.dat |
| SPCR | 0xfffbc000 | 80 | совпадает: SPCR-10.dat |
| TPM2 | 0xfffbb000 | 84 | совпадает: TPM2-11.dat |
| MSDM | 0xfffba000 | — | payload не читался |
| FPDT | 0xfffb9000 | 68 | совпадает: FPDT-6.dat |

BGRT: отличаются только byte38 status0→1 и checksum byte9 91→90. ImageAddress и координаты совпадают.
