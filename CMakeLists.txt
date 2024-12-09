idf_component_register(SRCS "src/sim800l_core.c" "src/sim800l_misc.c" "src/sim800l_urc.c" "src/sim800l_sms.c" "src/sim800l_call.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event driver)
