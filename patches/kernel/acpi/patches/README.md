# TCL B220G ACPI patch series

Каталог содержит 70 логически разделённых `git format-patch` файлов. Порядок применения задан файлом [series](series). Серия начинается с Linux stable v7.2.4 commit `5015d0d945b3d3f2b038d2667880d5762f7d9437` и заканчивается точным tree `885e8419bbeebe4f34b47d7e692311e301004167`, аппаратно проверенным как `7.2.4-tcl-acpi-repro1+`.

Основные группы идут в порядке зависимостей:

1. защита общего ACPI/fwnode/remoteproc/DRM/interconnect кода;
2. Qualcomm SCM, SMMU, RPMh, SMEM, GLINK и remoteproc ресурсы;
3. Wi-Fi, pinctrl, I²C и ввод;
4. DPU/DSI/LT8911 и Adreno;
5. APR/Q6AFE, LPASS, SoundWire и WCD9385;
6. UFS, Venus и их IORT/SMMU связи;
7. generic fwnode providers, CPUFreq, ACPI idle и финальная конфигурация.

Имена файлов описывают конечную функцию патча. Промежуточные отладочные изменения и отменённые варианты в серию не входят. SHA256 каждого файла и результат независимого replay записаны в [manifest.json](../manifest.json).
