# EpoxyUpdate

A host-build version of the ESP32 Arduino `Update` class (`Update.h`) for
programs compiled with EpoxyDuino. It is used by the native build of
[OpenEVSE](https://github.com/OpenEVSE/openevse_esp32_firmware), which downloads
and installs firmware on the device.

Every call succeeds, so code that installs an update runs without a flash
partition behind it. The difference from a plain stub is that the update can be
recorded for a test to inspect.

## Recording an update

Set the environment variable `EPOXY_UPDATE_SINK_DIR` to a directory before the
program starts. The directory must exist. Each update then writes three files:

| File             | Written by     | Contents                                                |
|------------------|----------------|---------------------------------------------------------|
| `ota_begin.json` | `begin(size)`  | `{"source":"<url>","begin_size":<size>}`                |
| `ota_image.bin`  | `write(...)`   | every byte passed to `write()`, in order                |
| `ota_end.json`   | `end()` / `abort()` | `{"ok":true,"written":<bytes>}` or `"ok":false`    |

`begin_size` is the value the program passed to `begin()`, so `UPDATE_SIZE_UNKNOWN`
is recorded as `4294967295`. A new `begin()` replaces the previous record.

With the variable unset, nothing is written to disk and every call behaves as
before.

## Recording where the update came from

The updater cannot see the URL a download came from, so the program names it
before calling `begin()`:

```cpp
#if defined(EPOXY_DUINO)
  EpoxyUpdateSetSource(url);
#endif
Update.begin(size);
```

`begin()` clears the source, so an upload that follows a download is recorded
with an empty `source`. The call is only declared for host builds; code for real
hardware must not call it.

## Usage

Add `EpoxyUpdate` to `ARDUINO_LIBS` in a Makefile that includes `EpoxyDuino.mk`,
or add the `libraries/EpoxyUpdate/src` directory to the include path of a
PlatformIO native build and compile `src/Update.cpp`.

## Tests

```sh
cd tests
make runtests
```

## Limitations

* Writes are not checked against a partition size, and `end()` always succeeds.
* `isRunning()` is always `false`, as in the stub this replaces.
* There is no flash, so a test cannot check what was written to a partition.
