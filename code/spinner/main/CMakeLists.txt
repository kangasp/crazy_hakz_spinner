idf_component_register(SRCS "pic_data.c" "ota.c" "server.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES web/index.html
                    EMBED_TXTFILES web/ota.html
                    )