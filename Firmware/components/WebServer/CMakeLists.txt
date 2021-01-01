idf_component_register(SRCS "SimpleServer.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp_http_server" "json" "nvs_flash" "WifiConnect" "Memory"
                    EMBED_FILES "WebPage/mainPage.html"
                    )
