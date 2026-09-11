# Происхождение qc7 до изменений TCL

Исходная сборка `6.18.34-stb-qc7+` — Linux stable с набором изменений для ноутбуков SC7180. Это спецификация происхождения исходной базы; собственные изменения TCL для DT и ACPI в неё не входят. Материалы этого каталога не являются общей зависимостью двух комплектов TCL и не предназначены для применения всей пачкой к новому ядру.

## Точные источники

- [Коммит qc7 `3fbde69c6a0b`](https://github.com/hexdump0815/linux-mainline-qcom-kernel/commit/3fbde69c6a0b31e7cba94b036301c003723c3c31): адаптация патчей к 6.18 и добавление gamma LUT.
- [Рецепт сборки в этом коммите](https://github.com/hexdump0815/linux-mainline-qcom-kernel/blob/3fbde69c6a0b31e7cba94b036301c003723c3c31/readme.qc7): sc7180_defconfig с дополнительными конфигурационными фрагментами; сборка Image, DTB, модулей, perf и cpupower.
- [Исходный диапазон TravMurav](https://github.com/TravMurav/linux/compare/adc218676eef25575469234709c2d87185ca223a...1cc9339f8b132e9f95b95eeb92cc9c07d49e7bce): 16 коммитов, указанных в заголовке объединённого патча qc7.
- [Оригинальные коммиты в mbox](travmurav-original-commits.mbox): сохранены авторы, сообщения и diff каждого коммита. Порядок и все 16 SHA сверены с GitHub API.
- [Манифест](manifest.json): полные SHA коммитов, Git blob и SHA256 файлов.

## Коммиты исходного диапазона

Названия приведены без исправления авторских пометок HACK/FIXME/REVERTME. Их наличие не означает, что изменение пригодно для upstream или всё ещё требуется в актуальном ядре.

| Коммит | Исходное название |
|---|---|
| [`d7fe3796aae4`](https://github.com/TravMurav/linux/commit/d7fe3796aae4b020c05e02024a03e69bc4c288e0) | FROMLIST: clk: qcom: dispcc-sc7180: Force off rotator clk at probe |
| [`fd0f5a756d15`](https://github.com/TravMurav/linux/commit/fd0f5a756d15af59deeb29b0c18a1853615cfeea) | HACK: drm/msm/dpu: Give each CRTC a dedicated cursor and primary |
| [`e1e5e031d9b7`](https://github.com/TravMurav/linux/commit/e1e5e031d9b7685a49fac11e7d2b9bc033f06e71) | HACK: thermal: gov_power_allocator: Suppress sustainable_power warning without trip_points |
| [`43338c78638a`](https://github.com/TravMurav/linux/commit/43338c78638a5434cc9c34298e77d7fcc2c5f523) | MSM8916: REVERTME: Add postmarketOS config fragment (v28) |
| [`543164bef528`](https://github.com/TravMurav/linux/commit/543164bef528dd00e3f8ec3ce78ee3ceb4cb8536) | REVERTME: Add sc7180 defconfig |
| [`58733478566c`](https://github.com/TravMurav/linux/commit/58733478566cb4d7859db97202dbf29ee62a287f) | FIXME: platform: arm64: acer-aspire1-ec: Implement UCSI support |
| [`bf69cdd2765e`](https://github.com/TravMurav/linux/commit/bf69cdd2765e0ab95c017f20cfb47284ef8a3701) | FIXME: firmware: qcom: uefisecapp: Add Aspire 1 to allowlist |
| [`bc37c945b922`](https://github.com/TravMurav/linux/commit/bc37c945b9225c45f6bd35ba836be660ac9d86b5) | ASoC: qcom: sc7180: Add support for VA macro on ADSP |
| [`49117cdecfd5`](https://github.com/TravMurav/linux/commit/49117cdecfd59981eeb1244d21f7adf4f1a0765b) | pinctrl: qcom: sc7280-lpi: Add compatible for sc7180 |
| [`4c438de77ce2`](https://github.com/TravMurav/linux/commit/4c438de77ce2a8cc7df5be1947865b601b2f9e51) | arm64: dts: qcom: sc7180: Add lpass-lpi-pinctrl |
| [`3dfc20a19752`](https://github.com/TravMurav/linux/commit/3dfc20a197524e92bb10c92172bd767725f11418) | arm64: dts: qcom: sc7180: Add VA macro |
| [`2637dc2cbba0`](https://github.com/TravMurav/linux/commit/2637dc2cbba04da60fce3156902ba0611a4b30a8) | FIXME: ASoC: q6dsp: q6afe: Tack on client_handle tracking |
| [`dfa7b00cc17d`](https://github.com/TravMurav/linux/commit/dfa7b00cc17dbb0a3767a8eee5eeb116f50267ff) | pinctrl: qcom: sc7280-lpi: Add power management |
| [`283c8d3f8579`](https://github.com/TravMurav/linux/commit/283c8d3f8579db2d3698b1e2c668a62e04a7090a) | ASoC: codecs: lpass-va-macro: Disable all clocks in suspend |
| [`05af8778b390`](https://github.com/TravMurav/linux/commit/05af8778b3903d40e46c68194a80e01cebcd04fe) | arm64: dts: qcom: acer-aspire1: Add lid microphone |
| [`1cc9339f8b13`](https://github.com/TravMurav/linux/commit/1cc9339f8b132e9f95b95eeb92cc9c07d49e7bce) | Revert "arm64: dts: qcom: acer-aspire1: Add lid microphone" |

Последние два коммита добавляют и затем отменяют описание микрофона крышки Acer. В результирующем diff этого изменения нет. Остальные изменения затрагивают питание дисплея, распределение DPU-плоскостей, LPASS/VA macro и Q6AFE, Acer EC/UCSI и QSEECom, конфигурации и предупреждение thermal governor.

## Фактически применявшиеся патчи qc7

Все три локальные копии ниже проверены по Git blob из коммита qc7, совпадение побайтное:

| Файл | Назначение |
|---|---|
| [travmurav-changes-v6.12.patch](travmurav-changes-v6.12.patch) | Объединённый диапазон TravMurav, адаптированный сопровождающим qc7 к 6.18 |
| [add-gamma-lut-support.patch](add-gamma-lut-support.patch) | DPU gamma LUT: цветокоррекция и ночной режим |
| [add-new-dts-files-to-makefile.patch](add-new-dts-files-to-makefile.patch) | Добавление сборки чернового DT Samsung Galaxy Book Go |

Gamma LUT происходит из [публикации патча v3](https://patchwork.kernel.org/project/linux-arm-msm/patch/20251019-dpu-add-dspp-gc-driver-v3-1-840491934e56@izzo.pro/), указанной в заголовке файла. Отдельный исходный Git-коммит этого патча здесь не установлен; коммит, включивший его в qc7, известен.

Рецепт также копирует [черновой DTS Samsung](https://github.com/hexdump0815/linux-mainline-qcom-kernel/blob/3fbde69c6a0b31e7cba94b036301c003723c3c31/misc.qc7/misc/sc7180-samsung-galaxy-book-go.draft) и применяет исправление суффикса версии из [kernel-extra-patches, fix-kernel-version/v6.12.5.patch](https://github.com/hexdump0815/kernel-extra-patches/blob/2a183c612916fb0d33333f1dfbc6d7ad9156f14d/fix-kernel-version/v6.12.5.patch). SHA `2a183c612916...` относится к репозиторию вспомогательных патчей, а не к коммиту ядра или qc7.

## Использование при переносе

Исходные коммиты можно получить из репозитория TravMurav и переносить по одному. Mbox сохраняет их содержание, но `git am` создаст новые SHA; адаптированный qc7 diff также не тождественен исходной серии. Перед переносом следует проверять наличие эквивалентного исправления в новом upstream и его необходимость для TCL. В первую очередь значимы исправление ROT/GDSC и обработка DSP client_handle. Поддержка Acer EC и Samsung DT сама по себе не подтверждает поддержку TCL. Аппаратная проверка этих изменений по отдельности на новом ядре пока не выполнена.
