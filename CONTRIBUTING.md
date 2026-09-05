# Contributing

Contributions are welcome.

## Development environment

The project targets KDE Plasma 6 and currently prioritizes CachyOS / Arch Linux for development.

Install the required packages:

```bash
sudo pacman -S --needed \
  base-devel \
  cmake \
  extra-cmake-modules \
  libplasma \
  ninja \
  plasma-sdk \
  qt6-base \
  qt6-declarative
```

You also need a working authenticated Codex CLI installation:

```bash
codex --version
```

## First build

From the repository root, configure a development build once:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DCMAKE_INSTALL_PREFIX=/usr
```

Then build and run the tests:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

The project contains a compiled QML plugin, so local development installs currently target the normal Plasma / Qt system prefix:

```bash
sudo cmake --install build
```

## Normal development loop

After the first CMake configure, you normally do **not** need to run `cmake -S ...` again for every change.

For day-to-day development, use:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

If you are updating your local checkout first:

```bash
git pull
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```

CMake will regenerate the build files automatically when tracked CMake files change. Re-run the full configure command manually if you change build options, the install prefix, generator, or want to recreate the build directory.

## Testing the widget

### Fast iteration with `plasmoidviewer`

For UI work, the quickest loop is usually:

```bash
plasmoidviewer -a io.github.sangimed.codexmenubarkde
```

Close and relaunch `plasmoidviewer` after reinstalling the widget so it reloads the latest QML and native plugin.

### Testing in the real Plasma panel

If the widget is already added to your panel, Plasma may keep the previous QML or C++ plugin loaded in the running `plasmashell` process.

After installing a new build, force Plasma to reload it with:

```bash
systemctl --user restart plasma-plasmashell.service
```

Your panel will briefly disappear and come back. You normally **do not need to remove and re-add the widget**.

A practical full loop when testing in the panel is therefore:

```bash
git pull
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
systemctl --user restart plasma-plasmashell.service
```

Restarting `plasmashell` is especially important after changes to the native C++ plugin. For QML-only changes, Plasma can still cache the loaded component, so restarting it remains the most reliable way to validate the installed widget.

## Logs and troubleshooting

The widget runs inside `plasmashell`, so its Qt / QML and backend logs are available in the user journal.

Follow relevant logs with:

```bash
journalctl --user -f -o cat \
  | grep --line-buffered -Ei 'codex|codexmenubarkde|plasmoid'
```

Then interact with the widget or press **Refresh** to reproduce the issue.

For isolated testing, `plasmoidviewer` also makes runtime errors easier to spot:

```bash
plasmoidviewer -a io.github.sangimed.codexmenubarkde 2>&1 \
  | tee /tmp/codex-menubar-kde.log
```

The popup displays the exact Codex CLI executable selected by the backend. You can also verify the shell-visible executable with:

```bash
command -v codex
codex --version
```

## Code layout

- `package/contents/ui/` — Plasma QML UI and popup sections
- `package/contents/config/` — widget configuration schema and configuration pages
- `package/contents/images/` — project / widget artwork
- `src/CodexBackend.*` — long-lived Codex app-server process and JSON-RPC transport
- `src/RateLimitParser.*` — rate-limit classification and metadata parsing
- `src/UsageHistoryStore.*` — seven-day local usage history persistence
- `src/UsageNotificationManager.*` — low-quota desktop notifications
- `tests/` — Qt Test coverage
- `packaging/` — Arch / AUR packaging assets
- `.github/workflows/` — CI and release automation

Keep Codex protocol parsing and stateful backend behavior outside QML whenever possible so they remain independently testable.
