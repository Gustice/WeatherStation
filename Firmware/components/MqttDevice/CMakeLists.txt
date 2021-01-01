idf_component_register(SRCS "MqttDevice.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "MyHal" "mqtt" "json" "nvs_flash" "WifiConnect")
