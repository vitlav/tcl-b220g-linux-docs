## Ошибка
В Linux 6.18.34, sound/soc/qcom/qdsp6/q6afe.c, q6afe_cdc_dma_port_prepare() не переносит ненулевую cfg->active_channels_mask в dma_cfg->active_channels_mask. Присваивание выполняется только при нулевой входной маске:

```c
if (!cfg->active_channels_mask)
    dma_cfg->active_channels_mask = (1 << cfg->num_channels) - 1;
```

В результате явно заданная маска теряется: в первоначально обнулённой конфигурации DSP получает ноль; при повторном использовании возможно сохранение предыдущего значения.

## Воспроизведение на TCL B220G
Ядро 6.18.34-tcl-ice1, Qualcomm SC7180, OEM ADSP работает, Q6AFE API 7.
Диагностическая ASoC-карта TCL-ADSP-Diagnostic: Q6ASM MultiMedia1 → Q6 routing → RX_CODEC_DMA_RX_0 (AFE 0xb030), snd-soc-dummy. Это тест цифрового тракта, не подтверждение настройки физического кодека.

1. Задать через snd_soc_dai_set_channel_map() active_channels_mask=3, два канала.
2. Включить RX_CODEC_DMA_RX_0 Audio Mixer MultiMedia1.
3. Запустить:
```sh
aplay -v -D hw:TCLADSPDiagnost,0 -t raw -f S16_LE -r48000 -c2 -d2 /dev/zero
```
Результат: Unable to install hw params; DSP для команды 0x100ef (AFE_PORT_CMD_SET_PARAM_V2) возвращает 0x2, ядро -22, fail to start AFE port 71.

Контроль в той же загрузке: пересоздать только диагностическую карту с active_channels_mask=0. Драйвер сам вычисляет 3. hw_params устанавливаются, прежняя ошибка AFE исчезает. Позднее запись PCM завершается Input/output error — это отдельная ещё не локализованная проблема; звук пока не работает. ADSP остаётся running.

## Требуемое исправление
Всегда присваивать исходящую маску: явное cfg->active_channels_mask либо вычисленное значение, если оно равно нулю. Проверить ненулевые маски (включая не совпадающие с последовательной маской по числу каналов), нулевой вариант и повторную конфигурацию.

Кандидат:
```c
dma_cfg->active_channels_mask = cfg->active_channels_mask;
if (!dma_cfg->active_channels_mask)
    dma_cfg->active_channels_mask = (1 << cfg->num_channels) - 1;
```

Исправленное ядро/модуль пока не установлены. Проверка актуального upstream и отправка upstream пока не выполнены.
Связанная общая задача: #19084. Эта ошибка блокирует корректную настройку явной маски каналов в работах по звуку TCL.
