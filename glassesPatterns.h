// Graphical pattern functions for glasses.

// Draw a sine wave on the bit array.
// Speeds up and slows down in a cycle.
float sinesIncRval;
float sinesIncr;
bool sinesIncreasing;
void sines() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        sinesIncRval = 0;
        sinesIncr = 0.3;
        sinesIncreasing = true;
    }

    for (int i = 0; i < NUM_LED_COLS; i++) {
        expandByte(i, 3 << (int)(sin(i / 2.0 + sinesIncRval) * 3.5 + 3.5), false, 0);
    }

    writePWMFrame(0, 0);

    sinesIncRval += sinesIncr;
    if (sinesIncreasing) sinesIncr += 0.001;
    else sinesIncr -= 0.001;

    if (sinesIncr > 0.5) sinesIncreasing = false;
    if (sinesIncr < 0.1) sinesIncreasing = true;

    delay(5);
}

// Draw a circular sine plasma.
int plasOffset;
void plasma() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        plasOffset = 0;
    }

    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            byte brightness = qsine(sqrt((x-11.5)*(x-11.5) + (y-3.5)*(y-3.5))*60 + plasOffset);

            GlassesPWM[x][y][0] = pgm_read_byte(&Cie1931LookupTable[brightness]);
        }
    }

    writePWMFrame(0, 0);
    plasOffset += 15;
    if (plasOffset > 359) plasOffset -= 359;
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
void scrollMessage(byte messageId) {
    if (!patternInit) {
        switchDrawType(0, 1);
        initMessage(messageId);
        patternInit = true;
    }

    // Even, scroll 0 buffer
    // Odd, scroll 1 buffer
    // if (currentCharColumn % 2 == 0) hScrollPWM(0, false, false);
    // else hScrollPWM(1, false, false);
    hScrollPWM(currentCharColumn % NUM_LED_BUFS, false, false);

    // Even, write next column to 1 buffer and display it
    // Odd, write next column to 0 buffer and display it
    // if (currentCharColumn % 2 == 1) {
    //     expandByte(NUM_LED_COLS - 1, charBuffer[currentCharColumn], true, 0);
    //     writePWMFrame(0, 0);
    // }
    // else {
    //     expandByte(NUM_LED_COLS - 1, charBuffer[currentCharColumn], true, 1);
    //     writePWMFrame(0, 1);
    // }
    expandByte(NUM_LED_COLS - 1, charBuffer[currentCharColumn], true, (currentCharColumn + 1) % NUM_LED_BUFS);
    writePWMFrame(0, (currentCharColumn + 1) % NUM_LED_BUFS);

    currentCharColumn++;
    if (currentCharColumn >= NUM_COLS_PER_CHAR) {
        currentCharColumn = 0;
        currentMessageChar++;
        char nextChar = loadStringChar(messageId, currentMessageChar);
        if (nextChar == 0) {
            currentMessageChar = 0;
            nextChar = loadStringChar(messageId, currentMessageChar);
        }
        loadCharBuffer(nextChar);
    }

    delay(3);
}

#define MAX_RAIN_ACTION 20
#define MAX_NEW_RAIN_DROPS_H 2
#define MAX_NEW_RAIN_DROPS_V 4
int rainAction;
void sideRain(bool increasing) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        rainAction = 0;
    }

    if (rainAction++ > MAX_RAIN_ACTION) {
        rainAction = 0;

        fillScrollBufferH(0);
        for (int i = 0; i < MAX_NEW_RAIN_DROPS_H; i++) {
            ScrollBufferH[random(0, NUM_LED_ROWS)] = SOLID_PIXEL;
        }

        hScrollPWM(0, increasing, true);

        writePWMFrame(0, 0);
    }
}

void rain(boolean increasing) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (rainAction++ > MAX_RAIN_ACTION) {
        rainAction = 0;

        fillScrollBufferV(0);
        for (int i = 0; i < MAX_NEW_RAIN_DROPS_V; i++) {
            ScrollBufferV[random(0, NUM_LED_COLS)] = SOLID_PIXEL;
        }

        vScrollPWM(0, increasing, true);

        writePWMFrame(0, 0);
    }
}

typedef struct Stars {
    float xIncr;
    float yIncr;
    float xPos;
    float yPos;
};

#define NUM_STARS 10
#define STAR_MIN_X_INCR 0.02
#define STAR_MIN_Y_INCR 0.02

Stars stars[NUM_STARS];

