/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Radomir Dopieralski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mperrno.h"
#include "extmod/machine_spi.h"
#include "../drivers/display/st7789/st7789.h"
#include "../drivers/display/display_HAL/display_HAL.h"

#define TRANSPARENT (0x1FF8)

typedef struct {
    mp_obj_base_t base;
    uint8_t *map;
    uint8_t *graphic;
    uint8_t *palette;
    int16_t x, y;
    uint8_t width, height;
    uint8_t frame;
    uint8_t rotation;
} layer_obj_t;

typedef struct {
    mp_obj_base_t base;
    uint8_t *chars;
    uint8_t *font;
    uint8_t *palette;
    int16_t x, y;
    uint8_t width, height;
} text_obj_t;


inline
uint16_t get_text_pixel(text_obj_t *text, int16_t x, uint16_t y) {

    // Shift by the text's position offset.
    x -= text->x;
    y -= text->y;

    // Bounds check.
    if ((x < 0) || (x >= text->width << 3) ||
            (y < 0) || (y >= text->height << 3)) {
        return TRANSPARENT;
    }

    // Get the tile from the grid location or from sprite frame.
    uint8_t tx = x >> 3;
    uint8_t ty = y >> 3;
    uint8_t c = text->chars[ty * text->width + tx];
    uint8_t color_offset = 0;
    if (c & 0x80) {
        color_offset = 4;
    }
    c &= 0x7f;
    if (!c) {
        return TRANSPARENT;
    }

    // Get the position within the char.
    x &= 0x07;
    y &= 0x07;

    // Get the value of the pixel.
    uint8_t pixel = text->font[(c << 4) + (y << 1) + (x >> 2)];
    pixel = ((pixel >> ((x & 0x03) << 1)) & 0x03) + color_offset;

    // Convert to 16-bit color using the palette.
    return text->palette[pixel << 1] | text->palette[(pixel << 1) + 1] << 8;
}

inline
uint16_t get_layer_pixel(layer_obj_t *layer, int16_t x, uint16_t y) {

    // Shift by the layer's position offset.
    x -= layer->x;
    y -= layer->y;

    // Bounds check.
    if ((x < 0) || (x >= layer->width << 4) ||
            (y < 0) || (y >= layer->height << 4)) {
        return TRANSPARENT;
    }

    // Get the tile from the grid location or from sprite frame.
    uint8_t frame = layer->frame;
    if (layer->map) {
        uint8_t tx = x >> 4;
        uint8_t ty = y >> 4;

        frame = layer->map[(ty * layer->width + tx) >> 1];
        if (tx & 0x01) {
            frame &= 0x0f;
        } else {
            frame >>= 4;
        }
    }

    // Get the position within the tile.
    x &= 0x0f;
    y &= 0x0f;

    // Rotate the image.
    uint8_t ty = y; // Temporary variable for swapping.
    switch (layer->rotation) {
        case 1: // 90 degrees clockwise
            y = 15 - x;
            x = ty;
            break;
        case 2: // 180 degrees
            y = 15 - ty;
            x = 15 - x;
            break;
        case 3: // 90 degrees counter-clockwise
            y = x;
            x = 15 - ty;
            break;
        case 4: // 0 degrees, mirrored
            x = 15 - x;
            break;
        case 5: // 90 degrees clockwise, mirrored
            y = x;
            x = ty;
            break;
        case 6: // 180 degrees, mirrored
            y = 15 - ty;
            break;
        case 7: // 90 degrees counter-clockwise, mirrored
            y = 15 - x;
            x = 15 - ty;
            break;
        default: // 0 degrees
            break;
    }

    // Get the value of the pixel.
    uint8_t pixel = layer->graphic[(frame << 7) + (y << 3) + (x >> 1)];
    if (x & 0x01) {
        pixel &= 0x0f;
    } else {
        pixel >>= 4;
    }

    // Convert to 16-bit color using the palette.
    //uint16_t color = layer->palette[pixel << 1] | layer->palette[(pixel << 1) + 1] << 8 ;
    uint8_t blue = (layer->palette[pixel << 1] & 0b11111000);
    uint8_t red = (layer->palette[(pixel << 1) + 1] & 0b11111000);
    uint8_t green = (layer->palette[(pixel << 1) + 1] & 0b000000111) << 3  | (layer->palette[pixel << 1] & 0b00000111);
    uint16_t out = green << 8;
    out = out + ((red) << 3);
    out = out + ((blue) >> 3);

    return out;
}


