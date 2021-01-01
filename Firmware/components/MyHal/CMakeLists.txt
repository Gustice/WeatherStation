idf_component_register(SRCS "I2cPort.cpp" 
                            "SpiPort.cpp"                             
                            "GpioPort.cpp" 
                            "AdcPort.cpp"  
                       INCLUDE_DIRS "include" "private"
                       REQUIRES "esp8266"
                       )
