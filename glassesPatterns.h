// Graphical pattern functions for glasses.

//// GLOBALS
float incRval = 0;
float incr = 0.3;
byte incDir = 1;

float offset  = 0;
float plasIncr = -1;
float plasVector = 0;

// Draw a sine wave on the bit array.
// Speeds up and slows down in a cycle.
void sines() {
    if (!patternInit) {
        switchDrawType(0, 0);
        patternInit = true;
    }

    for (int i = 0; i < 24; i++) {
        GlassesBits[i][0] = 3 << (int)(sin(i / 2.0 + incRval) * 3.5 + 3.5);
    }

    writeBitFrame(0, 0);

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

    for (int x = 0; x < 24; x++) {
        for (int y = 0; y < 8; y++) {
            byte brightness = qsine(sqrt((x-11.5)*(x-11.5) + (y-3.5)*(y-3.5))*60 + plasOffset);

            GlassesPWM[x][y] = pgm_read_byte(&Cie1931LookupTable[brightness]);
        }
    }

    writePWMFrame(0);
    plasOffset += 15;
    if (plasOffset > 359) plasOffset -= 359;
}

#define FADE_INCREMENT 0.9
void fadeAllPWM() {
    for (int x = 0; x < 24; x++) {
        for (int y = 0; y < 8; y++) {
            GlassesPWM[x][y] *= FADE_INCREMENT;
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
        switchDrawType(0, 0);
        patternInit = true;
    }

    if (rainAction++ > RAIN_DELAY) {
        rainAction = 0;

        byte tempRain = 0;

        tempRain = (1 << random(0,8)) | (1 << random(0,8));

        if (dir == 0) {
            scrollBits(0,0);
            GlassesBits[0][0] = tempRain;
        }
        else {
            scrollBits(1,0);
            GlassesBits[23][0] = tempRain;
        }

        writeBitFrame(0, 0);
    }
}

void rain() {
    if (!patternInit) {
        switchDrawType(0, 0);
        patternInit = true;
    }

    if (rainAction++ > RAIN_DELAY) {
        rainAction = 0;

        for (int i = 0; i < 24; i++) {
            GlassesBits[i][0] <<= 1;
        }

        GlassesBits[random(0,24)][0] |= 1;
        GlassesBits[random(0,24)][0] |= 1;

        writeBitFrame(0, 0);
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
            GlassesPWM[xPos][yPos] = 255;
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
                GlassesPWM[x][y] = 255;
            }
        }
    }
    else {
        for (int x = 0; x < 24; x++) {
            for (int y = 0; y < 8; y++) {
                GlassesPWM[x][y] = 0;
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
                GlassesPWM[x][y] = pgm_read_byte(&Cie1931LookupTable[(((x + y + (int)slantPos) % 8) * 32)]);
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
    for (int i = 0; i < SPARKLE_COUNT; i++) GlassesPWM[random(0, 24)][random(0, 8)] = 255;
    writePWMFrame(0);
}

int riderCount = 0;
int riderPos = 0;
int tpos = 0;
void rider() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        riderCount = 0;
        riderPos = 0;
        tpos = 0;
    }

    fadeAllPWM();
    if (riderCount++ > 5) {
        riderCount = 0;

        if (riderPos < 8)           tpos = riderPos;
        else if (riderPos < 12)     tpos = -1;
        else if (riderPos < 20)     tpos = 19 - riderPos;
        else if (riderPos <= 40)    tpos = -1;
        else if (riderPos > 40)     riderPos = 0;

        for (int x = tpos*3; x < (tpos * 3 + 3); x++) {
            for (int y = 0; y < 8; y++) {
                GlassesPWM[x][y] = pgm_read_byte(&Cie1931LookupTable[255*(tpos != -1)]);
            }
        }

        riderPos++;
        writePWMFrame(0);
    }
}

// Simply grab a character from the font and put it in the 8x8 section of both sides of the glasses.
void displayChar(int character) {
    if (!patternInit) {
        switchDrawType(0, 0);
        patternInit = true;
    }

    loadCharBuffer(character);

    for (int i = 0; i < 8; i++) {
        GlassesBits[i+1][0] = charBuffer[i];
        GlassesBits[i+15][0] = charBuffer[i];
    }

    writeBitFrame(0, 0);
}

// Draw various emoticon style faces.
int emotecounter = 0;
byte currentEmote = 0;
#define EMOTE_DELAY 100
void emote() {
    if (!patternInit) {
        switchDrawType(0, 0);
        patternInit = true;
        currentEmote = 0;
    }

    if (emotecounter == 0) {
        switch(currentEmote) {
            case 0:
                loadCharBuffer('X');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('X');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
            case 1:
                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
            case 2:
                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
            case 3:
                loadCharBuffer('>');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('<');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
            case 4:
                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
            case 5:
                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+2][0] = charBuffer[i];
                }

                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    GlassesBits[i+15][0] = charBuffer[i];
                }
                break;
        }

        currentEmote = (currentEmote + 1) % 6;
    }

    emotecounter = (emotecounter + 1) % EMOTE_DELAY;

    writeBitFrame(0, 0);
}

int fireAction = 0;
int fireRandom = 0;
byte lineBuffer[24] = {0};
void fire() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (fireAction++ > 2) {
        fireAction = 0;
        int x;

        // NOTE: Removed some commented out code, see original if needed.
        for (x = 0; x < 24; x++) {
            GlassesPWM[x][8] = (random(0,4) == 1) * 255;
        }

        for (int y = 1; y < 9 ; y++) {
            for (x = 0; x < 24; x++) lineBuffer[x] = GlassesPWM[x][y];
            for (x = 0; x < 24; x++) {
                int tempBright = (lineBuffer[(x-1) % 24] + lineBuffer[(x+1) % 24] + lineBuffer[x] + GlassesPWM[x][(y+1) % 9])/3.7-10;
                if (tempBright < 0) tempBright = 0;
                if (tempBright > 255) tempBright = 255;
                GlassesPWM[x][y-1] = tempBright;
            }
        }

        writePWMFrame(0);
    }
}

// Awww!
byte currentHeartFrame = 0;
byte heartLoopCount = 0;
void beatingHearts() {
    if (!patternInit) {
        switchDrawType(0, 0);
        patternInit = true;
    }

    heartLoopCount++;
    if (heartLoopCount > 50) {
        heartLoopCount = 0;

        if (currentHeartFrame < 3) loadGraphicsFrame(currentHeartFrame);
        else loadGraphicsFrame(5 - currentHeartFrame);

        currentHeartFrame++;
        if (currentHeartFrame > 5) currentHeartFrame = 0;

        writeBitFrame(0, 0);
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
        switchDrawType(0, 0);
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
            GlassesBits[i*2][0] = 0xFF << (8 - eqLevels[i]);
            GlassesBits[i*2+1][0] = 0xFF << (8 - eqLevels[i]);
        }
        writeBitFrame(0, 0);
    }
}
