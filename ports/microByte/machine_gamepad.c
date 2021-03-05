#include "py/runtime.h"
#include "py/mphal.h"
#include "drivers/input/input_HAL/input_HAL.h"
#include "modmachine.h"

typedef enum {
    BTN_UP = 0,
    BTN_DOWN = 1,
    BTN_LEFT = 2,
    BTN_RIGHT = 3,
    BTN_A = 4,
    BTN_B = 5,
    BTN_X = 6,
    BTN_Y = 7,
    BTN_R = 8,
    BTN_L = 9,
    BTN_START = 10,
    BTN_SELECT = 11,
    BTN_MENU = 12,
}BTN_num_t;


typedef struct _machine_gamepad_obj_t {
    mp_obj_base_t base;
    BTN_num_t id;
} machine_gamepad_obj_t;

STATIC const machine_gamepad_obj_t machine_btn_obj[] = {
    {{&machine_gamepad_type}, BTN_UP},
    {{&machine_gamepad_type}, BTN_DOWN},
    {{&machine_gamepad_type}, BTN_LEFT},
    {{&machine_gamepad_type}, BTN_RIGHT},
    {{&machine_gamepad_type}, BTN_A},
    {{&machine_gamepad_type}, BTN_B},
    {{&machine_gamepad_type}, BTN_X},
    {{&machine_gamepad_type}, BTN_Y},
    {{&machine_gamepad_type}, BTN_R},
    {{&machine_gamepad_type}, BTN_L},
    {{&machine_gamepad_type}, BTN_START},
    {{&machine_gamepad_type}, BTN_SELECT},
    {{&machine_gamepad_type}, BTN_MENU},
};

mp_obj_t mp_btn_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);
    const machine_gamepad_obj_t *self = NULL;
    int id = mp_obj_get_int(args[0]);

    if(id>=0 && id < MP_ARRAY_SIZE(machine_btn_obj)){
        self = (machine_gamepad_obj_t*)&machine_btn_obj[id];
    }
    else{
         mp_raise_ValueError(MP_ERROR_TEXT("Invalid ID"));
    }
    
    return MP_OBJ_FROM_PTR(self);
}

//TODO: REmove and initialize on the main loop
STATIC mp_obj_t machine_gamepad_init(mp_obj_t self_in){
    //TODO: Check if it's already initialize
    input_init();
    printf("hola\r\n");
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_1(machine_gamepad_init_obj, machine_gamepad_init);

mp_obj_t machine_gamepad_pressed(size_t n_args, const mp_obj_t *args){
    machine_gamepad_obj_t *aux = args[0];
    uint16_t btn_status = input_read();
    if(!((btn_status >> aux->id) & 0x01)){
        return mp_obj_new_bool(true);
    }
    
    return mp_obj_new_bool(false);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_gamepad_pressed_obj, 1, 2, machine_gamepad_pressed);

//TODO: Improve ID buttons
mp_obj_t machine_gamepad_help(){
    printf("Buttons ID:\r\n");
    printf("- Button Down: 0\r\n");
    printf("- Button Left: 1\r\n");
    printf("- Button Up: 2\r\n");
    printf("- Button Right: 3\r\n");
    printf("- Button Down: 0\r\n");
    return mp_const_none;
}

MP_DEFINE_CONST_FUN_OBJ_0(machine_gamepad_help_obj, machine_gamepad_help);

STATIC const mp_rom_map_elem_t machine_gamepad_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_gamepad_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_pressed), MP_ROM_PTR(&machine_gamepad_pressed_obj) },
    { MP_ROM_QSTR(MP_QSTR_help), MP_ROM_PTR(&machine_gamepad_help_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_gamepad_locals_dict, machine_gamepad_locals_dict_table);



const mp_obj_type_t machine_gamepad_type = {
    { &mp_type_type },
    .name = MP_QSTR_gamepad,
    //.print = esp32_pwm_print,
    .make_new = mp_btn_make_new,
    .locals_dict = (mp_obj_dict_t *)&machine_gamepad_locals_dict,
};