STATIC mp_obj_t text_make_new(const mp_obj_type_t *type, size_t n_args,
        size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 5, 5, false);

    text_obj_t *self = m_new_obj(text_obj_t);
    self->base.type = type;

    self->width = mp_obj_get_int(args[0]);
    self->height = mp_obj_get_int(args[1]);
    self->x = 0;
    self->y = 0;

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    self->font = bufinfo.buf;
    if (bufinfo.len != 2048) {
        mp_raise_ValueError(MP_ERROR_TEXT("font must be 2048 bytes long"));
    }

    mp_get_buffer_raise(args[3], &bufinfo, MP_BUFFER_READ);
    self->palette = bufinfo.buf;
    if (bufinfo.len != 32) {
        mp_raise_ValueError(MP_ERROR_TEXT("palette must be 32 bytes long"));
    }

    mp_get_buffer_raise(args[4], &bufinfo, MP_BUFFER_READ);
    self->chars = bufinfo.buf;
    if (bufinfo.len < self->width * self->height) {
        mp_raise_ValueError(MP_ERROR_TEXT("chars buffer too small"));
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t text_move(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    text_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->x = mp_obj_get_int(x_in);
    self->y = mp_obj_get_int(y_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(text_move_obj, text_move);


#if !MICROPY_ENABLE_DYNRUNTIME
STATIC const mp_rom_map_elem_t text_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_move), MP_ROM_PTR(&text_move_obj) },
};
STATIC MP_DEFINE_CONST_DICT(text_locals_dict, text_locals_dict_table);

const mp_obj_type_t mp_type_text = {
    { &mp_type_type },
    .name = MP_QSTR_Text,
    .make_new = text_make_new,
    .locals_dict = (mp_obj_dict_t*)&text_locals_dict,
};
#else
extern mp_obj_type_t mp_type_text;
#endif


