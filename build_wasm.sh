WASI_SDK_PATH="/opt/wasi-sdk"
WASI_SYSROOT_PATH="${WASI_SDK_PATH}/share/wasi-sysroot"

CC="clang++"

$CC -g -DPLATFORM_WEB_WASM -DPLATFORM_WEB --target=wasm32 --no-standard-libraries -Wl,--error-limit=0 -I${WASI_SYSROOT_PATH}/include/wasm32-wasi/c++/v1 -I${WASI_SYSROOT_PATH}/include/wasm32-wasi -Isrc -Isrc/engine -Isrc/game -Isrc/gl -Ithirdparty -Wl,--export-table -Wl,--no-entry  \
 -o wasm/main.wasm src/main.cpp src/game/game.cpp src/game/flappy_drawing.cpp src/engine/core.cpp src/engine/platform/platform_web_wasm.cpp src/engine/platform/wasm_stdc.c \
 -Wl,--export=main,--export=MainLoop,--export=malloc,--export=free \
 -Wl,--allow-undefined \
 -DRESOURCES_PATH="\"../resources/\"" \