void starField() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    mulAllPWM(FADE_FACTOR_SLOW, 0);
    for (int i = 0; i < NUM_STARS; i++) {
        if (abs(stars[i].xIncr) < STAR_MIN_X_INCR || abs(stars[i].yIncr) < STAR_MIN_Y_INCR) {
            stars[i].xPos = (NUM_LED_COLS - 1) / 2.0;
            stars[i].yPos = (NUM_LED_ROWS - 1) / 2.0;
            stars[i].xIncr = random(0,200)/100.0 - 1.0;
            stars[i].yIncr = random(0,200)/200.0 - 0.5;
        }

        stars[i].xPos += stars[i].xIncr;
        stars[i].yPos += stars[i].yIncr;

        int xPos = (int)stars[i].xPos;
        int yPos = (int)stars[i].yPos;

        if (xPos < 0 || xPos >= NUM_LED_COLS || yPos < 0 || yPos >= NUM_LED_ROWS) {
            stars[i].xIncr = 0;
            stars[i].yIncr = 0;
        }
        else {
            GlassesPWM[xPos][yPos][0] = 255;
        }
    }

    writePWMFrame(0, 0);
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

    writePWMFrame(0, 0);
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

        writePWMFrame(0, 0);
    }
}

#define SPARKLE_COUNT 5
void sparkles() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    mulAllPWM(FADE_FACTOR_SLOW, 0);
    for (int i = 0; i < SPARKLE_COUNT; i++) GlassesPWM[random(0, NUM_LED_COLS)][random(0, NUM_LED_ROWS)][0] = SOLID_PIXEL;
    writePWMFrame(0, 0);
}

// Simply grab a character from the font and put it in the 8x8 section of both sides of the glasses.
void displayChar(int character) {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    loadCharBuffer(character);

    for (int i = 0; i < 8; i++) {
        expandByte(i+1, charBuffer[i], true, 0);
        expandByte(i+15, charBuffer[i], true, 0);
    }

    writePWMFrame(0, 0);
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
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('X');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
            case 1:
                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('?');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
            case 2:
                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
            case 3:
                loadCharBuffer('>');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('<');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
            case 4:
                loadCharBuffer('o');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('O');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
            case 5:
                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+2, charBuffer[i], true, 0);
                }

                loadCharBuffer('^');
                for (int i = 0; i < 8; i++) {
                    expandByte(i+15, charBuffer[i], true, 0);
                }
                break;
        }

        currentEmote = (currentEmote + 1) % 6;
    }

    emotecounter = (emotecounter + 1) % EMOTE_DELAY;

    writePWMFrame(0, 0);
}

int fireAction = 0;
int fireRandom = 0;
byte lineBuffer[NUM_LED_COLS] = {0};
byte nextFireLine[NUM_LED_COLS] = {0};
#define MAX_FIRE_ACTION 4
byte fireLookup(byte x, byte y) {
    y = y % (NUM_LED_ROWS + 1);
    if (y < NUM_LED_ROWS) {
        return GlassesPWM[x % NUM_LED_COLS][y][0];
    }
    else if (y == NUM_LED_ROWS) {
        return lineBuffer[x % NUM_LED_COLS];
    }
}

void fire() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    if (fireAction++ > MAX_FIRE_ACTION) {
        fireAction = 0;
        int x;

        for (x = 0; x < NUM_LED_COLS; x++) {
            lineBuffer[x] = (random(0, 4) == 1) * SOLID_PIXEL;
        }

        for (int y = 0; y < 8 ; y++) {
            for (x = 0; x < NUM_LED_COLS; x++) {
                int tempBright = fireLookup(x - 1, y + 1)
                               + fireLookup(x + 1, y + 1)
                               + fireLookup(    x, y + 1)
                               + fireLookup(    x, y + 2);
                tempBright = tempBright / 3.7 - 10;
                if (tempBright < EMPTY_PIXEL) tempBright = EMPTY_PIXEL;
                if (tempBright > SOLID_PIXEL) tempBright = SOLID_PIXEL;
                GlassesPWM[x][y][0] = tempBright;
            }
        }

        writePWMFrame(0, 0);
    }
}

void loadGraphicsFrame(int frame) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        expandByte(x, pgm_read_byte(BeatingHeartFrames[frame]+x), true, 0);
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

        writePWMFrame(0, 0);
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
            expandByte(i*2, 0xFF << (8 - eqLevels[i]), true, 0);
            expandByte(i*2+1, 0xFF << (8 - eqLevels[i]), true, 0);
        }
        writePWMFrame(0, 0);
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

    mulAllPWM(FADE_FACTOR_SLOW, 0);
    expandByte((byte)pRider, 0b11111111, false, 0);
    writePWMFrame(0, 0);

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

