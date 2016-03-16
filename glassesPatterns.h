// Graphical pattern functions for glasses.

//// GLOBALS
float incRval = 0;
float incr = 0.3;
byte incDir = 1;

float offset  = 0;
float plasIncr = -1;
float plasVector = 0;

#define SOLID 255
#define EMPTY 0

void expandByte(byte col, byte value) {
    for (byte i = 0; i < 8; i++) {
        GlassesPWM[col][i][0] = value & 0b10000000 ? SOLID : EMPTY;
        value <<= 1;
    }
}

void expandByteRev(byte col, byte value) {
    for (byte i = 0; i < 8; i++) {
        GlassesPWM[col][i][0] = value & 0b00000001 ? SOLID : EMPTY;
        value >>= 1;
    }
}

// Draw a sine wave on the bit array.
// Speeds up and slows down in a cycle.
void sines() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    for (int i = 0; i < NUM_LED_COLS; i++) {
        expandByte(i, 3 << (int)(sin(i / 2.0 + incRval) * 3.5 + 3.5));
    }

    writePWMFrame(0);

    incRval += incr;
    if (incDir == 1) incr += 0.001;
    else incr -= 0.001;

    if (incr > 0.5) incDir = 0;
    if (incr < 0.1) incDir = 1;
  
    delay(5);
}

// Draw a circular sine plasma.
int plasOffset = 0;
void plasma() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < 8; y++) {
            byte brightness = qsine(sqrt((x-11.5)*(x-11.5) + (y-3.5)*(y-3.5))*60 + plasOffset);

            GlassesPWM[x][y][0] = pgm_read_byte(&Cie1931LookupTable[brightness]);
        }
    }

    writePWMFrame(0);
    plasOffset += 15;
    if (plasOffset > 359) plasOffset -= 359;
}

#define FADE_INCREMENT 0.9
void fadeAllPWM() {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            GlassesPWM[x][y][0] *= FADE_INCREMENT;
        }
    }
}

// Initialize / load message string
byte currentCharColumn = 0;
int currentMessageChar = 0;
void initMessage(byte message) {
    currentCharColumn = 0;
    currentMessageChar = 0;
    selectFlashString(message);
    loadCharBuffer(loadStringChar(message, currentMessageChar));
}

// Draw message scrolling across the two arrays.
// SCROLL1X is normal scrolling.
// SCROLL2X is page-flipping scroll that simulates double horizontal resolution using persistence of vision.
#define SCROLL1X 0
#define SCROLL2X 1
void scrollMessage(byte message, byte mode) {
    if (!patternInit) {
        switchDrawType(0, 0);
        initMessage(message);
        patternInit = true;
    }

    if ((currentCharColumn % 2 == 0) || mode != SCROLL2X) scrollBits(1, 0);
    else scrollBits(1, 1);

    if ((currentCharColumn % 2 == 1) || mode != SCROLL2X) {
        GlassesBits[23][0] = charBuffer[currentCharColumn];
        writeBitFrame(0, 0);
    }
    else {
        GlassesBits[23][1] = charBuffer[currentCharColumn];
        writeBitFrame(0,1);
    }

    currentCharColumn++;
    if (currentCharColumn > 7) {
        currentCharColumn = 0;
        currentMessageChar++;
        char nextChar = loadStringChar(message, currentMessageChar);
        if (nextChar == 0) {
            currentMessageChar = 0;
            nextChar = loadStringChar(message, currentMessageChar);
        }
        loadCharBuffer(nextChar);
    }

    if (mode != SCROLL2X) delay(30);
    else delay(10);
}

#define RAIN_DELAY 20
int rainAction = 0;
void sideRain(byte dir) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (rainAction++ > RAIN_DELAY) {
        rainAction = 0;

        byte tempRain = 0;

        tempRain = (1 << random(0,8)) | (1 << random(0,8));

        if (dir == 0) {
            scrollPWM(0);
            expandByte(0, tempRain);
        }
        else {
            scrollPWM(1);
            expandByte(23, tempRain);
        }

        writePWMFrame(0);
    }
}

byte vRainCols[NUM_LED_COLS] = {0};

void rain(boolean up) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (rainAction++ > RAIN_DELAY) {
        rainAction = 0;

        for (int i = 0; i < 24; i++) {
            vRainCols[i] <<= 1;
        }

        vRainCols[random(0,24)] |= 1;
        vRainCols[random(0,24)] |= 1;

        for (int i = 0; i < 24; i++) {
            up ? expandByte(i, vRainCols[i]) : expandByteRev(i, vRainCols[i]);
        }

        writePWMFrame(0);
    }
}

typedef struct Stars {
    float xIncr;
    float yIncr;
    float xPos;
    float yPos;
};

