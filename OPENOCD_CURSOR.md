# OPENOCD_CURSOR.md

Краткий рабочий справочник по прошивке и отладке STM32 в Cursor через OpenOCD (Windows).

---

## 1) Рабочая конфигурация для этого проекта

В проекте используется:

- `primGPT Debug.cfg`:
  - `source [find interface/stlink-dap.cfg]`
  - `transport select "dapdirect_swd"`
  - `set CLOCK_FREQ 4000`
  - `source [find target/stm32f1x.cfg]`

- OpenOCD binary:
  - `C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe`

- OpenOCD scripts (`-s`):
  - `C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts`

---

## 2) Почему в Cursor часто падает `find interface/*.cfg`

Причина:
- OpenOCD запускается, но не знает путь к `st_scripts`.

Симптом:
- `Can't find interface/stlink-dap.cfg` (или `stlink.cfg`).

Лечение:
- всегда передавать `-s <.../st_scripts>` в команде OpenOCD;
- добавить этот же путь в `searchDir` в `.vscode/launch.json`.

---

## 3) Минимальные команды

## 3.1 Проверка, что конфиг вообще читается

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg" `
  -c "shutdown"
```

Ожидаемо: OpenOCD стартует и завершится строкой `shutdown command invoked`.

## 3.2 Прошивка ELF

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg" `
  -c "program Debug/primGPT.elf verify reset exit"
```

Ожидаемо:
- `** Programming Finished **`
- `** Verified OK **`
- `** Resetting Target **`

## 3.3 Поднять GDB server

```powershell
& "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.openocd.win32_2.4.100.202501161620\tools\bin\openocd.exe" `
  -s "C:\ST\STM32CubeIDE_1.17.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.debug.openocd_2.3.200.202510310951\resources\openocd\st_scripts" `
  -f "primGPT Debug.cfg"
```

Ожидаемо: `Listening on port 3333 for gdb connections`.

---

## 4) Что настроено в проекте под Cursor

Уже обновлены:

- `.vscode/tasks.json` и `.vscode/tasks_windows.json`
  - у задачи `Flash` добавлен `-s ...\st_scripts`.

- `.vscode/launch.json`
  - в `searchDir` добавлен путь к `st_scripts`.

Это позволяет запускать `Flash`, `Build & Flash` и `F5` без ручного редактирования команд.

---

## 5) Типовые ошибки и быстрый фикс

## Ошибка 1: `Can't find interface/stlink*.cfg`

Проверьте:
1. в задаче `Flash` есть `-s ...\st_scripts`;
2. в `launch.json` добавлен этот путь в `searchDir`;
3. в `primGPT Debug.cfg` указан правильный `source [find interface/stlink-dap.cfg]`.

## Ошибка 2: `openocd: command not found`

Проверьте:
- PATH в `.vscode/tasks*.json` содержит каталог `...openocd.../tools/bin`;
- либо запускайте OpenOCD по полному пути (как в примерах выше).

## Ошибка 3: ST-Link не найден

Проверьте:
1. драйвер ST-Link установлен (`STSW-LINK009`);
2. кабель/питание платы;
3. STM32CubeIDE и другие инструменты закрыты (не должны держать ST-Link).

## Ошибка 4: скорость SWD снижена (`requested 8000, using 4000`)

Для этого проекта уже выставлено `set CLOCK_FREQ 4000`, поэтому предупреждения больше не должно быть.

---

## 6) Короткий чеклист перед прошивкой

1. Папка проекта открыта именно `primGPT`.
2. `Build` проходит без ошибок.
3. ST-Link подключен и виден в системе.
4. Запущена задача `Flash` или `Build & Flash`.
5. В логе есть `Verified OK`.

