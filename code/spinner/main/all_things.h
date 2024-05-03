
#include <stdio.h>
#include <string.h>


#define MOUNT_POINT "/sdcard"
#define FALSE 0
#define TRUE 1

#define sleepy_time( ms )    vTaskDelay( ms / 10 )

// TYPES
typedef struct
{
    uint8_t r;
    uint8_t b;
    uint8_t g;
} rgb_t;



extern rgb_t g_buf[360][15];
#define G_BUF_SZ (360*15*sizeof(rgb_t))


// DECLARATIONS
void frame_buffer(void* args);
void ota_server(void* args);
void sd_main(void);
int read_pic( const char * const f_name, uint *buf, size_t sz );

void sd_open(void);
void sd_close(void);
