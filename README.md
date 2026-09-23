# Deus Translate

A localization (i18n) plugin for **SA:MP** servers. Load per-language message
files as JSON, assign a language to each player, and send messages by key — the
plugin resolves the right text for each player automatically.

## Features

- Load any number of languages from JSON files at runtime.
- One key → one message per language (same keys across all languages).
- Per-player language assignment.
- `SendLanguageMessage` resolves a key for the player's language, applies
  `printf`-style formatting, and sends it with `SendClientMessage`.
- Fallback chain: player language → default language → the literal key, so
  missing translations are obvious in-game.
- No external dependencies (self-contained JSON parser).
- Windows (`.dll`) and Linux (`.so`), 32-bit.

## How it works

1. In `OnGameModeInit` you call `Lang_Load` once per language, pointing at a
   JSON file. The plugin parses it into an in-memory `key -> text` map, keyed by
   an (uppercased) language code.
2. `Lang_SetDefault` sets the fallback language, used when a player has no
   language assigned or a key is missing.
3. `Lang_SetPlayer` stores a language code for a player id.
4. When you call `SendLanguageMessage(playerid, color, key, ...)`, the plugin:
   - looks up the player's language (or the default),
   - finds `key` in that language (falling back to the default language, then
     to the literal key),
   - formats the template with the extra arguments,
   - sends the result via the server's `SendClientMessage`.

## Installation

Ready-to-use files live in the [`Plugin/`](Plugin/) folder. The Windows `.dll`
is included; the Linux `.so` is produced by CI (download it from the latest
**Actions** run, or build it yourself — see *Building from source*).

1. Plugin binary → server `plugins/`:
   - Windows: `Plugin/plugins/deus_translate.dll`
   - Linux: `Plugin/plugins/deus_translate.so`
2. Register it in `server.cfg` (one line):

   ```
   plugins deus_translate
   ```

   On Linux, some server builds expect the extension: `plugins deus_translate.so`.

3. Include → your Pawn include path (e.g. `pawno/include/`):
   `Plugin/pawno/include/deus_translate.inc`, then in your script:

   ```pawn
   #include "deus_translate"
   ```
4. Locale files → server `scriptfiles/locales/`:
   copy `Plugin/scriptfiles/locales/*.json`.

## Locale files

One JSON file per language — a flat object of `key -> text`:

```json
{
    "MSG_HOLA":   "Hello, %s! Welcome to the server.",
    "MSG_DINERO": "You have $%d in the bank.",
    "MSG_BYE":    "Goodbye, see you!"
}
```

- Keep the **same keys** in every language.
- Values may contain `printf`-style placeholders (see below).
- Nested objects are supported and flattened with a dot:
  `{"cmd":{"help":"..."}}` becomes the key `cmd.help`.
- A UTF-8 BOM, if present, is stripped automatically.

### Encoding (important)

SA:MP does not use UTF-8 on the client; text is sent as raw bytes and rendered
with the client's charset. **This plugin passes bytes through unchanged — it
does not convert encodings.** Save each locale file in the charset the client
expects (for example Windows-1251 for Cyrillic). A UTF-8 file with non-ASCII
characters will look garbled in-game. `\uXXXX` escapes are only kept when they
fit in a single byte (<= 0xFF); otherwise they become `?`.

## Native reference

### Lang_Load
```pawn
native Lang_Load(const code[], const file[]);
```
Loads `file` (JSON) and associates it with language `code` (case-insensitive).
The path is relative to the **server root** (where `samp-server.exe` /
`samp03svr` lives), not `scriptfiles`. Returns `1` on success, `0` if the file
can't be read or parsed. Call it in `OnGameModeInit`. Loading the same code
again replaces that language's data.

### Lang_SetDefault
```pawn
native Lang_SetDefault(const code[]);
```
Sets the global fallback language, used when a player has no language assigned
or their language is missing a key.