void plotFour(int cx, int cy, int dx, int dy, float f) {
    smartPlotf(cx + dx, cy + dy, f);
    smartPlotf(cx - dx, cy + dy, f);
    smartPlotf(cx + dx, cy - dy, f);
    smartPlotf(cx - dx, cy - dy, f);
}

void wuEllipse(float cx, float cy, float w, float h) {
    int xi, yi; // Integer iterators
    float xj, yj; // Real-value x and y
    float frc;
    int flr;

    if (w <= 0 || h <= 0) return;

    float a = w / 2.0;
    float b = h / 2.0;
    float asq = a * a;
    float bsq = b * b;

    int ffd;

    ffd = (int)round(asq / sqrt(bsq + asq));
    for (int xi = 0; xi <= ffd; xi++) {
        yj = b * sqrt(1 - xi * xi / asq);
        flr = (int)yj;
        frc = yj = flr;
        plotFour(cx, cy, xi, flr,     1 - frc);
        plotFour(cx, cy, xi, flr + 1, frc);
    }

    ffd = (int)round(bsq / sqrt(bsq + asq));
    for (int yi = 0; yi <= ffd; yi++) {
        xj = a * sqrt(1 - yi * yi / bsq);
        flr = (int)xj;
        frc = xj = flr;
        plotFour(cx, cy, flr,     yi, 1 - frc);
        plotFour(cx, cy, flr + 1, yi, frc);
    }
}

#define MAX_NUM_RIPPLES 4
#define RIPPLE_FRAME_DELAY 0
typedef struct Ripple {
    byte xPos;
    byte yPos;
    byte maxSize;
    float currSize;
    float sizeInc;
};

Ripple ripples[MAX_NUM_RIPPLES];

void ripple() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }

    mulAllPWM(FADE_FACTOR_SLOW, 0);
    for (int i = 0; i < MAX_NUM_RIPPLES; i++) {
        if (ripples[i].maxSize < 4) {
            ripples[i].xPos = random(0, NUM_LED_COLS);
            ripples[i].yPos = random(0, NUM_LED_ROWS);
            ripples[i].maxSize = random(4, 10);
            ripples[i].currSize = 0.0;
            ripples[i].sizeInc = ripples[i].maxSize / (random(20, 30) * 1.0);
        }

        if (ripples[i].currSize > ripples[i].maxSize) {
            ripples[i].maxSize = 0;
        }
        else {
            wuEllipse(ripples[i].xPos, ripples[i].yPos, ripples[i].currSize, ripples[i].currSize);
        }

        ripples[i].currSize += ripples[i].sizeInc;
    }

    writePWMFrame(0, 0);
}

#define MAX_NUM_FIREWORKS 3
#define MIN_FIREWORKS_HEIGHT 2
#define MAX_FIREWORKS_HEIGHT 7

typedef struct Firework {
    byte cHeight;
};

void fireworks() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
    }
}

bool drawAnimeStarburst(int x, int y, int n, int i, byte v) {
    // If we have a spoke of length zero, return without drawing anything.
    if (n <= 0) return false;
    // For a given spoke length n, there are 2*n frames in the starburst animation, and the last one is empty.
    if (i >= 2 * n) return false;
    if (i >= 2 * n - 1) return true;

    // Build up.
    if (i < n) {
        for (byte t = 1; t <= i; t++) {
            smartPlot(x + t,     y, v);
            smartPlot(x - t,     y, v);
            smartPlot(    x, y + t, v);
            smartPlot(    x, y - t, v);
        }

        smartPlot(x, y, v);
    }
    // Decay.
    else {
        int q = 2 * n - i - 1;
        int o = n - 1;
        for (byte t = 0; t < q; t++) {
            smartPlot(x + o - t,         y, v);
            smartPlot(x - o + t,         y, v);
            smartPlot(        x, y - o + t, v);
            smartPlot(        x, y + o - t, v);
        }
    }

    return true;
}

#define STARBURST_SPOKE_LENGTH 3
int counter;
void testStarbursts() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        counter = 0;
    }

    fillPWMFrame(0, EMPTY_PIXEL);
    drawAnimeStarburst(3, 3, STARBURST_SPOKE_LENGTH, counter, SOLID_PIXEL);
    drawAnimeStarburst(20, 3, STARBURST_SPOKE_LENGTH, counter, SOLID_PIXEL);
    writePWMFrame(0, 0);
    counter = (counter + 1) % (STARBURST_SPOKE_LENGTH * 2);
    delay(50);
}

enum AnimeShadeStates {
    BEFORE_FLASH,
    GENERATE_FLASH,
    RIDE_FLASH,
    AFTER_FLASH,
    GRADIENT_FILL,
    STARBURSTS,
    FADE_OUT,
};

