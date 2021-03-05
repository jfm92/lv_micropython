/*********************
*      INCLUDES
*********************/
#include "stdbool.h"

#include "esp_log.h"
#include "driver/i2c.h"

//#include "system_configuration.h"

/*********************
*      DEFINES
*********************/

#define CONFIG_REG 0x06
#define READ_REG 0x00 
#define I2C_dev_address 0x20

/**********************
*  STATIC PROTOTYPES
**********************/

static const char *TAG = "TCA9555";

static int8_t TCA955_write(uint8_t I2C_bus, uint8_t data, size_t size){

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( I2C_dev_address  << 1 ) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write(cmd, &data, size, 0x1); // Using &data will give a warning a compilation time, but is necessary to avoid I2C invalid address at running time
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if(ret != ESP_OK){
        ESP_LOGE(TAG, "Write error err = %d",ret );
        return -1;
    }
    
    return 1;

}

static int8_t TCA955_read(uint8_t I2C_bus, uint8_t *data, size_t size){

   if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_dev_address << 1) | I2C_MASTER_READ, 0x1);
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, 0x0);
    }
    i2c_master_read_byte(cmd, data + size - 1, 0x1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_bus, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret; 
}

/**********************
*   GLOBAL FUNCTIONS
**********************/

bool TCA955_init(void){
    //Init I2C bus as master
    esp_err_t ret;

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 21;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = 22;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400*1000 ;

    i2c_param_config(0, &conf);

    ret =  i2c_driver_install(0, conf.mode,0,0, 0);
    
    if(ret != ESP_OK){
        ESP_LOGE(TAG, "I2C driver initialization fail");
        return false;
    }

    //Check if the device is connected
    if(TCA955_write(0,0x00,1) == -1){
        ESP_LOGE(TAG,"TCA9555 not detected");
        return false;
    }

    printf("TCA9555 detected\r\n");

    return true;
}

int16_t TCA9555_readInputs(void){
    uint8_t data[2] = {0xFF,0xFF};

    TCA955_write(0, READ_REG, 1);
    TCA955_read(0, data, 2);

    uint16_t data_out = ((uint16_t)data[1] << 8) | data[0];

    return data_out;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

