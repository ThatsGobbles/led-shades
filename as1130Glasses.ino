// AS1130 LED Shades
// Designed for 8MHz 3.3V ATMega328P

//// MAIN INCLUDES

#include <Wire.h>                           // I2C library.
#include <avr/pgmspace.h>                   // Flash storage of long strings and fonts.

//// DEFINES

// I2C addresses for AS1130 chips.
#define I2C_ADDR_AS1130_L                   0x30
#define I2C_ADDR_AS1130_R                   0x37

// Initial brightness value.
#define STARTING_BRIGHTNESS                 16

#define G_VERSION                           1

// Dimensions of PWM buffers.
#define NUM_LED_COLS                        24
#define NUM_LED_ROWS                        8
#define NUM_LED_BUFS                        2

// Auto-cycle delay per pattern (in ms).
#define AUTO_CYCLE_DELAY_MS                 15000

//// GLOBALS

// Bitmap buffer, 24 column x 8 row bit arrays (on/off frame).
byte GlassesBits[24][2]                     = {{0}};

// PWM buffer, 24 column x 8 row byte array (PWM frame).
byte GlassesPWM[NUM_LED_COLS][NUM_LED_ROWS][NUM_LED_BUFS]                   = {};

// Effect initialization flag.
boolean effectInit                          = false;

boolean autoCycle                           = false;
boolean lastHeld                            = false;
unsigned long autoTimer                     = 0;
byte currentPattern                         = 0;
int brightAct                               = 0;
byte charShow                               = 0;
unsigned long audioMillis                   = 0;
unsigned long currentMillis                 = 0;
unsigned long cycleMillis                   = 0;

byte currentEffect                          = 0;

//// AUX INCLUDES

// Lookup tables stored in flash memory.
#include "lookupMessageStrings.h"
#include "lookupCie1931.h"
#include "lookupQsine.h"
#include "lookupPcBiosFont.h"
#include "lookupGraphicFrames.h"

// Helper functions for AS1130 interfacing and graphics generation.
#include "as1130Functions.h"
#include "glassesFunctions.h"
#include "audio.h"
#include "effects.h"
#include "buttons.h"

// Initialize program.
void setup() {
    // Start I2C and set to 400KHz bus speed (on 8MHz device).
    Wire.begin();
    TWBR = 2;

    // Preconfigure AS1130 chips and initialize.
    glassesInit();

    pinMode(MODEBUTTON, INPUT_PULLUP);              // Prepare button input
    pinMode(BRIGHTNESSBUTTON, INPUT_PULLUP);        // Prepare button input
    analogReference(DEFAULT);                       // Select analog reference source
    pinMode(AUDIO_ANALOG_PIN, INPUT);                      // Set MSGEQ7 input pin to input
    pinMode(AUDIO_STROBE_PIN, OUTPUT);                     // Set MSGEQ7 strobe pin to output
    pinMode(AUDIO_RESET_PIN, OUTPUT);                      // Set MSGEQ7 reset pin to output
    digitalWrite(AUDIO_RESET_PIN, LOW);                    // Set MSGEQ7 reset pin to initial value
    digitalWrite(AUDIO_STROBE_PIN, HIGH);                  // Set MSGEQ7 strobe pin to initial value
}

functionList effectList[] = {
    beatingHearts,
    animeShades,
    shiftBoxes,
    oscCheckers,
    fire,
    emote,
    starField,
    fillAudioPWM,
    slantBars,
    rider,
    plasma,
};

const byte numEffects = (sizeof(effectList)/sizeof(effectList[0]));

// Main program loop (incomplete).
void loop() {
    currentMillis = millis();
    updateButtons();
    doButtons();

    if (currentMillis - audioMillis > AUDIO_DELAY) {
        audioMillis = currentMillis;  
        doAnalogs();
    }

    // switch to a new effect every cycleTime milliseconds
    if (currentMillis - cycleMillis > AUTO_CYCLE_DELAY_MS && autoCycle == true) {
        cycleMillis = currentMillis; 
        if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
        effectInit = false; // trigger effect initialization when new effect is selected
    }
    
    // // run the currently selected effect every effectDelay milliseconds
    // if (currentMillis - effectMillis > effectDelay) {
    //     effectMillis = currentMillis;
    //     effectList[currentEffect](); // run the selected effect function
    // }

    effectList[currentEffect](); // run the selected effect function
}
