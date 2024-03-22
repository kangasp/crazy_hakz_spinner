idf_component_register(SRCS "ota.c" "server.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES web/index.html
                    )