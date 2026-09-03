# Contributing

Contributions are welcome.

## Development environment

On CachyOS or Arch Linux:

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

You also need a working authenticated Codex CLI installation.

## Build and test

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON \
  -DCMAKE_INSTALL_PREFIX=/usr

cmake --build build
ctest --test-dir build --output-on-failure
```

## Install locally

The project contains a compiled QML plugin, so development installs currently target the normal Plasma/Qt prefix:

```bash
sudo cmake --install build
```

Then test with:

```bash
plasmoidviewer -a io.github.sangimed.codexmenubarkde
```

If Plasma already loaded an older plugin binary, restart `plasmoidviewer` before testing again.

## Code layout

- `package/contents/ui/` — Plasma QML UI
- `package/contents/config/` — widget configuration schema
- `src/CodexBackend.*` — long-lived Codex app-server process and JSON-RPC transport
- `src/RateLimitParser.*` — rate-limit classification and metadata parsing
- `tests/` — Qt Test coverage

Keep Codex protocol parsing outside QML so it remains independently testable.