### Lang_SetPlayer
```pawn
native Lang_SetPlayer(playerid, const code[]);
```
Assigns a language to a player. Returns `0` if that language was never loaded
(and changes nothing), `1` otherwise.

### Lang_GetPlayer
```pawn
native Lang_GetPlayer(playerid, dest[], size = sizeof dest);
```
Writes the player's current language code (or the default if none) into `dest`.
Returns the length of the code.

### Lang_GetText
```pawn
native Lang_GetText(playerid, const key[], dest[], size = sizeof dest);
```
Resolves `key` for the player's language (with fallback to the default) and
writes the raw text into `dest`, without sending anything. Returns `1` if found;
if not found it copies `key` itself into `dest` and returns `0`.

### SendLanguageMessage
```pawn
native SendLanguageMessage(playerid, color, const key[], {Float,_}:...);
```
Resolves `key` for the player's language, formats it with the extra arguments,
and sends it with `SendClientMessage(playerid, color, ...)`. If the key can't be
found anywhere, the literal key is sent (handy while adding translations).

```pawn
// es.json: "MSG_HOLA": "¡Hola, %s! Tenés $%d."
Lang_SetPlayer(playerid, "ES");
SendLanguageMessage(playerid, -1, "MSG_HOLA", "Lucas", 2500);
// -> ¡Hola, Lucas! Tenés $2500.
```

## Formatting specifiers

Templates resolved by `SendLanguageMessage` support:

| Specifier  | Meaning                        |
|------------|--------------------------------|
| `%s`       | string argument                |
| `%d`, `%i` | signed integer                 |
| `%u`       | unsigned integer               |
| `%x`       | hex integer                    |
| `%c`       | single character               |
| `%f`       | float (6 decimals by default)  |
| `%.Nf`     | float with N decimals (`%.2f`) |
| `%%`       | literal percent sign           |

Arguments are consumed left to right. Extra arguments are ignored; a specifier
with no matching argument produces empty output.

## Building from source

32-bit only (SA:MP is 32-bit). All build files live in [`Source/`](Source/).

### Windows (`.dll`)
Requires Visual Studio with the "Desktop development with C++" workload
(x86 tools):
```
Source\build.bat
```
Output: `Source\bin\Release\deus_translate.dll`.

### Linux (`.so`)
Requires a 32-bit g++ toolchain (`g++-multilib` on 64-bit hosts):
```
sudo apt-get install g++-multilib   # Debian/Ubuntu
make -C Source
```
Output: `Source/deus_translate.so`.

### CI (GitHub Actions)
`.github/workflows/build.yml` builds both targets on every push and uploads
`deus_translate.dll` and `deus_translate.so` as downloadable artifacts.

## Example gamemode

See [`Plugin/gamemodes/test.pwn`](Plugin/gamemodes/test.pwn) for a runnable demo
with `/es`, `/en`, `/ru`, `/hola`, `/dinero`, `/bye` and `/lang` commands.

## Notes & limitations

- Player ids `0 .. 999` are supported (SA:MP `MAX_PLAYERS`).
- `SendClientMessage` truncates around 144 characters; keep messages within it.
- Language codes are case-insensitive (stored uppercased internally).
- The plugin reads locale files from disk only when `Lang_Load` is called.

## Project layout

```
Source/                              build everything from here
  src/main.cpp                       entry points + AMX bridge + natives
  src/lang.h, src/lang.cpp           language store + JSON parser
  sdk/                               SA:MP plugin SDK (vendored)
  deus_translate.vcxproj / .sln      Visual Studio project (Win32)
  deus_translate.def                 exported plugin symbols
  build.bat                          Windows build (MSBuild)
  Makefile                           Linux build (g++ -m32)
Plugin/                              ready-to-use distribution
  plugins/deus_translate.dll         compiled plugin (.so added by CI)
  pawno/include/deus_translate.inc   Pawn include (natives)
  scriptfiles/locales/*.json         example locale files
  gamemodes/test.pwn                 example gamemode
.github/workflows/build.yml          CI: builds .dll + .so
```




