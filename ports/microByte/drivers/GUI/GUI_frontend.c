/*********************
 *      LIBRARIES
 *********************/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl/lvgl.h"

#include "../input/input_HAL/input_HAL.h"

/**********************
 *  Global Variable
 **********************/
bool ctrl_available = true;
bool tab_move_available = false;

uint32_t btn_left_time = 0;
uint32_t btn_right_time = 0;
uint32_t btn_up_time = 0;
uint32_t btn_down_time = 0;
uint32_t btn_a_time = 0;
uint32_t btn_b_time = 0;
uint32_t btn_menu_time = 0;

/**********************
 *  LVGL groups and objects
 **********************/

// Interactive related things
static lv_group_t * group_interact; 
static lv_indev_t * kb_indev;

//Static
static bool user_input_task(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void GUI_frontend_menu(){
    //TODO: Add tittle
    //TODO: Add REPL button
    
    

    /*lv_obj_t * list_py_files = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(list_py_files, 210, 130);
    lv_obj_align(list_py_files, NULL, LV_ALIGN_CENTER, 0, 10);
    lv_group_add_obj(group_interact, list_py_files);*/


    //TODO: Get .py files
    //TODO: Show .py files on the list
}


void GUI_frontend_main(){

    group_interact = lv_group_create();

    lv_indev_drv_t kb_drv;
    lv_indev_drv_init(&kb_drv);
    kb_drv.type = LV_INDEV_TYPE_KEYPAD;
    kb_drv.read_cb = user_input_task;
    kb_indev = lv_indev_drv_register(&kb_drv);
    lv_indev_set_group(kb_indev, group_interact);

    lv_obj_t * repl_btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_align(repl_btn, NULL, LV_ALIGN_CENTER, 0, 10);
    lv_group_add_obj(group_interact, repl_btn);


    GUI_frontend_menu();
}



void GUI_frontend_ctrl_available(bool _ctrl_available){

}

static bool user_input_task(lv_indev_drv_t * indev_drv, lv_indev_data_t * data){
    if(ctrl_available){
       /* uint16_t inputs_value =  input_read();

        if(!((inputs_value >> 0) & 0x01)){
            //Down BTN
            uint32_t actual_time= xTaskGetTickCount()/portTICK_PERIOD_MS;

            if((actual_time-btn_down_time)>2){
            data->state = LV_INDEV_STATE_PR;
            data->key = LV_KEY_PREV;

            // Save the actual time to calculate the bounce time.
            btn_down_time = actual_time;
            }
        }*/

    }
    return true;
}


