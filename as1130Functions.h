// Mostly generic functions for AS1130 use

// Entry code for changing memory registers
#define REGISTER_SELECT                     0xFD

// Memory Registers
#define MEMORY_ON_OFF_START                 0x01
#define MEMORY_BLINK_PWM_START              0x40
#define MEMORY_DOT_CORRECTION               0x80
#define MEMORY_CONTROL_REGISTERS            0xC0

// Control Registers
#define CONTROL_PICTURE                     0x00
#define CONTROL_MOVIE                       0x01
#define CONTROL_MOVIE_MODE                  0x02
#define CONTROL_FRAME_TIME                  0x03
#define CONTROL_DISPLAY_OPTION              0x04
#define CONTROL_CURRENT_SOURCE              0x05
#define CONTROL_AS1130_CONFIG               0x06
#define CONTROL_INTERRUPT_MASK              0x07
#define CONTROL_INTERRUPT_FRAME_DEF         0x08
#define CONTROL_SHUTDOWN_OPEN_SHORT         0x09
#define CONTROL_I2C_INTERFACE_MON           0x0A
#define CONTROL_CLK_SYNC                    0x0B
#define CONTROL_INTERRUPT_STATUS            0x0E
#define CONTROL_AS1130_STATUS               0x0F
#define CONTROL_OPEN_LED_BEGIN              0x20

// Send a value to a device's register
void writeRegister(byte addr, byte reg, byte value) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

// Select control memory area for subsequent writes
void selectControlMemory(byte address) {
    writeRegister(address, REGISTER_SELECT, MEMORY_CONTROL_REGISTERS);
}

// Select memory frame to display
void setFrame(byte address, byte frame, byte enable) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_PICTURE, ((enable > 0) << 6) | (frame & 0b11111));
}

// Set movie display options
void setMovie(byte address, byte startFrame, byte frameCount, byte loopFrame, byte enable) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_MOVIE, ((enable > 0) << 6) | (startFrame & 0b11111));
    writeRegister(address, CONTROL_MOVIE_MODE, (1 << 7) | ((loopFrame > 0) << 6) | (frameCount & 0b11111));
}

// Set movie frame speed and scrolling options
void setMovieOptions(byte address, byte fading, byte scrollDir, byte enableScroll, byte frameDelay) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_FRAME_TIME, ((fading > 0) << 7) | ((scrollDir > 0) << 6) | ((enableScroll > 0) << 4) | (frameDelay & 0b1111));
}

// Configure movie looping options
void setMovieLooping(byte address, byte loops) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_DISPLAY_OPTION, ((loops & 0b111) << 5) | 0b1011);
}

// Set brightness level for device
void setBrightness(byte address, byte brightness) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_CURRENT_SOURCE, brightness);
}

// Set configuration options
#define AS1130_low_vdd_rst           7
#define AS1130_low_vdd_stat          6
#define AS1130_led_error_correction  5
#define AS1130_dot_corr              4
#define AS1130_common_addr           3
void setConfigs(byte address, byte options, byte memConfig) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_AS1130_CONFIG, options | (memConfig & 0b111));
}

// Configure interrupt mask
#define AS1130_selected_pic    7
#define AS1130_watchdog        6
#define AS1130_por             5
#define AS1130_overtemp        4
#define AS1130_low_vdd         3
#define AS1130_open_err        2
#define AS1130_short_err       1
#define AS1130_movie_fin       0
void setInterruptMask(byte address, byte options) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_INTERRUPT_MASK, options);
}

// Select movie frame to generate interrupt
void setInterruptFrame(byte address, byte frame) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_INTERRUPT_FRAME_DEF, (frame & 0b11111));
}

// Configure test/shutdown register
#define AS1130_test_all    4
#define AS1130_auto_test   3
#define AS1130_manual_test 2
#define AS1130_init        1
#define AS1130_shdn        0
void setShutdownTest(byte address, byte options) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_SHUTDOWN_OPEN_SHORT, (options & 0b11111));
}

// Configure I2C watchdog
void setI2CWatchdog(byte address, byte timeout, byte enable) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_I2C_INTERFACE_MON, ((timeout & 0b11111) << 1) | (enable > 0));
}

// Configure clock sync
#define AS1130_clock_speed_1MHz    0b00
#define AS1130_clock_speed_500kHz  0b01
#define AS1130_clock_speed_125kHz  0b10
#define AS1130_clock_speed_32kHz   0b11
#define AS1130_sync_OUT            0b10
#define AS1130_sync_IN             0b01
void setClockSync(byte address, byte clockSpeed, byte syncDir) {
    selectControlMemory(address);
    writeRegister(address, CONTROL_CLK_SYNC, ((clockSpeed & 0b11) << 2) | (syncDir & 0b11));
}
