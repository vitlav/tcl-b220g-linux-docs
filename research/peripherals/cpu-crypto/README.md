# TCL: криптографические инструкции CPU

2026-09-09, ядро6.18.34-tcl-ice1. /proc/cpuinfo: aes,pmull,sha1,sha2,asimd. OpenSSL3.5.5 Ubuntu ARM64 автоматически определил OPENSSL_armcap=0xbd. OpenSSH10.2p1 Ubuntu-2ubuntu3.6 связан с libcrypto.so.3 этой библиотеки. В kernel Crypto API зарегистрированы aes-ce,xts-aes-ce,gcm-aes-ce и NEON реализации. Это независимо от UFS ICE.

## Измерение A/B/A

На CPU6, блок16384байт, каждый запуск3секунды elapsed:

| Режим | AES-256-GCM, МБ/с (10^6 байт) |
|---|---:|
| Авто0xbd, A1 |1825.84934|
| Без AES/PMULL,0x99 |103.92371|
| Авто0xbd, A2 |1832.53948|

Разница~17.6раз. Команды:

```sh
taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
env OPENSSL_armcap=0x99 taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
taskset -c 6 openssl speed -elapsed -seconds 3 -bytes 16384 -evp aes-256-gcm
```

0x99=0xbd без битов AES0x04 и PMULL0x20. Override действует только на один процесс, системных изменений нет. В фоне работало окно камеры; частоты не фиксировались. Это короткий тест библиотеки в памяти, НЕ throughput OpenVPN/SSH и не измерение сети. OpenVPN на TCL не установлен/command-v не дал пути; не утверждать проверку его конкретной сборки или VPN-туннеля. OpenVPN может собираться с OpenSSL/mbedTLS; DCO использует kernel data path. SSH использует AES-ускорение при выборе соответствующего AES cipher, но выбор алгоритма текущей SSH сессии этим тестом не фиксировался.

Источники: https://docs.openssl.org/3.5/man3/OPENSSL_armcap/ ; https://build.openvpn.net/man/openvpn-2.6/openvpn.8.html .
