idf_component_register(SRCS "src/sim800l_core.c" "src/sim800l_misc.c" "src/sim800l_sms.c" "src/sim800l_call.c" "src/sim800l_http.c" "src/sim800l_bearer.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event driver)
