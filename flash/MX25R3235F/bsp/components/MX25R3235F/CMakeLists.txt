# idf_component_register(SRCS "MX25R3235F.c" 
#                        INCLUDE_DIRS "."
#                        PRIV_REQUIRES "driver")
idf_component_register(SRCS "MX25R3235F.c"
                       INCLUDE_DIRS "." "${IDF_PATH}/components/spi_flash/include"
                       PRIV_REQUIRES "driver" "spi_flash")
