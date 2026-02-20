# Урок 6: Отладка в Cursor или Antigravity

Этот урок про одинаковую техническую основу: OpenOCD + ST-Link + GDB.
IDE может быть разной (Cursor/Antigravity), но pipeline один.

## 1) Базовая цепочка отладки

1. Собрать `Debug/primGPT.elf`.
2. Запустить OpenOCD с правильным `Run.cfg`/`primGPT Debug.cfg`.
3. Подключить GDB (через IDE или вручную).
4. Поставить breakpoint и проверить состояние задач/очередей.

## 2) Важный нюанс для Windows IDE

Если видите:
- `Can't find interface/stlink*.cfg`

значит OpenOCD запущен без scripts-каталога.

Решение:
- использовать `-s <.../st_scripts>`;
- либо убедиться, что в `launch.json` есть `searchDir` с этим путем.

## 3) Что отлаживать в этом проекте в первую очередь

### A) UART поток
- Точки останова:
  - `HAL_UART_RxCpltCallback` (`uart.c`)
  - начало обработки кадра в `Uart1Task` (`main.c`)
- Проверки:
  - байт попадает в очередь,
  - timeout кадра срабатывает корректно,
  - CRC отбраковывает неверные кадры.

### B) FreeRTOS память
- Смотреть лог `Heap` и `StkB`.
- Проверять watermark стеков на худшем сценарии.

### C) FRAM
- Проверять `fram_write`/`fram_read` по команде `0x02`.
- Если данные "плывут": начать с SPI waveform/CS timing.

## 4) Практический минимальный сценарий

1. `Build & Flash`.
2. Запустить debug (`F5` или аналог в Antigravity).
3. Отправить валидную UART-команду с CRC.
4. Убедиться, что:
   - breakpoint в `Uart1Task` достигается,
   - ответ отправляется,
   - ошибок heap/stack нет.

## 5) Что полезно добавить далее

- отдельный "debug command" для печати queue depth;
- трассировку ошибок протокола (bad CRC, timeout, unknown cmd);
- автоматический тест-скрипт для UART в PC.

