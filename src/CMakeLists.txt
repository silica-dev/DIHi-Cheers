add_executable(Pico_Game_Controller
        pico_game_controller.c
        usb_descriptors.c)

target_include_directories(Pico_Game_Controller PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(Pico_Game_Controller PRIVATE pico_game_controller.c)

target_link_libraries(Pico_Game_Controller PRIVATE
        pico_multicore
        pico_stdlib
        tinyusb_device
        tinyusb_board
        hardware_pio
        hardware_dma
        hardware_irq)

pico_add_extra_outputs(Pico_Game_Controller)

add_custom_command(TARGET Pico_Game_Controller
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/Pico_Game_Controller.uf2 ${PROJECT_SOURCE_DIR}/build_uf2/Pico_Game_Controller.uf2
)