#define ANIME_FLASH_WIDTH 5
#define ANIME_BEFORE_FLASH_MS_DELAY 100
#define ANIME_AFTER_FLASH_MS_DELAY 250
#define ANIME_GRADIENT_INCR 51
#define ANIME_GRADIENT_MS_DELAY 10
#define ANIME_STARBURST_SPOKE_LENGTH 3
#define ANIME_MAX_NUM_STARBURSTS 3
#define ANIME_STARBURST_OFFSET_DELAY 4
#define ANIME_STARBURST_MS_DELAY 35
#define ANIME_FADE_OUT_MS_DELAY 10
#define DEG_TO_RAD 0.0174533
AnimeShadeStates animeState;
int animeFlashStep;
byte animeFlashByte;
byte animeFrameCounter;
int animeStarbustStep;
const byte animeStarburstXPoses[ANIME_MAX_NUM_STARBURSTS] = {15, 18, 21};
const byte animeStarburstYPoses[ANIME_MAX_NUM_STARBURSTS] = {3, 2, 4};
bool animeStarburstActivity;
void animeShades() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        animeState = BEFORE_FLASH;
    }

    if (animeState == BEFORE_FLASH) {
        delay(ANIME_BEFORE_FLASH_MS_DELAY);
        animeState = GENERATE_FLASH;
        animeFlashStep = 1;
    }
    else if (animeState == GENERATE_FLASH) {
        animeFlashByte = (~(255 << min(animeFlashStep, ANIME_FLASH_WIDTH))) << max(0, animeFlashStep - ANIME_FLASH_WIDTH);
        if (animeFlashByte != 0) {
            hScrollPWM(0, true, false);
            expandByte(0, animeFlashByte, true, 0);
            writePWMFrame(0, 0);
            animeFlashStep++;
        }
        else {
            animeState = RIDE_FLASH;
            animeFrameCounter = 0;
        }
    }
    else if (animeState == RIDE_FLASH) {
        // We should just need to scroll the pixels NUM_LED_COLS times.
        if (animeFrameCounter < NUM_LED_COLS) {
            hScrollPWM(0, true, false);
            expandByte(0, 0, false, 0);
            writePWMFrame(0, 0);
            animeFrameCounter++;
        }
        else {
            animeState = AFTER_FLASH;
        }
    }
    else if (animeState == AFTER_FLASH) {
        // At this point, all LEDs should be off.
        delay(ANIME_AFTER_FLASH_MS_DELAY);
        animeState = GRADIENT_FILL;
    }
    else if (animeState == GRADIENT_FILL) {
        // At this point, all LEDs should *still* be off.
        if (GlassesPWM[0][NUM_LED_ROWS - 1][0] < SOLID_PIXEL) {
            vScrollPWM(0, true, false);

            for (int x = 0; x < NUM_LED_COLS; x++) {
                GlassesPWM[x][0][0] = min(SOLID_PIXEL, GlassesPWM[x][0][0] + ANIME_GRADIENT_INCR);
            }

            writePWMFrame(0, 0);
            delay(ANIME_GRADIENT_MS_DELAY);
        }
        else {
            animeState = STARBURSTS;
            animeFrameCounter = 0;
            animeStarbustStep = 0;
        }
    }
    else if (animeState == STARBURSTS) {
        // At this point, all LEDs should be at max brightness.
        animeStarburstActivity = false;
        fillPWMFrame(0, EMPTY_PIXEL);
        for (int i = 0; i < ANIME_MAX_NUM_STARBURSTS; i++) {
            animeStarburstActivity |= drawAnimeStarburst(animeStarburstXPoses[i], animeStarburstYPoses[i],
                ANIME_STARBURST_SPOKE_LENGTH, animeStarbustStep - (ANIME_STARBURST_OFFSET_DELAY * i), SOLID_PIXEL);
        }
        invertPWMFrame(0);
        writePWMFrame(0, 0);

        if (animeStarburstActivity) {
            animeStarbustStep++;
            delay(ANIME_STARBURST_MS_DELAY);
        }
        else {
            fillPWMFrame(0, SOLID_PIXEL);
            animeState = FADE_OUT;
        }
    }
    else if (animeState == FADE_OUT) {
        if (GlassesPWM[0][0][0] > EMPTY_PIXEL) {
            mulAllPWM(FADE_FACTOR_MEDI, 0);
            writePWMFrame(0, 0);
            delay(ANIME_FADE_OUT_MS_DELAY);
        }
        else {
            fillPWMFrame(0, EMPTY_PIXEL);
            animeState = BEFORE_FLASH;
        }
    }
}
