// Based on Raylib wasm implementation by Alexey Kutepov at https://github.com/tsoding/zozlib.js


function make_environment(env) {
    return new Proxy(env, {
        get(target, prop, receiver) {
            if (env[prop] !== undefined) {
                return env[prop].bind(env);
            }
            return (...args) => {
                throw new Error(`NOT IMPLEMENTED: ${prop} ${args}`);
            }
        }
    });
}

function goFullScreen(){
    var canvas = document.getElementById("game");
    if(canvas.requestFullScreen)
        canvas.requestFullScreen();
    else if(canvas.webkitRequestFullScreen)
        canvas.webkitRequestFullScreen();
    else if(canvas.mozRequestFullScreen)
        canvas.mozRequestFullScreen();
}

class MlnlibJs {
    #reset() {
        this.previous = undefined;
        this.exports = undefined;
        this.ctx = undefined;
        this.dt = undefined;
        this.targetFPS = 60;
        this.entryFunction = undefined;
        this.currentPressedKeyState = new Set();
        this.currentPressedMouseButtonState = new Set();
        this.currentMouseWheelMoveState = 0;
        this.currentMousePosition = {x: 0, y: 0};
        this.images = [];
        this.sounds = new Map();
        this.next_sound_id = 0;
        this.fonts = new Map();
        this.next_font_id = 0;
        this.quit = false;
        this.projection_matrix = {a:1.0, b:0, c:0, d:1.0, e:0, f:0};
        this.view_matrix = {a:1.0, b:0, c:0, d:1.0, e:0, f:0};
    }

    constructor() {
        this.#reset();
    }

    stop() {
        this.quit = true;
    }

    async startExports({ exports, canvasId }) {
        if (this.exports !== undefined) {
            console.error("The game is already running. Please stop() it first.");
            return;
        }

        const canvas = document.getElementById(canvasId);
        this.ctx = canvas.getContext("2d");
        if (this.ctx === null) {
            throw new Error("Could not create 2d canvas context");
        }

        this.exports = exports;

        const keyDown = (e) => {
            this.currentPressedKeyState.add(glfwKeyMapping[e.code]);
        };
        const keyUp = (e) => {
            this.currentPressedKeyState.delete(glfwKeyMapping[e.code]);
        };
        const wheelMove = (e) => {
            this.currentMouseWheelMoveState = Math.sign(-e.deltaY);
        };
        const mouseMove = (e) => {
            this.currentMousePosition = {x: e.clientX, y: e.clientY};
        };
        const mouseButtonDown = (e) => {
            this.currentPressedMouseButtonState.add(glfwMouseButtonMapping[e.button]);
        };
        const mouseButtonUp = (e) => {
            this.currentPressedMouseButtonState.delete(glfwMouseButtonMapping[e.button]);
        };
        window.addEventListener("keydown", keyDown);
        window.addEventListener("keyup", keyUp);
        window.addEventListener("wheel", wheelMove);
        window.addEventListener("mousemove", mouseMove);
        window.addEventListener("mousedown", mouseButtonDown)
        window.addEventListener("mouseup", mouseButtonUp)

        this.exports.main();
        const next = (timestamp) => {
            if (this.quit) {
                this.ctx.clearRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
                window.removeEventListener("keydown", keyDown);
                this.#reset()
                return;
            }
            this.dt = (timestamp - this.previous)/1000.0;
            this.previous = timestamp;
            this.entryFunction();
            window.requestAnimationFrame(next);
        };
        window.requestAnimationFrame((timestamp) => {
            this.previous = timestamp;
            window.requestAnimationFrame(next);
        });
    }

    async start({ wasmPath, canvasId }) {
        let wasm = await WebAssembly.instantiateStreaming(fetch(wasmPath), {
            env: make_environment(this)
        });

        this.startExports( {
            exports: wasm.instance.exports,
            canvasId,
        })
    }


    WasmSetMainLoop(entry) {
        this.entryFunction = this.exports.__indirect_function_table.get(entry);
    }

    InitGraphics(width, height) {
        this.ctx.canvas.width = width;
        this.ctx.canvas.height = height; 

        // This makes 0,0 the origin
        this.projection_matrix = {
            a: 1,
            b: 0,
            c: 0,
            d: 1,
            e: this.ctx.canvas.width / 2.0,
            f: this.ctx.canvas.height / 2.0,
        }
    }

    InitAudio() {

    }

    SetProjection(matrix_ptr) {
        // NOTE: We proably shouldn't have a way for the game to set the projection since it depends on graphics backend
    }

    SetView(matrix_ptr) {
        const buffer = this.exports.memory.buffer;
        var transform = new Float32Array(buffer, matrix_ptr, 16);

        this.view_matrix = {
            a: transform[0],
            b: transform[1],
            c: transform[4],
            d: transform[5],
            e: transform[12],
            f: transform[13],
        }
    }

    LoadSoundFromFileWave(path_ptr) {
        const mem = this.exports.memory.buffer;
        const path = cstr_by_ptr(mem, path_ptr)

        const audio = new Audio(path);
        const sound_id = this.next_sound_id;
        this.next_sound_id += 1;

        this.sounds[sound_id] = audio;
        
        return sound_id;
    }

    LoadFont(font_path_ptr) {
        const mem = this.exports.memory.buffer;
        const font_path = cstr_by_ptr(mem, font_path_ptr)

        const font_id = this.next_font_id;
        this.next_font_id += 1;
        const font_face = new FontFace(`Font_${font_id}`, `url("${font_path}")`);
        document.fonts.add(font_face);
        font_face.load();
        this.fonts[font_id] = font_face;
        return font_id;
    }

    UnloadFont(font_id) {
        const font_face = this.fonts[font_id];
        document.fonts.delete(font_face);
        this.fonts.delete(font_face);
    }


    PlatformPrint(text_ptr) {
        const mem = this.exports.memory.buffer;
        const str = cstr_by_ptr(mem, text_ptr)
        console.log(str);
    }

    qsort(array_ptr, element_count, element_size, cmp_func_ptr) {

    }

    WindowShouldClose(){
        return false;
    }

    PlatformLoadFileBinary(filename_ptr, size_ptr) {
        const buffer = this.exports.memory.buffer;
        const filename = cstr_by_ptr(buffer, filename_ptr);
        
        const request = new XMLHttpRequest();
        request.open("GET", filename, false); // `false` makes the request synchronous
        request.overrideMimeType('text/plain; charset=x-user-defined');
        request.send(null);
        
        if (request.status != 200){
            var size_out = new Uint32Array(this.exports.memory.buffer, size_ptr, 1);
            size_out[0] = 0;
            return 0;
        }

        var binStr = request.responseText;
        var size = binStr.length;
        const allocatedMemory = this.exports.malloc(size);
        var bytes = new Uint8Array(this.exports.memory.buffer, allocatedMemory, size);
        
        for (var i = 0, len = binStr.length; i < len; ++i) {
            var c = binStr.charCodeAt(i);
            var byte = c & 0xff;  // byte at offset i

            bytes[i] = byte;
        }

        var size_out = new Uint32Array(this.exports.memory.buffer, size_ptr, 1);
        size_out[0] = size;

        return allocatedMemory;
    }

    PlatformUnloadFileBinary(data_ptr) {
        this.exports.free(data_ptr);
    }


    PlatformLoadFileText(filename_ptr) {
        const buffer = this.exports.memory.buffer;
        const filename = cstr_by_ptr(buffer, filename_ptr);
        
        const request = new XMLHttpRequest();
        request.open("GET", filename, false); // `false` makes the request synchronous
        request.overrideMimeType('text/plain; charset=x-user-defined');
        request.send(null);
        
        if (request.status != 200){
            return 0;
        }

        var binStr = request.responseText;
        var size = binStr.length;
        const allocatedMemory = this.exports.malloc(size + 1);
        var bytes = new Uint8Array(this.exports.memory.buffer, allocatedMemory, size + 1);
        
        for (var i = 0, len = binStr.length; i < len; ++i) {
            var c = binStr.charCodeAt(i);
            var byte = c & 0xff;  // byte at offset i
            
            bytes[i] = byte;
        }
        bytes[size] = 0;

        return allocatedMemory;
    }

    PlatformUnloadFileText(text_ptr) {
        this.exports.free(text_ptr);
    }

    LoadTextureFromImage(out_texture_ptr, image_ptr, has_filter, has_mipmaps) {
        const buffer = this.exports.memory.buffer;
        // Texture: id(32) width(32) height(32)
        // Image: data_ptr(32) width(32) height(32) components(32)
        var image = new Uint32Array(buffer, image_ptr, 4);
        const image_width = image[1];
        const image_height = image[2];
        const image_components = image[3];

        const imageBytes = new Uint8ClampedArray(buffer, image[0], image_width * image_height * image_components);
        const imageData = new ImageData(imageBytes, image_width, image_height);

        var canvas = document.createElement("canvas");
        canvas.width = image_width;
        canvas.height = image_height;
        var ctx = canvas.getContext("2d");
        ctx.putImageData(imageData, 0, 0);

        const js_image = new Image(image_width, image_height);
        js_image.src = canvas.toDataURL("image/png");

        this.images.push(js_image);

        var result = new Uint32Array(buffer, out_texture_ptr, 3);
        result[0] = this.images.indexOf(js_image);
        result[1] = image_width; // width
        result[2] = image_height; // height

        return result;
    }

    JsIsKeyDown(key_glfw) {
        return this.currentPressedKeyState.has(key_glfw);
    }

    JsIsMouseButtonDown(button_glfw) {
        return this.currentPressedMouseButtonState.has(button_glfw);
    }

    JsClearInput() {
        this.currentPressedKeyState.clear();
        this.currentPressedMouseButtonState.clear();
    }

    JsGetCanvasWidth() {
        return this.ctx.canvas.width;
    }

    JsGetCanvasHeight() {
        return this.ctx.canvas.height;
    }

    rand() {
        return Math.random() * 2147483647; // 0, RAND_MAX
    }

    sinf(radians) {
        return Math.sin(radians);
    }

    cosf(radians) {
        return Math.cos(radians);
    }

    atan2f(y, x) {
        return Math.atan2(y, x);
    }

    powf(num, exp) {
        return Math.pow(num, exp);
    }

    fmodf(num, mod) {
        return num % mod;
    }

    DrawRectTextured(transform_ptr, texture_ptr, texture_rect_ptr, color_ptr) { 
        // TODO: tint with color

        // Transform: 4x4 float 32
        // Texture: id(32) width(32) height(32)
        // Rect: x(32) y(32) width(32) height(32)
        const buffer = this.exports.memory.buffer;
        var transform = new Float32Array(buffer, transform_ptr, 16);
        var texture = new Uint32Array(buffer, texture_ptr, 3);
        var rect = new Uint32Array(buffer, texture_rect_ptr, 4);



        this.ctx.setTransform(this.projection_matrix);
        this.ctx.transform(this.view_matrix.a, this.view_matrix.b, this.view_matrix.c, this.view_matrix.d, this.view_matrix.e, this.view_matrix.f);
        this.ctx.transform(transform[0], transform[1], transform[4], transform[5], transform[12], transform[13]);


        const image = this.images[texture[0]];
        this.ctx.drawImage(image, rect[0], rect[1], rect[2], rect[3], -rect[2] / 2.0, -rect[3] / 2.0, rect[2], rect[3]);
    }

    // void DrawText(Mln::Font font, const char *str, Mln::Vector2 position, float scale, Mln::Color color, TextAlign alignment = TEXT_ALIGN_LEFT);
    DrawText(font_id, text_ptr, position_ptr, font_scale, color_ptr, alignment) {
        const buffer = this.exports.memory.buffer;
        const text = cstr_by_ptr(buffer, text_ptr);
        const [posX, posY] = new Float32Array(buffer, position_ptr, 2);
        
        const color = getColorFromMemory(buffer, color_ptr);
        this.ctx.fillStyle = color;

        const font_size = 48 * font_scale;
        this.ctx.font = `${font_size}px Font_${font_id}`;

        const text_size = this.ctx.measureText(text);
        const text_width = text_size.width;
        const text_height = text_size.height;

        var offsetX = 0;
        if (alignment == 1) { // ALIGN_CENTER
            offsetX = -text_width / 2.0;
        }
        else if (alignment == 2) { // ALIGN_RIGHT
            offsetX = -text_width;
        }


        this.ctx.setTransform(this.projection_matrix);
        this.ctx.transform(this.view_matrix.a, this.view_matrix.b, this.view_matrix.c, this.view_matrix.d, this.view_matrix.e, this.view_matrix.f);
        const lines = text.split('\n');
        for (var i = 0; i < lines.length; i++) {
            this.ctx.fillText(lines[i], posX + offsetX, posY + (i * font_size));
        }
    }

    PlatformGetTime() {
        return this.previous / 1000.0;
    }

    PlaySound(sound_id) {
        const audio = this.sounds[sound_id];
        audio.play();
    }

    ClearBackground(color_ptr) {
        const color = getColorFromMemory(this.exports.memory.buffer, color_ptr)
        this.ctx.fillStyle = color;
        this.ctx.setTransform(1, 0, 0, 1, 0, 0);
        this.ctx.fillRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
    }

}

