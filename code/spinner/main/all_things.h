
#include <stdio.h>
#include <string.h>


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
void server(void* args);
void ota_server(void* args);