#define NUM_STARS 10

Stars stars[NUM_STARS];

void starField() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    fadeAllPWM();
    for (int i = 0; i < NUM_STARS; i++) {
        if (abs(stars[i].xIncr) < 0.02 || abs(stars[i].yIncr) < 0.02) {
            stars[i].xPos = 11.5;
            stars[i].yPos = 3.5;
            stars[i].xIncr = random(0,200)/100.0 - 1.0;
            stars[i].yIncr = random(0,200)/200.0 - 0.5;
        }

        stars[i].xPos += stars[i].xIncr;
        stars[i].yPos += stars[i].yIncr;

        int xPos = (int)stars[i].xPos;
        int yPos = (int)stars[i].yPos;

        if (xPos < 0 || xPos > 23 || yPos < 0 || yPos > 7) {
            stars[i].xIncr = 0;
            stars[i].yIncr = 0;
        }
        else {
            GlassesPWM[xPos][yPos][0] = 255;
        }
    }

    writePWMFrame(0);
}

byte blinkAction = 0;
#define BLINK_COUNT 50
void fullOn() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (blinkAction++ > BLINK_COUNT) {
        blinkAction = 0;
    }

    if(blinkAction > BLINK_COUNT/2) {
        for (int x = 0; x < 24; x++) {
            for (int y = 0; y < 8; y++) {
                GlassesPWM[x][y][0] = 255;
            }
        }
    }
    else {
        for (int x = 0; x < 24; x++) {
            for (int y = 0; y < 8; y++) {
                GlassesPWM[x][y][0] = 0;
            }
        }
    }

    writePWMFrame(0);
}

int slantPos = 23;
byte slantAction = 0;
#define SLANT_COUNT 3
void slantBars() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (slantAction++ > SLANT_COUNT) {
        slantAction = 0;

        for (int x = 0; x < 24; x++) {
            for (int y = 0; y < 8; y++) {
                GlassesPWM[x][y][0] = pgm_read_byte(&Cie1931LookupTable[(((x + y + (int)slantPos) % 8) * 32)]);
            }
        }

        slantPos--;
        if (slantPos < 0) slantPos = 23;

        writePWMFrame(0);
    }
}

#define SPARKLE_COUNT 5
void sparkles() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    fadeAllPWM();
    for (int i = 0; i < SPARKLE_COUNT; i++) GlassesPWM[random(0, 24)][random(0, 8)][0] = 255;
    writePWMFrame(0);
}

// int riderCount = 0;
// int riderPos = 0;
// int tpos = 0;
// void rider() {
//     if (!patternInit) {
//         switchDrawType(0, 1);
//         patternInit = true;
//         riderCount = 0;
//         riderPos = 0;
//         tpos = 0;
//     }

//     fadeAllPWM();
//     if (riderCount++ > 5) {
//         riderCount = 0;

//         if (riderPos < 8)           tpos = riderPos;
//         else if (riderPos < 12)     tpos = -1;
//         else if (riderPos < 20)     tpos = 19 - riderPos;
//         else if (riderPos <= 40)    tpos = -1;
//         else if (riderPos > 40)     riderPos = 0;

//         for (int x = tpos*3; x < (tpos * 3 + 3); x++) {
//             for (int y = 0; y < 8; y++) {
//                 GlassesPWM[x][y][0] = pgm_read_byte(&Cie1931LookupTable[255*(tpos != -1)]);
//             }
//         }

//         riderPos++;
//         writePWMFrame(0);
//     }
// }

// Simply grab a character from the font and put it in the 8x8 section of both sides of the glasses.
void displayChar(int character) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    loadCharBuffer(character);

    for (int i = 0; i < 8; i++) {
        expandByteRev(i+1, charBuffer[i]);
        expandByteRev(i+15, charBuffer[i]);
    }

    writePWMFrame(0);
}

// Draw various emoticon style faces.
int emotecounter = 0;
byte currentEmote = 0;
#define EMOTE_DELAY 10
void emote() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        currentEmote = 0;
    }

    if (emotecounter == 0) {
        switch(currentEmote) {
            case 0:
                loadCharBuffer('X');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('X');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
            case 1:
                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
            case 2:
                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
            case 3:
                loadCharBuffer('>');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('<');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
            case 4:
                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
            case 5:
                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+2, charBuffer[i]);
                }

                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    expandByteRev(i+15, charBuffer[i]);
                }
                break;
        }

        currentEmote = (currentEmote + 1) % 6;
    }

    emotecounter = (emotecounter + 1) % EMOTE_DELAY;

    writePWMFrame(0);
}