const glfwKeyMapping = {
    "Space":          32,
    "Quote":          39,
    "Comma":          44,
    "Minus":          45,
    "Period":         46,
    "Slash":          47,
    "Digit0":         48,
    "Digit1":         49,
    "Digit2":         50,
    "Digit3":         51,
    "Digit4":         52,
    "Digit5":         53,
    "Digit6":         54,
    "Digit7":         55,
    "Digit8":         56,
    "Digit9":         57,
    "Semicolon":      59,
    "Equal":          61,
    "KeyA":           65,
    "KeyB":           66,
    "KeyC":           67,
    "KeyD":           68,
    "KeyE":           69,
    "KeyF":           70,
    "KeyG":           71,
    "KeyH":           72,
    "KeyI":           73,
    "KeyJ":           74,
    "KeyK":           75,
    "KeyL":           76,
    "KeyM":           77,
    "KeyN":           78,
    "KeyO":           79,
    "KeyP":           80,
    "KeyQ":           81,
    "KeyR":           82,
    "KeyS":           83,
    "KeyT":           84,
    "KeyU":           85,
    "KeyV":           86,
    "KeyW":           87,
    "KeyX":           88,
    "KeyY":           89,
    "KeyZ":           90,
    "BracketLeft":    91,
    "Backslash":      92,
    "BracketRight":   93,
    "Backquote":      96,
    //  GLFW_KEY_WORLD_1   161 /* non-US #1 */
    //  GLFW_KEY_WORLD_2   162 /* non-US #2 */
    "Escape":         256,
    "Enter":          257,
    "Tab":            258,
    "Backspace":      259,
    "Insert":         260,
    "Delete":         261,
    "ArrowRight":     262,
    "ArrowLeft":      263,
    "ArrowDown":      264,
    "ArrowUp":        265,
    "PageUp":         266,
    "PageDown":       267,
    "Home":           268,
    "End":            269,
    "CapsLock":       280,
    "ScrollLock":     281,
    "NumLock":        282,
    "PrintScreen":    283,
    "Pause":          284,
    "F1":             290,
    "F2":             291,
    "F3":             292,
    "F4":             293,
    "F5":             294,
    "F6":             295,
    "F7":             296,
    "F8":             297,
    "F9":             298,
    "F10":            299,
    "F11":            300,
    "F12":            301,
    "F13":            302,
    "F14":            303,
    "F15":            304,
    "F16":            305,
    "F17":            306,
    "F18":            307,
    "F19":            308,
    "F20":            309,
    "F21":            310,
    "F22":            311,
    "F23":            312,
    "F24":            313,
    "F25":            314,
    "NumPad0":        320,
    "NumPad1":        321,
    "NumPad2":        322,
    "NumPad3":        323,
    "NumPad4":        324,
    "NumPad5":        325,
    "NumPad6":        326,
    "NumPad7":        327,
    "NumPad8":        328,
    "NumPad9":        329,
    "NumpadDecimal":  330,
    "NumpadDivide":   331,
    "NumpadMultiply": 332,
    "NumpadSubtract": 333,
    "NumpadAdd":      334,
    "NumpadEnter":    335,
    "NumpadEqual":    336,
    "ShiftLeft":      340,
    "ControlLeft" :   341,
    "AltLeft":        342,
    "MetaLeft":       343,
    "ShiftRight":     344,
    "ControlRight":   345,
    "AltRight":       346,
    "MetaRight":      347,
    "ContextMenu":    348,
    //  GLFW_KEY_LAST   GLFW_KEY_MENU
}


const glfwMouseButtonMapping = [
    0, 2, 1, 3, 4, 5, 6, 7, 8
]

function cstrlen(mem, ptr) {
    let len = 0;
    while (mem[ptr] != 0) {
        len++;
        ptr++;
    }
    return len;
}

function cstr_by_ptr(mem_buffer, ptr) {
    const mem = new Uint8Array(mem_buffer);
    const len = cstrlen(mem, ptr);
    const bytes = new Uint8Array(mem_buffer, ptr, len);
    return new TextDecoder().decode(bytes);
}

function color_hex_unpacked(r, g, b, a) {
    r = parseInt(r * 255).toString(16).padStart(2, '0');
    g = parseInt(g * 255).toString(16).padStart(2, '0');
    b = parseInt(b * 255).toString(16).padStart(2, '0');
    a = parseInt(a * 255).toString(16).padStart(2, '0');
    return "#"+r+g+b+a;
}

function getColorFromMemory(buffer, color_ptr) {
    const [r, g, b, a] = new Float32Array(buffer, color_ptr, 4);
    return color_hex_unpacked(r, g, b, a);
}