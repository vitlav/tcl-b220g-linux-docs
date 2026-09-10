# OEM eDP power resources: первичное восстановление таблиц

2026-09-08. Только офлайн-чтение XBL, без выполнения прошивки и изменения напряжений TCL.

Из проверенного XBL FV найдены ещё два PE ARM64 модуля, содержащих /pmic/client/disp_edp и /pmic/client/disp_bridge_refclk. Их имена/роль загрузки пока не установлены, идентификаторы и SHA256 в oem-power-modules.json. Полные firmware-файлы остаются приватно /tmp/tcl-graphics-private/.

Модуль FV offset0x29a524, SHA e04746eb16d13322321e6e816c8be8ba30065243be7f915187ed2622d8f6aba5:
- record0x17b60 содержит указатель на строку /pmic/client/disp_edp;
- record+0x10 указывает на0x17860, массив трёх указателей;
- record+0x30 указывает на0x27888, record+0x38 равно18;
- по0x27888 находятся18записей по24байта с указателями на mode/mV/en для ldoc3,ldoa4,ldoc8, каждый дважды (типы0x40/0x800 не расшифрованы окончательно);
- таблицы по0x177f0,0x17814,0x17838 содержат тройки [4,1200,0],[4,1200,1],[7,1200,1]; соответственно880 и1800 для следующих двух таблиц.

| Ресурс в таблице eDP | Предполагаемое значение mV | Ограничение |
|---|---:|---|
| /pm/ldoc3 | 1200 | Порядок таблиц согласован с dependencies; нужна сверка структуры по исходникам/исполняющему коду |
| /pm/ldoa4 | 880 | Не копировать800mV из названия родственного DTS |
| /pm/ldoc8 | 1800 | Функция линии на TCL пока не установлена |

Связь ресурса с тремя именами подтверждена указателями. Интерпретация массивов как [mode,mV,enable] основана на именах и регулярной структуре; enum4/7 и выбор таблицы конкретной платформой пока не подтверждены. Это не электрическая схема и не доказательство текущего физического напряжения.

В DisplayDxe resource pointer table0x44ef0/0x44f08 использует /pmic/client/disp_edp для DisplayPrim/DisplaySec; имя ресурса /pmic/client/disp_bridge_refclk также присутствует. Его реальный provider и последовательность запросов ещё нужно разобрать.

Live regulator_summary рабочего Linux сообщает ldo3=1200mV и ldo4=880mV, потребители USB/UFS. Эти данные отражают состояние Linux regulator framework, не измерение напряжения и не подтверждение подключения DSI. Ничего не менять по этому промежуточному отчёту.

Дерево qc7 теперь подготовлено в /tmp/tcl-qc7-build/linux-6.18.34: olddefconfig/modules_prepare успешны. Есть отличия toolchain/config, fuzz1 двух hunks, kernelrelease без плюса и отсутствие Module.symvers. См. qc7-build-preparation.md. Драйвер ещё не реализован/не собран/не загружен; это база для исследования API, не готовый совместимый модуль.

## 2026-09-08: формат состояний подтверждён PmicDxe

Извлечён PE ARM64 из FV0x486f8c, size0x2f000, SHA256 e28eb7f6a6e3c6d41a627e478e7a124ab55057a590952084801920dfe3c02401. Сразу после конца PE по FV0x4b5f90 следует UTF16 имя PmicDxe. Исходный бинарник остаётся приватным.

Связанный открытый Qualcomm PRM header подтверждает структуру resource record: group_name/max_mode/resource_attributes/resource_data/node_name/node_attributes/node_data/node_dependencies/dependency_count. Но его vreg struct содержит4поля (с headroom), тогда как TCL версия содержит3. Поэтому не применяли чужую структуру без проверки бинарника.

Подтверждение в исполняющем коде TCL:
- PmicDxe0x13098 использует шаг64байта для record;0x130d0/0x13100/0x13108 читает указатели resource_data/dependencies/count по0x10/0x30/0x38.
- Функция0x1576c обрабатывает ресурс;0x1580c индексирует массив rail_data с шагом16;0x15858 задаёт stride12,0x15868 вычисляет адрес состояния;0x1586c задаёт3ключа.
- Функция0x15a08 по key0/1/2 читает offsets0/4/8.
- В resource descriptors первого модуля:0x281c0 /pm/ldoc3/mode key0;0x28220 /pm/ldoc3/mV key1;0x28280 /pm/ldoc3/en key2. Для ldoa4 аналогично0x282e0/0x28340/0x283a0.

Следовательно, порядок тройки [mode,mV,enable] и значения напряжения подтверждены обработчиком, не только повторяемостью чисел.

| Resource | state0 | state1 | state2 |
|---|---|---|---|
| ldoc3 | mode4,1200mV,en0 | mode4,1200mV,en1 | mode7,1200mV,en1 |
| ldoa4 | mode4,880mV,en0 | mode4,880mV,en1 | mode7,880mV,en1 |
| ldoc8 | mode4,1800mV,en0 | mode4,1800mV,en1 | mode7,1800mV,en1 |

Enum mode4/7 ещё не назван; фактический выбор платформенного ресурса и запрос конкретного состояния DisplayDxe ещё не прослежены полностью. Это подтверждённое содержимое таблицы OEM, не измеренные напряжения и не распиновка панели. Настройки питания Linux не менялись.

Связанный исходник для сопоставления структуры:
https://github.com/Rivko/android-firmware-qti-sdm670/blob/20bb8ae36c93fc16bbadda0e0a83f930c0c8a271/boot_images/QcomPkg/Library/PmicLib/prm/inc/pm_prm_device.h
https://github.com/Rivko/android-firmware-qti-sdm670/blob/20bb8ae36c93fc16bbadda0e0a83f930c0c8a271/boot_images/QcomPkg/Library/PmicLib/prm/src/scalar/pm_prm_process_rsrc.c

### DisplayDxe: создание клиента и запрос state2

Panel_CLS_PowerUp0x1fb88:0x1fc38 получает имена ресурса/клиента из table0x44ee8 с шагом24; для DisplayPrim это /pmic/client/disp_edp.0x1fc3c вызывает wrapper создания клиента с type0x40.0x1fc58/0x1fc6c создаёт отдельный disp_bridge_refclk.0x1fd40 загружает display client handle,0x1fd44 задаёт state2,0x1fd58 вызывает wrapper0x1c2bc. Это статический путь PowerUp, не трасса текущей загрузки; подтверждена логика запроса state2 при прохождении ветви. Evidence display-edp-power-request.asm.
