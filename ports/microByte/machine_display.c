#include "py/runtime.h"
#include "py/mphal.h"
#include "drivers/display/display_HAL/display_HAL.h"
#include "modmachine.h"

const mp_obj_type_t machine_display_type;

typedef struct _machine_display_obj_t {
    mp_obj_base_t base;
    mp_int_t height;
    mp_int_t width;
    
} machine_display_obj_t;


mp_obj_t mp_display_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    machine_display_obj_t *self = m_new_obj_with_finaliser(machine_display_obj_t);
    self->base.type = &machine_display_type;
    self->height = 240;
    self->width = 240;
    //display_HAL_init();
    return MP_OBJ_FROM_PTR(self);
}

mp_obj_t machine_display_clear(){
    printf("Clear\r\n");
    display_HAL_clear();
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_display_clear_obj, 1, 2, machine_display_clear);

STATIC mp_obj_t machine_display_flush(size_t n_args, const mp_obj_t *args){
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_display_flush_obj, 1, 2, machine_display_flush);


//uGame releated functions

mp_obj_t machine_display_block(size_t n_args, const mp_obj_t *args){
    printf("fooo\r\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_display_block_obj, 5, 5, machine_display_block);

mp_obj_t machine_display_exit(size_t n_args, const mp_obj_t *args){
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_display_exit_obj, 1, 2, machine_display_exit);

mp_obj_t machine_display_enter(size_t n_args, const mp_obj_t *args){
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_display_enter_obj, 1, 2, machine_display_enter);

STATIC const mp_rom_map_elem_t machine_display_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&machine_display_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&machine_display_flush_obj) },
    { MP_ROM_QSTR(MP_QSTR_block), MP_ROM_PTR(&machine_display_block_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&machine_display_exit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&machine_display_enter_obj) },

    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_INT(240) }, //TODO: Somenthing temporary
    { MP_ROM_QSTR(MP_QSTR_height), MP_ROM_INT(240) }, //TODO: Somenthing temporary

};

STATIC MP_DEFINE_CONST_DICT(machine_display_locals_dict, machine_display_locals_dict_table);

const mp_obj_type_t machine_display_type = {
    { &mp_type_type },
    .name = MP_QSTR_display,
    //.print = esp32_pwm_print,
    .make_new = mp_display_make_new,
    .locals_dict = (mp_obj_dict_t *)&machine_display_locals_dict,
};