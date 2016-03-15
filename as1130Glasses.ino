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

// Auto-cycle delay per pattern (in ms).
#define AUTO_CYCLE_DELAY_MS                 15000

//// GLOBALS

// Bitmap buffer, 24 column x 8 row bit arrays (on/off frame).
byte GlassesBits[24][2]                     = {{0}};

// PWM buffer, 24 column x 8 row byte array (PWM frame).
byte GlassesPWM[24][9]                      = {};

// Pattern initialization flag.
boolean patternInit                         = false;

boolean autoCycle                           = true;
boolean lastHeld                            = false;
unsigned long autoTimer                     = 0;
byte currentPattern                         = 0;
int brightAct                               = 0;
byte charShow                               = 0;

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
#include "glassesPatterns.h"

// Initialize program.
void setup() {
    // Start I2C and set to 400KHz bus speed (on 8MHz device).
    Wire.begin();
    TWBR = 2;

    // Preconfigure AS1130 chips and initialize.
    glassesInit();

    // Prepare button inputs.
    #if G_VERSION == 0
        pinMode(9, INPUT_PULLUP);
        pinMode(10, INPUT_PULLUP);
    #else
        pinMode(3, INPUT_PULLUP);
        pinMode(4, INPUT_PULLUP);
    #endif

    // Serial.begin(9600);

    // Start 5ms interrupt for button debouncing.
    setupTimerInterrupt();
}

// Main program loop (incomplete).
void loop() {
    if (((millis() - autoTimer > AUTO_CYCLE_DELAY_MS) && autoCycle == true) || onButtonPressed(0)) {
        currentPattern++;
        if (currentPattern > 12) currentPattern = 0;
        autoTimer = millis();
        patternInit = false;
    }

    if (onButtonPressed(1)) cycleBrightness();

    if (!lastHeld && onButtonHeld(0)) {
        autoCycle = !autoCycle;
        lastHeld = true;
        if (autoCycle) {
            switchDrawType(0,0);
            displayChar('A');
            delay(500);
            patternInit = false;
        }
        else {
            switchDrawType(0,0);
            displayChar('M');
            delay(500);
            patternInit = false;
        }
    }
    else if (lastHeld && !onButtonHeld(0)) {
        lastHeld = false;
    }

    switch(currentPattern) {
        case 0:
            beatingHearts();
            break;
        case 1:
            sines();
            break;
        case 2:
            fire();
            break;
        case 3:
            emote();
            break;
        case 4:
            starField();
            break;
        case 5:
            fakeEQ();
            break;
        case 6:
            slantBars();
            break;
        case 7:
            sideRain(0);
            break;
        case 8:
            sparkles();
            break;
        case 9:
            scrollMessage(2, SCROLL2X);
            break;
        case 10:
            rain(true);
            break;
        case 11:
            plasma();
            break;
        case 12:
            fullOn();
            break;
    }

    /*
    readBrightness();
    brightAct++;
    if (brightAct > 10) {
        brightAct = 0;
        // Serial.println(smoothedBrightness);

        int autoBright = smoothedBrightness;
        if (autoBright < 50) autoBright = 50;
        if (autoBright > 255) autoBright = 255;
        setBrightness(I2C_ADDR_AS1130_R, autoBright);
        setBrightness(I2C_ADDR_AS1130_L, autoBright);
    }
    */

    delay(1);
}