STATIC mp_obj_t layer_make_new(const mp_obj_type_t *type, size_t n_args,
        size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 4, 5, false);

    layer_obj_t *self = m_new_obj(layer_obj_t);
    self->base.type = type;

    self->width = mp_obj_get_int(args[0]);
    self->height = mp_obj_get_int(args[1]);
    self->x = 0;
    self->y = 0;
    self->frame = 0;
    self->rotation = false;

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    self->graphic = bufinfo.buf;
    if (bufinfo.len != 2048) {
        mp_raise_ValueError(MP_ERROR_TEXT("graphic must be 2048 bytes long"));
    }

    mp_get_buffer_raise(args[3], &bufinfo, MP_BUFFER_READ);
    self->palette = bufinfo.buf;
    if (bufinfo.len != 32) {
        mp_raise_ValueError(MP_ERROR_TEXT("palette must be 32 bytes long"));
    }

    if (n_args > 4) {
        mp_get_buffer_raise(args[4], &bufinfo, MP_BUFFER_READ);
        self->map = bufinfo.buf;
        if (bufinfo.len < (self->width * self->height) / 2) {
            mp_raise_ValueError(MP_ERROR_TEXT("map buffer too small"));
        }
    } else {
        self-> map = NULL;
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t layer_move(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in) {
    layer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->x = mp_obj_get_int(x_in);
    self->y = mp_obj_get_int(y_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(layer_move_obj, layer_move);


STATIC mp_obj_t layer_frame(mp_obj_t self_in, mp_obj_t frame_in,
                            mp_obj_t rotation_in) {
    layer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->frame = mp_obj_get_int(frame_in);
    self->rotation = mp_obj_get_int(rotation_in);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(layer_frame_obj, layer_frame);


#if !MICROPY_ENABLE_DYNRUNTIME
STATIC const mp_rom_map_elem_t layer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_move), MP_ROM_PTR(&layer_move_obj) },
    { MP_ROM_QSTR(MP_QSTR_frame), MP_ROM_PTR(&layer_frame_obj) },
};
STATIC MP_DEFINE_CONST_DICT(layer_locals_dict, layer_locals_dict_table);

const mp_obj_type_t mp_type_layer = {
    { &mp_type_type },
    .name = MP_QSTR_Layer,
    .make_new = layer_make_new,
    .locals_dict = (mp_obj_dict_t*)&layer_locals_dict,
};
#else
extern mp_obj_type_t mp_type_layer;
#endif

inline
void render_stage(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
        mp_obj_t *layers, size_t layers_size,
        uint16_t *buffer, size_t buffer_size,
        /*mp_obj_t spi,*/ uint8_t scale) {

    // TODO(deshipu): Do a collision check of each layer with the
    // rectangle, and only process the layers that overlap with it.

    //mp_obj_base_t *s = (mp_obj_base_t*)MP_OBJ_TO_PTR(spi);
    //mp_machine_spi_p_t *spi_p = (mp_machine_spi_p_t*)s->type->protocol;
    size_t index = 0;
    for (uint16_t y = 0; y < 240; ++y) {
            for (uint16_t x = 0; x < 240; ++x) {
                uint16_t c = TRANSPARENT;
                for (size_t layer = 0; layer < layers_size; ++layer) {
                    layer_obj_t *obj = MP_OBJ_TO_PTR(layers[layer]);
                    if (obj->base.type == &mp_type_layer) {
                        c = get_layer_pixel(obj, x, y);
                    } else if (obj->base.type == &mp_type_text) {
                        c = get_text_pixel((text_obj_t *)obj, x, y);
                    }
                    if (c != TRANSPARENT) {
                        break;
                    }
                }
                    buffer[index] = c ;
                    index += 1;
                    // The buffer is full, send it.
                    if (index >= buffer_size) {
                        /*spi_p->transfer(s, buffer_size * 2,
                                        (const uint8_t*)buffer, NULL);*/
                       display_HAL_set_windows(x0,x1,(y-19),y);
                       //display_HAL_set_windows(x0,x1,y0,y1);
                        //vTaskDelay(1500 / portTICK_RATE_MS);
                        xQueueSend(vidQueue, &buffer, 0);
                        index = 0;

                    }
                
            }
    }
    // Send the remaining data.
    if (index) {
        //spi_p->transfer(s, index * 2, (const uint8_t*)buffer, NULL);
        /*printf("Index %i x0 %i y0 %i x1 %i y1 %i\r\n",index,x0,y0,x1,y1);
        for(int i=0;i<512;i++){
            buf_aux[i]=0xffff;
        }
        for(int i=y0;i<y1-y0;i++){
            display_HAL_set_windows(x0,x1,i,i+1);
            display_HAL_print(buffer, index*2);
        }*/

        display_HAL_set_windows(x0,x1,y0,y1);
        xQueueSend(vidQueue, &buffer, 0);
        
    }
}


STATIC mp_obj_t stage_render(size_t n_args, const mp_obj_t *args) {
    uint16_t x0 = mp_obj_get_int(args[0]);
    uint16_t y0 = mp_obj_get_int(args[1]);
    uint16_t x1 = mp_obj_get_int(args[2]);
    uint16_t y1 = mp_obj_get_int(args[3]);

    size_t layers_size = 0;
    mp_obj_t *layers;
#if !MICROPY_ENABLE_DYNRUNTIME
    mp_obj_get_array(args[4], &layers_size, &layers);
#else
    mp_fun_table.mp_obj_get_array(args[4], &layers_size, &layers);
#endif

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[5], &bufinfo, MP_BUFFER_WRITE);
    uint16_t *buffer = bufinfo.buf;
    size_t buffer_size = bufinfo.len / 2; // 16-bit indexing

    /*mp_obj_t spi = args[6];
    // TODO: Make sure it's an SPI object.
    const mp_obj_type_t *type = mp_obj_get_type(spi);
    if (type->protocol == NULL) {
         mp_raise_ValueError(MP_ERROR_TEXT("SPI protocol required"));
    }*/
    uint8_t scale = 1;
    if (n_args >= 8) {
        scale = mp_obj_get_int(args[7]);
    }

    render_stage(x0, y0, x1, y1, layers, layers_size,
                 buffer, buffer_size, /*spi,*/ scale);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(stage_render_obj, 7, 8, stage_render);


#if !MICROPY_ENABLE_DYNRUNTIME
STATIC const mp_rom_map_elem_t stage_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR__stage) },
    { MP_ROM_QSTR(MP_QSTR_Layer), MP_ROM_PTR(&mp_type_layer) },
    { MP_ROM_QSTR(MP_QSTR_Text), MP_ROM_PTR(&mp_type_text) },
    { MP_ROM_QSTR(MP_QSTR_render), MP_ROM_PTR(&stage_render_obj) },
};

STATIC MP_DEFINE_CONST_DICT(stage_module_globals, stage_module_globals_table);

const mp_obj_module_t mp_module__stage = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&stage_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR__stage, mp_module__stage, MODULE_STAGE_ENABLED);
#endif