int fireAction = 0;
int fireRandom = 0;
byte lineBuffer[24] = {0};
byte nextFireLine[24] = {0};
byte fireLookup(byte x, byte y) {
    y = y % (8 + 1);
    if (y < 8) {
        return GlassesPWM[x % 24][y][0];
    }
    else if (y == 8) {
        return lineBuffer[x % 24];
    }
}

void fire() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (fireAction++ > 2) {
        fireAction = 0;
        int x;

        for (x = 0; x < 24; x++) {
            lineBuffer[x] = (random(0,4) == 1) * 255;
        }

        for (int y = 0; y < 8 ; y++) {
            for (x = 0; x < 24; x++) {
                int tempBright = fireLookup(x - 1, y + 1)
                               + fireLookup(x + 1, y + 1)
                               + fireLookup(x, y + 1)
                               + fireLookup(x, y + 2);
                tempBright = tempBright / 3.7 - 10;
                if (tempBright < 0) tempBright = 0;
                if (tempBright > 255) tempBright = 255;
                GlassesPWM[x][y][0] = tempBright;
            }
        }

        writePWMFrame(0);
    }
}

void loadGraphicsFrame(int frame) {
    for (int x = 0; x < 24; x++) {
        expandByteRev(x, pgm_read_byte(Graphics[frame]+x));
    }
}

// Awww!
byte currentHeartFrame = 0;
byte heartLoopCount = 0;
void beatingHearts() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    heartLoopCount++;
    if (heartLoopCount > 50) {
        heartLoopCount = 0;

        if (currentHeartFrame < 3) loadGraphicsFrame(currentHeartFrame);
        else loadGraphicsFrame(5 - currentHeartFrame);

        currentHeartFrame++;
        if (currentHeartFrame > 5) currentHeartFrame = 0;

        writePWMFrame(0);
    }
}

byte eqLevels[12] = {0};
int eqDecay = 0;
int eqRandomizerDelay = 0;
int eqRandomizerCap = 0;
#define EQ_DECAY_SPEED 85
#define EQ_MIN_INTERVAL 100
#define EQ_MAX_INTERVAL 400
void fakeEQ() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        eqRandomizerCap = random(0, EQ_MAX_INTERVAL - EQ_MIN_INTERVAL) + EQ_MIN_INTERVAL;
    }

    // decay the eq array at a set interval
    eqDecay++;
    if (eqDecay > EQ_DECAY_SPEED) {
        eqDecay = 0;
        for (byte i = 0; i < 12; i++) {
            if (eqLevels[i] > 0) eqLevels[i]--;
        }
    }

    // splash random bars at a semi-random interval
    eqRandomizerDelay++;
    if (eqRandomizerDelay >= eqRandomizerCap) {
        eqRandomizerDelay = 0;
        eqRandomizerCap = random(0, EQ_MAX_INTERVAL - EQ_MIN_INTERVAL) + EQ_MIN_INTERVAL;
        for (byte i = 0; i < 12; i++) {
            byte eqNewLevel = random(0, 9);
            if (eqLevels[i] < eqNewLevel) eqLevels[i] = eqNewLevel;
        }
    }

    // render the bars if something visible has happened
    if (eqDecay == 0 || eqRandomizerDelay == 0) {
        for (byte i = 0; i < 12; i++) {
            expandByteRev(i*2, 0xFF << (8 - eqLevels[i]));
            expandByteRev(i*2+1, 0xFF << (8 - eqLevels[i]));
        }
        writePWMFrame(0);
    }
}

#define PI 3.14159
// Adapted from http://gizma.com/easing/
float easeInOutQuad(float t, float start, float change, float duration) {
    t /= duration/2;
    if (t < 1) return change/2*t*t + start;
    t--;
    return -change/2 * (t*(t-2) - 1) + start;
}
float easeInOutSine(float t, float start, float change, float duration) {
    return -change/2 * (cos(PI*t/duration) - 1) + start;
}

// Setting this too low will cause skips in the current version of rider.
#define EASING_DURATION 48
#define EASING_SLEEP 32
int tRider;
int sleepCooldown;
int dirRider;
float pRider;
void rider() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        tRider = 0;
        dirRider = 1;
        sleepCooldown = 0;
    }

    pRider = easeInOutSine(tRider, 0, (NUM_LED_COLS - 0.1), EASING_DURATION);

    fadeAllPWM();
    expandByte((byte)pRider, 0b11111111);
    writePWMFrame(0);

    if (tRider >= EASING_DURATION && dirRider > 0) {
        tRider = EASING_DURATION;
        dirRider = -1;
        sleepCooldown = EASING_SLEEP;
    }
    else if (tRider <= 0 && dirRider < 0) {
        tRider = 0;
        dirRider = 1;
        sleepCooldown = EASING_SLEEP;
    }

    if (sleepCooldown > 0) sleepCooldown--;
    else tRider += dirRider;
}
