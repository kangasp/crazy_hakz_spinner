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
#include "all_things.h"


#define LED_NUM  45
#define BUF_SZ  ((3 + LED_NUM) * sizeof(uint32_t))  // 1 uint32 for start, 2 uint32 for end
#define RGBL(r,g,b,lum) ((((gamma8[g]) & 0xFF)<<24) | (((gamma8[b]) & 0xFF)<<16) | (((gamma8[r]) & 0xFF)<<8) | (((lum) & 0xFF) | 0xE0)<<0  ) 
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



const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };






// static rgb_t g_buf[360][15] = {
// 0
// };

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
    uint8_t r, g, b, i, l;
    int s;
    #ifdef WORKEDDD
        memcpy( &buf[1],    &g_buf[p][0], sizeof(uint32_t)*15 );
        memcpy( &buf[1+15], &g_buf[(p+120) % 360][0], sizeof(uint32_t)*15 );
        memcpy( &buf[1+30], &g_buf[(p+240)%360][0], sizeof(uint32_t)*15 );
    #endif
    s = 0/*  */;
    // printf("Hello P: %d\n", p);
    for(i=0; i<LED_NUM; i++)
        {
        if( i >=0 && i < 15 )
            {
            s = p;
            }
        else if( i >=15 && i < 30 )
            {
            s = (p+240) % 360;
            }
        else if( i >=30 && i < 45 )
            {
            s = (p+120) % 360;
            }
        l = i % 15; 
        r = g_buf[s][l].r;
        g = g_buf[s][l].g;
        b = g_buf[s][l].b;

        buf[i+1] = RGBL(r,g,b,LUM);

        if ( i == 20 )
            buf[i+1] = RGBL(0,0,0,LUM);

        // if( p == 1)
        //     buf[i+1] = RGBL(0x0,0x55,0x55,LUM);
        // else
        //     buf[i+1] = RGBL(0x0,0x00,0x00,LUM);
        
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
    memset(buf, 0, sizeof(*buf));

    for( i = 0; i < LED_NUM+3; i++ )
    {
        buf[i] = 0x0;
    }


    buf[0] = 0x00000000;
    buf[LED_NUM+1] = 0xFFFFFFFF;
    buf[LED_NUM+2] = 0xFFFFFFFF;
    t.tx_buffer = buf;
    t.length = BUF_SZ * 8;


    // for( i = 0; i < 0; i++ )
    // {
    // for( int j = 0; j < 15; j++ )
    //     {
    //     g_buf[i][j].r = (g_stuff[i/6][j] >> 16) & 0xFF;
    //     g_buf[i][j].g = (g_stuff[i/6][j] >> 8) & 0xFF;
    //     g_buf[i][j].b = (g_stuff[i/6][j] >> 0) & 0xFF;
    //     }
    // }


// esp_err_t spi_device_transmit(spi_device_handle_t handle, spi_transaction_t *trans_desc);
    printf("Hello, Tester!\n");
    printf("rgb:  0x%02X, 0x%02X, 0x%02X\n", g_buf[346][14].r,  g_buf[346][14].g,  g_buf[346][14].b);
    i = 0;

    // TaskHandle_t myTaskHandle2 = NULL;
    // xTaskCreatePinnedToCore(server, "THE_SERVER!!", 4096, NULL,10, &myTaskHandle2, 1);
    ota_server( NULL );

    while( 1 )
        {
            /*
            spoke    radial
            -------------------
            1        90, 346   
            2       210, 106
            3       330, 226
            */
        i = encoder->get_counter_value(encoder);
        while( i < 0 ) i = i+360;
        update_buf(buf, i%360);
        ret = spi_device_transmit( spi, &t );
        ets_delay_us(5);
        vTaskDelay(0);
        }
}

