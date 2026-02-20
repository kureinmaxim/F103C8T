# Быстрый старт: primGPT в Cursor на Windows

---

## Шаг 1 — Установить расширения

Открыть Extensions (`Ctrl+Shift+X`) и установить:

| Расширение | ID |
|------------|----|
| C/C++ IntelliSense | `ms-vscode.cpptools` |
| Cortex-Debug | `marus25.cortex-debug` |
| RTOS Views | `mcu-debug.rtos-views` |

---

## Шаг 2 — Открыть проект

`File → Open Folder` → выбрать папку:

```
C:\Project\ProjectSTM32\STM32F1\primGPT
```

> Открывать именно папку `primGPT`, не родительскую.

---

## Шаг 3 — Собрать проект

**`Ctrl+Shift+B`** → выбрать **Build**

Или через палитру: `Ctrl+Shift+P` → `Tasks: Run Task` → `Build`

Доступные задачи:

| Задача | Действие |
|--------|----------|
| `Build` | Сборка (по умолчанию) |
| `Clean` | Очистка выходных файлов |
| `Clean & Build` | Полная пересборка |
| `Flash` | Прошивка через OpenOCD |
| `Build & Flash` | Сборка + прошивка |
| `Firmware Size` | Размер прошивки (text/data/bss) |

---

## Шаг 4 — Прошить контроллер

Подключить ST-Link к плате, затем:

`Ctrl+Shift+P` → `Tasks: Run Task` → **`Flash`**

Или сразу сборка + прошивка: задача **`Build & Flash`**

---

## Шаг 5 — Отладка

1. Подключить ST-Link
2. Нажать **`F5`** (или открыть вкладку Run and Debug `Ctrl+Shift+D`)
3. Выбрать конфигурацию **Debug (OpenOCD)**

Отладчик автоматически соберёт проект и остановится на `main()`.

---

## Горячие клавиши

| Клавиша | Действие |
|---------|----------|
| `Ctrl+Shift+B` | Запустить сборку |
| `F5` | Начать отладку |
| `Shift+F5` | Остановить отладку |
| `F9` | Поставить/снять breakpoint |
| `F10` | Step Over |
| `F11` | Step Into |
| `Shift+F11` | Step Out |
| `Ctrl+Shift+P` | Палитра команд |
| `Ctrl+P` | Быстрый переход к файлу |

---

## Toolchain (пути настроены автоматически)

Файл `.vscode/tasks.json` уже содержит пути к инструментам из STM32CubeIDE 1.17.0:

- **GCC 13.3** — `gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344`
- **Make** — `make.win32_2.2.0.202409170845`
- **OpenOCD** — `openocd.win32_2.4.100.202501161620`

Ручная настройка PATH не требуется.

---

## OpenOCD: конфигурация и команды

### Рабочая конфигурация проекта

- `primGPT Debug.cfg`:
  - `source [find interface/stlink-dap.cfg]`
  - `transport select "dapdirect_swd"`
  - `set CLOCK_FREQ 4000`
  - `source [find target/stm32f1x.cfg]`

- OpenOCD binary:
  - `C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe`

- OpenOCD scripts (`-s`):
  - `C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts`

### Почему в Cursor падает `find interface/*.cfg`

Причина: OpenOCD запускается, но не знает путь к `st_scripts`.

Симптом: `Can't find interface/stlink-dap.cfg` (или `stlink.cfg`).

Лечение:
- всегда передавать `-s <.../st_scripts>` в команде OpenOCD;
- добавить этот же путь в `searchDir` в `.vscode/launch.json`.

### Команды PowerShell

**Проверка конфигурации:**

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg" `
  -c "shutdown"
```

Ожидаемо: OpenOCD стартует и завершится строкой `shutdown command invoked`.

**Прошивка ELF:**

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg" `
  -c "program Debug/primGPT.elf verify reset exit"
```

Ожидаемо: `** Programming Finished **`, `** Verified OK **`, `** Resetting Target **`.

**Поднять GDB server:**

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg"
```

Ожидаемо: `Listening on port 3333 for gdb connections`.

### Чеклист перед прошивкой

1. Папка проекта открыта именно `primGPT`.
2. `Build` проходит без ошибок.
3. ST-Link подключен и виден в системе.
4. Запущена задача `Flash` или `Build & Flash`.
5. В логе есть `Verified OK`.

---

## Troubleshooting

**IntelliSense не работает:**
`Ctrl+Shift+P` → `C/C++: Reset IntelliSense Database` → перезапустить Cursor

**`arm-none-eabi-gcc not found` при сборке:**
Проверить, что STM32CubeIDE 1.17.0 установлен в `C:\ST\`

**`openocd: command not found`:**
PATH в `.vscode/tasks.json` должен содержать каталог `...openocd.../tools/bin`, либо запускайте OpenOCD по полному пути (как в примерах выше).

**OpenOCD не видит ST-Link:**
1. Установить драйверы [STSW-LINK009](https://www.st.com/en/development-tools/stsw-link009.html)
2. Закрыть STM32CubeIDE (он захватывает ST-Link)
3. Проверить в Device Manager — ST-Link должен отображаться

**`Can't find interface/stlink*.cfg`:**
1. В задаче `Flash` должен быть аргумент `-s ...\st_scripts`
2. В `launch.json` добавить этот путь в `searchDir`
3. В `primGPT Debug.cfg` указан `source [find interface/stlink-dap.cfg]`

**Отладчик не останавливается на breakpoint:**
Убедиться, что прошивка актуальна — запустить `Build & Flash` перед `F5`

**Скорость SWD снижена (`requested 8000, using 4000`):**
Для этого проекта уже выставлено `set CLOCK_FREQ 4000`, предупреждения быть не должно.

---

## Подробная документация

- [VSCODE_WINDOWS_GUIDE.md](VSCODE_WINDOWS_GUIDE.md) — полное руководство
- [DEBUG_GUIDE.md](DEBUG_GUIDE.md) — руководство по отладке
- [RTOS_VIEWS_GUIDE.md](RTOS_VIEWS_GUIDE.md) — мониторинг задач и памяти FreeRTOS

---

*STM32F103C8T6 · FreeRTOS · STM32CubeIDE 1.17.0 · GCC 13.3*
