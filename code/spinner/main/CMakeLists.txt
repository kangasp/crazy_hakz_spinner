idf_component_register(SRCS "sd.c" "pic_data.c" "ota.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES web/index.html
                    EMBED_TXTFILES web/ota.html
                    )