#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
//#include "hal/spi_types.h"
#include "esp_spi_flash.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
// #include "driver/pulse_cnt.h"  This is for newer framework... 5+ ??

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
#define PIN_A GPIO_NUM_25
#define PIN_B GPIO_NUM_26

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;


static rgb_t g_buf[360][15] = {
0
};

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
    #define LUM 0xe5
    uint8_t r, g, b, i, l, s;
    #ifdef WORKEDDD
        memcpy( &buf[1],    &g_buf[p][0], sizeof(uint32_t)*15 );
        memcpy( &buf[1+15], &g_buf[(p+120) % 360][0], sizeof(uint32_t)*15 );
        memcpy( &buf[1+30], &g_buf[(p+240)%360][0], sizeof(uint32_t)*15 );
    #endif
    s = 0/*  */;
    for(i=0; i<LED_NUM; i++)
        {
        if( i >=0 && i < 15 )
            {
            s = p;
            }
        if( i >=15 && i < 30 )
            {
            s = (p+120) % 360;
            }
        if( i >=30 && i < 45 )
            {
            s = (p+240) % 360;
            }
        l = i % 15; 
        r = g_buf[s][l].r;
        g = g_buf[s][l].g;
        b = g_buf[s][l].b;
        buf[i+1] = RGBL(r,g,b,LUM);
        }
    }


#ifdef USE_THE_NEWER_PCNT_DRIVER
void setup_encoder(pcnt_unit_handle_t *pcnt_unit)
    {
    ESP_LOGI(TAG, "install pcnt unit");
    pcnt_unit_config_t unit_config = {
        .high_limit = 360,
        .low_limit = 0,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(*pcnt_unit, &filter_config));

    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = PIN_A,
        .level_gpio_num = PIN_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(*pcnt_unit, &chan_a_config, &pcnt_chan_a));
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = PIN_B,
        .level_gpio_num = PIN_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(*pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
}
#endif



rotary_encoder_t* setup_encoder()
{
    rotary_encoder_t* encoder = NULL;
    uint32_t pcnt_unit = 0;
    rotary_encoder_config_t config = ROTARY_ENCODER_DEFAULT_CONFIG((rotary_encoder_dev_t)pcnt_unit, PIN_A, PIN_B);
    ESP_ERROR_CHECK(rotary_encoder_new_ec11(&config, &encoder));
    // Filter out glitch (1us)
    ESP_ERROR_CHECK(encoder->set_glitch_filter(encoder, 0));
    // Start encoder
    ESP_ERROR_CHECK(encoder->start(encoder));
    return encoder;
}


void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t spi;
    uint32_t *buf;
    int i;
    // pcnt_unit_handle_t pcnt_unit;
    rotary_encoder_t *encoder;
    size_t size = BUF_SZ;
    spi_transaction_t t = {0};

    // setup_encoder(&pcnt_unit);
    encoder = setup_encoder();
    ret = init_led_spi(&spi);
	buf = (uint32_t*)heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    buf[0] = 0x00000000;
    buf[LED_NUM+1] = 0xFFFFFFFF;
    buf[LED_NUM+2] = 0xFFFFFFFF;
    t.tx_buffer = buf;
    t.length = BUF_SZ * 8;


    for( i = 0; i < 360; i++ )
    {
    for( int j = 0; j < i%15; j++ )
        {
        g_buf[i][j].r = 0x55;
        g_buf[i][j].g = 0x55;
        g_buf[i][j].b = 0x00;
        }
    }


// esp_err_t spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc);
    printf("Hello, Tester!\n");
    i = 0;
    while( 1 )
        {
        i = encoder->get_counter_value(encoder);
        update_buf(buf, i%360);
        ret = spi_device_transmit( spi, &t );
        ets_delay_us(5);
        vTaskDelay(0);
        }
}

