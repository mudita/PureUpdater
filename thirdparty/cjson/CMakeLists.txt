add_library(cjson STATIC)

target_sources(cjson
        PRIVATE
        src/cJSON/cJSON.c
        )

target_include_directories(cjson
        PUBLIC
        src
        PRIVATE
        src/cJSON
        )
