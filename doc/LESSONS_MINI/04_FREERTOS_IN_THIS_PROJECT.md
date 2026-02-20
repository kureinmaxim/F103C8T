# Урок 4: FreeRTOS в проекте — от базовых задач к диагностике памяти

## 1) Как развивалась идея RTOS

- Без RTOS: один суперцикл, сложно масштабировать.
- С RTOS: отдельные задачи по ролям, приоритеты, очереди, синхронизация.
- Для embedded это стандарт, когда есть несколько интерфейсов и реакция в реальном времени.

## 2) Как устроено у вас

Задачи:
- `StartDefaultTask` — периодический лог и диагностика.
- `LedTask` — индикация.
- `Uart1Task` — обработка команд (повышенный приоритет).

Очередь:
- `uartQueue = xQueueCreate(40, sizeof(char));`

## 3) Память RTOS

- `configTOTAL_HEAP_SIZE` задан в `Core/Inc/FreeRTOSConfig.h`.
- В проекте добавлен runtime-лог:
  - `Heap: free/min`
  - `StkB: ...` (водяные знаки стеков в байтах).

## 4) Где смотреть код

- Создание задач и очереди: `Core/Src/main.c`
- Диагностика heap/stack в `StartDefaultTask`: `Core/Src/main.c`

## 5) Путь роста

1. Стабилизировать watermark стеков.
2. Добавить hooks:
  - `vApplicationStackOverflowHook`
  - `vApplicationMallocFailedHook`
3. Перейти на более формальный протокол и статистику очередей.

