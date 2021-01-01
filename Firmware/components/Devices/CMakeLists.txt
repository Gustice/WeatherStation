idf_component_register(SRCS "Shtc3_Sensor.cpp" "Display_SSD1306.cpp" "MyDisplay.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES "esp8266" "MyHal" "Adafruit-GFX-Library")
