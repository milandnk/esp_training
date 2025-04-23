# idf_component_register(SRCS "uart_isr.c"
#                        INCLUDE_DIRS ".")
idf_component_register(
    SRCS "uart_isr.c"
    INCLUDE_DIRS "."
    REQUIRES driver
)
