
Import('env')

mcu = env.BoardConfig().get("build.mcu", None)
core = env.BoardConfig().get("build.core", None)
build_flags = []

match mcu:
    case 'esp32s2':
        build_flags.append("-DCFG_TUSB_MCU=OPT_MCU_ESP32S2")
    case 'esp32s3':
        build_flags.append("-DCFG_TUSB_MCU=OPT_MCU_ESP32S3")

match core:
    case 'nRF5':
        build_flags.append("-DCFG_TUSB_MCU=OPT_MCU_NRF5X")
        
env.Append(CPPDEFINES=build_flags)