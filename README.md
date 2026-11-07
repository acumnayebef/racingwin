# XantaMenu — Highway Racer, offline currency menu (iOS, no jailbreak, no JIT)

Target: UnityFramework, Unity 2022.3.62f3, IL2CPP arm64 (UUID 481e02c5681a3f7b984e72343abc779f).
Method: pure `il2cpp_*` API calls (data only, no code patching) -> works without JIT.

## Build (needs macOS; on Windows use GitHub Actions)
Push this folder to a GitHub repo -> Actions -> `build` -> download artifact `XantaMenu-dylib`.
Local macOS: `make`  ->  `build/XantaMenu.dylib`

## Inject
Sideloadly: Advanced Options -> add `XantaMenu.dylib` to "Inject dylibs/frameworks", sign and install the IPA.

## Use
Tap the ✦ orb (draggable). CURRENCY tab: set AMOUNT, then +ADD / SET per currency. CREATORS tab: @xantapers.
The status line shows READY or the exact missing class/method/export.

## Where build-specific names live
`src/game_config.hpp` (namespace, class, method names). No RVAs are used.
