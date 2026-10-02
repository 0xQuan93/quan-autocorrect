# Quan Autocorrect

A local English autocorrect input method for Fcitx5 on Linux. It uses an
explicit list of 173 common typo forms, so unknown words remain unchanged.
Nothing is sent over the network or logged. It was built for Omarchy and
tested on Fcitx5 5.1.22 with Wayland.

## What it does

- Corrects known typos on Space, comma, exclamation mark, question mark, or
  semicolon: `teh ` becomes `the `.
- Keeps domains, paths, versions, identifiers, mixed-case names, and unknown
  words literal. Periods and colons do not trigger correction.
- Bypasses password and sensitive fields, URLs, email fields, terminals, and
  recognized code editors.
- Press **Escape** before Space to keep a spelling. Press **Ctrl+Alt+S** while
  composing a word for Hunspell suggestions, then **Alt+1–5** to choose one.
- Switch input methods with your Fcitx5 trigger, commonly **Ctrl+Space**.

Apps that do not support input methods cannot receive these corrections.
Chromium on Wayland may need its Wayland IME flags and a full restart; see the
[Fcitx5 Wayland guide](https://fcitx-im.org/wiki/Using_Fcitx_5_on_Wayland).

## Requirements

- Linux with Fcitx5 Core **5.1.22 or newer**, its Spell module and development
  headers, a C++20 compiler, CMake, Python 3, and Enchant with Hunspell.
- An English Hunspell dictionary for suggestions. On Arch/Omarchy, the package
  is `hunspell-en_us`. The bundled dictionary is used by the isolated tests.

On Arch/Omarchy, these packages provide the build and test requirements:

```sh
sudo pacman -S --needed fcitx5 enchant hunspell hunspell-en_us gcc cmake ninja python
```

## Build and test

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

The test runs with temporary XDG directories and uses only the bundled
dictionary. It checks correction, protected contexts, shortcut handling,
punctuation, composition, and spelling suggestions without changing your Fcitx5
profile. A pass here checks the engine; it does not replace a live typing test
in each desktop application.

## Install for one user

These commands copy only the built addon and its Fcitx5 metadata. They do not
change your Fcitx5 profile, desktop service, browser flags, or system files.
Review existing files at these paths before overwriting an older installation.

```sh
install -Dm755 build/libquan-autocorrect.so "$HOME/.local/lib/fcitx5/libquan-autocorrect.so"
install -Dm644 quan-autocorrect.conf "$HOME/.local/share/fcitx5/addon/quan-autocorrect.conf"
install -Dm644 english-autocorrect.conf "$HOME/.local/share/fcitx5/inputmethod/english-autocorrect.conf"
```

Fcitx5 must search the user addon library directory when it starts. On the
tested Arch/Omarchy layout, add
`FCITX_ADDON_DIRS=$HOME/.local/lib/fcitx5:/usr/lib/fcitx5` to the environment
of the service or command that starts Fcitx5. Keep any other addon directories
your installation already uses. Restart Fcitx5, then add **English Autocorrect**
to your input-method group with the Fcitx5 configuration tool and select it.
Use `fcitx5-remote -n` to confirm the active method is
`english-autocorrect`. If you use a different distribution, check its system
Fcitx5 addon library directory before setting `FCITX_ADDON_DIRS`.

To remove the addon, select another input method, stop Fcitx5, and remove the
three files copied above. Remove the added directory from your Fcitx5 startup
environment if it is no longer needed, then start Fcitx5 again.

## Licensing

Original addon code is MIT licensed, © 2026 0xQuan. The bundled dictionary has
separate attribution and redistribution terms; see
[third-party notices](THIRD_PARTY_NOTICES.md) and its full
[license text](dictionary/README_en_US.txt).
