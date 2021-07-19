#include "stdlib.h"

#include "lvgl/lvgl.h"

#include "../display/display_HAL/display_HAL.h"
#include "../input/input_HAL/input_HAL.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "GUI_frontend.h"


/*********************
 *      DEFINES
 *********************/
#define BUF_SIZE 240*20
#define LV_TICK_PERIOD_MS 10

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
static lv_disp_drv_t disp_drv;
static void GUI_task(void *arg);

TaskHandle_t GUI_task_handler;
bool GUI_task_running = false;

/**********************
*   GLOBAL FUNCTIONS
**********************/

void GUI_init(){
    
    //Initialize back control thread
    xTaskCreatePinnedToCore(GUI_task, "Graphical User Interface", 1024*6, NULL, 1, &GUI_task_handler, 0);
    GUI_task_running = true;
}

bool GUI_pause(bool status){
    if(GUI_task_running && !status) vTaskSuspend(GUI_task_handler);
    else if(!GUI_task_running && status) vTaskResume(&GUI_task_handler);
    else{
        printf("Error changing GUI task state\r\n");
        return false;
    }
    return true;
}

void GUI_pause_controls(bool status){

}

void GUI_clean(){

}

void GUI_change_move(){

}


/**********************
*   STATIC FUNCTIONS
**********************/

static void GUI_task(void *arg){
    //it's suppose that the display is already initialize

    lv_init();

    //Display buffer initialization
    static  lv_color_t * buf1[BUF_SIZE];
    static  lv_color_t * buf2[BUF_SIZE];
    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = BUF_SIZE; 
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    // Initialize LVGL display and attach the flush function
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = display_HAL_flush;
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 240;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Create timer to handle LVGL system tick
    const esp_timer_create_args_t periodic_timer_args = {
            .callback = &lv_tick_task,
            .name = "periodic_gui"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    //On ESP32 it's better to create a periodic task instead of esp_register_freertos_tick_hook
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000)); // LV_TICK_PERIOD_MS expressed as microseconds

    
    //GUI_frontend_main();
    while(1){
        lv_task_handler();
    }
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}
