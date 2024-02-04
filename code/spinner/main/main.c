#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
//#include "hal/spi_types.h"
#include "esp_spi_flash.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "rotary_encoder.h"

#define LED_NUM  45
#define BUF_SZ  ((3 + LED_NUM) * sizeof(uint32_t))  // 1 uint32 for start, 2 uint32 for end
#define RGBL(r,g,b,lum) ((((g) & 0xFF)<<24) | (((b) & 0xFF)<<16) | (((r) & 0xFF)<<8) | (((lum) & 0xFF) | 0xE0)<<0  ) 
//#define HSPI_HOST   SPI2_HOST
//#define VSPI_HOST   SPI3_HOST
#define LED_HOST HSPI_HOST
    // host = HSPIC_HOST;
    // host = VSPIC_HOST;
#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
//#define PIN_NUM_MOSI 23
//#define PIN_NUM_CLK  18
// #define PIN_NUM_MOSI 30
// #define PIN_NUM_CLK  24
#define PIN_NUM_CS   -1

static int init_led_spi(spi_device_handle_t *spi);


static int init_led_spi(spi_device_handle_t *spi)
{
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=BUF_SZ
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=SPI_MASTER_FREQ_10M, //      SPI_MASTER_FREQ_40M, //     20*1000*1000,
        .mode=0, // 3, SPICOMMON_BUSFLAG_DUAL   SPI_TRANS_MODE_DIO                                //SPI mode 0
        .spics_io_num=-1,                       //CS pin
        .queue_size=1,                          //We want to be able to queue 7 transactions at a time
        //.pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };

    ret=spi_bus_initialize(LED_HOST, &buscfg, SPI_DMA_CH_AUTO);
    printf("spi_bus_init, ret:  %d\n", ret );
    ret=spi_bus_add_device(LED_HOST, &devcfg, spi);
    printf("spi_bus_add_device, ret:  %d\n", ret );
	return ESP_OK;
}

void update_buf(uint32_t *buf, int p)
    {
    #define LUM 0xef
    uint8_t r, g, b, i;
    for(i=0; i<LED_NUM; i++)
        {
        // r = (p-i) * 4;
        // g = (p-i+15) * 4;
        // b = (p-i+30) * 4;
        r = 0;
        g = 0;
        b = 0;
        if( i == p)
            {
            if( p < 15 )
                r = 0x55;
            else if( p < 30 )
                g = 0x55;
            else
                b = 0x55;
            }
        buf[i+1] = RGBL(r,g,b,LUM);
        }
    }

void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spi;
    uint32_t *buf;
    int i;
    size_t size = BUF_SZ;
    spi_transaction_t t = {0};
    rotary_encoder_info_t  rot_info;
    gpio_num_t pin_a = GPIO_NUM_25; // PIN_A;
    gpio_num_t pin_b = GPIO_NUM_26; //  PIN_B;
    rotary_encoder_state_t rot_state;

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ret = rotary_encoder_init( &rot_info, pin_a, pin_b );


    ret = init_led_spi(&spi);
	buf = (uint32_t*)heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    buf[0] = 0x00000000;
    buf[LED_NUM+1] = 0xFFFFFFFF;
    buf[LED_NUM+2] = 0xFFFFFFFF;
    t.tx_buffer = buf;
    t.length = BUF_SZ * 8;

// esp_err_t spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc);
    printf("Hello, Tester!\n");
    i = 0;
    while( 1 )
        {
        // i = i%50;
        ret = rotary_encoder_get_state( &rot_info, &rot_state );
        ret = rotary_encoder_enable_half_steps( &rot_info, false );
        i = rot_state.position;
        printf("position: %d\n", i);
        update_buf(buf, i % LED_NUM );
        ret = spi_device_transmit( spi, &t );
        // printf("spi_device_transmit, ret:  %d\n", ret );
        vTaskDelay(2);
        }
}

