/*********************
 *      INCLUDES
 *********************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../st7789/st7789.h"
#include "display_HAL.h"
#include "../../HW_config/hw_config.h"

/*********************
 *      DEFINES
 *********************/
#define LINE_BUFFERS (2)
#define LINE_COUNT   (20)


uint16_t *line[LINE_BUFFERS];

extern uint16_t myPalette[];

/**********************
*      VARIABLES
**********************/
st7789_driver_t display = {
		.pin_reset = HSPI_RST,
		.pin_dc = HSPI_DC,
		.pin_mosi = HSPI_MOSI,
		.pin_sclk = HSPI_CLK,
		.spi_host = HSPI_HOST,
		.dma_chan = 1,
		.display_width = 240,
		.display_height = 240,
		.buffer_size = 240*20, // 2 buffers with 20 lines
	};

static const char *TAG = "Display_HAL";

/**********************
*  STATIC PROTOTYPES
**********************/
static void task(void *arg);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

// Display HAL basic functions.

bool display_HAL_init(void){
    vidQueue = xQueueCreate(7, sizeof(uint16_t *));
    xTaskCreatePinnedToCore(&task, "videoTask", 1024 * 4, NULL, 1, NULL, 1);
    return ST7789_init(&display);
}

void display_HAL_clear(){
    ST7789_fill_area(&display, WHITE, 0, 0, display.display_width, display.display_height);
}

// Boot Screen Functions
uint16_t * display_HAL_get_buffer(){
    return display.current_buffer;
}

size_t display_HAL_get_buffer_size(){
    return display.buffer_size;
}

void display_HAL_boot_frame(uint16_t * buffer){
    // The boot animation to the buffer
    display.current_buffer = buffer;

    //Send to the driver layer and change the buffer
    ST7789_swap_buffers(&display);
}

void display_HAL_change_endian(){
    ST7789_set_endian(&display);
}

// LVGL library releated functions
void display_HAL_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map){

    uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

    //Set the area to print on the screen
    ST7789_set_window(&display,area->x1,area->y1,area->x2 ,area->y2);

    //Save the buffer data and the size of the data to send
    display.current_buffer = (void *)color_map;
    display.buffer_size = size;

    //Send it
    //ST7789_write_pixels(&display, display.current_buffer, display.buffer_size);
    ST7789_swap_buffers(&display);

    //Tell to LVGL that is ready to send another frame
    lv_disp_flush_ready(drv);
}

void display_HAL_set_windows(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1){
    //ST7789_set_window(&display, x0, y0, (x1-x0) - 1, (y1-y0)- 1);
    ST7789_set_window(&display,x0,y0,x1,y1);
}

void display_HAL_print(uint16_t *buffer, size_t size){
    // The boot animation to the buffer
   // display.current_buffer = buffer;

    //memcpy(display.current_buffer,buffer,size);
    display.buffer_size = size;
  
    
    //Send to the driver layer and change the buffer
    ST7789_swap_buffers(&display);
    //ST7789_write_pixels(&display, buffer, size);
}

static void task(void *arg){
    uint16_t *param;

    while(1){
        xQueuePeek(vidQueue, &param, portMAX_DELAY);
  
        memcpy(display.current_buffer,param,240*20);
        display_HAL_print(param,240*20);

        xQueueReceive(vidQueue, &param, portMAX_DELAY);
    }
}



/**********************
 *   STATIC FUNCTIONS
 **********************/
