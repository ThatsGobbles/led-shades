// Graphical pattern functions for glasses.

void expandByte(byte col, byte value) {
    for (byte i = 0; i < 8; i++) {
        GlassesPWM[col][i][0] = value & 0b10000000 ? SOLID_PIXEL : EMPTY_PIXEL;
        value <<= 1;
    }
}

void expandByteRev(byte col, byte value) {
    for (byte i = 0; i < 8; i++) {
        GlassesPWM[col][i][0] = value & 0b00000001 ? SOLID_PIXEL : EMPTY_PIXEL;
        value >>= 1;
    }
}

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
        expandByte(i, 3 << (int)(sin(i / 2.0 + sinesIncRval) * 3.5 + 3.5));
    }

    writePWMFrame(0);

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

    writePWMFrame(0);
    plasOffset += 15;
    if (plasOffset > 359) plasOffset -= 359;
}

#define FADE_FACTOR_SLOW 0.9
#define FADE_FACTOR_MEDI 0.8
#define FADE_FACTOR_FAST 0.7
void mulAllPWM(float q, byte frame) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            GlassesPWM[x][y][frame] *= q;
        }
    }
}

void addAllPWM(float q, byte frame) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        for (int y = 0; y < NUM_LED_ROWS; y++) {
            GlassesPWM[x][y][frame] += q;
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

        writePWMFrame(0);
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

    mulAllPWM(FADE_FACTOR_SLOW, 0);
    for (int i = 0; i < SPARKLE_COUNT; i++) GlassesPWM[random(0, NUM_LED_COLS)][random(0, NUM_LED_ROWS)][0] = SOLID_PIXEL;
    writePWMFrame(0);
}

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

        writePWMFrame(0);
    }
}

void loadGraphicsFrame(int frame) {
    for (int x = 0; x < NUM_LED_COLS; x++) {
        expandByteRev(x, pgm_read_byte(BeatingHeartFrames[frame]+x));
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

    mulAllPWM(FADE_FACTOR_SLOW, 0);
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

    writePWMFrame(0);
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

enum AnimeShadeStates {
    BEFORE_FLASH,
    GENERATE_FLASH,
    RIDE_FLASH,
    AFTER_FLASH,
    GRADIENT_FILL,
    SPARKLE,
};

#define ANIME_FLASH_WIDTH 5
#define ANIME_BEFORE_FLASH_DELAY 0
#define ANIME_AFTER_FLASH_DELAY 250
#define ANIME_GRADIENT_INCR 51
#define ANIME_GRADIENT_DELAY 10
#define ANIME_MAX_NUM_SPARKLE_TWIRLS 4
#define ANIME_SPARKLE_ORIGIN_X 20
#define ANIME_SPARKLE_ORIGIN_Y 2
#define ANIME_SPARKLE_RADIUS 3.5
#define DEG_TO_RAD 0.0174533
AnimeShadeStates state;
int flashStep;
byte flashByte;
byte animeScrollCountdown;
int animeSparkleTheta;
float x0a, y0a, x1a, y1a;   // First sparkle spoke
float x0b, y0b, x1b, y1b;   // Second sparkle spoke
float cosTheta, sinTheta;
void animeShades() {
    if (!patternInit) {
        switchDrawType(0, 1);
        patternInit = true;
        state = BEFORE_FLASH;
    }

    if (state == BEFORE_FLASH) {
        delay(ANIME_BEFORE_FLASH_DELAY);
        state = GENERATE_FLASH;
        flashStep = 1;
    }
    else if (state == GENERATE_FLASH) {
        flashByte = (~(255 << min(flashStep, ANIME_FLASH_WIDTH))) << max(0, flashStep - ANIME_FLASH_WIDTH);
        if (flashByte != 0) {
            hScrollPWM(0, true, false);
            expandByteRev(0, flashByte);
            writePWMFrame(0);
            flashStep++;
        }
        else {
            state = RIDE_FLASH;
            animeScrollCountdown = NUM_LED_COLS;
        }
    }
    else if (state == RIDE_FLASH) {
        // We should just need to scroll the pixels NUM_LED_COLS times.
        if (animeScrollCountdown > 0) {
            hScrollPWM(0, true, false);
            expandByte(0, 0);
            writePWMFrame(0);
            animeScrollCountdown--;
        }
        else {
            state = AFTER_FLASH;
        }
    }
    else if (state == AFTER_FLASH) {
        // At this point, all LEDs should be off.
        delay(ANIME_AFTER_FLASH_DELAY);
        state = GRADIENT_FILL;
    }
    else if (state == GRADIENT_FILL) {
        // At this point, all LEDs should *still* be off.
        if (GlassesPWM[0][NUM_LED_ROWS - 1][0] < SOLID_PIXEL) {
            vScrollPWM(0, true, false);

            for (int x = 0; x < NUM_LED_COLS; x++) {
                GlassesPWM[x][0][0] = min(SOLID_PIXEL, GlassesPWM[x][0][0] + ANIME_GRADIENT_INCR);
            }

            writePWMFrame(0);
            delay(ANIME_GRADIENT_DELAY);
        }
        else {
            state = SPARKLE;
            animeSparkleTheta = 0;
        }
    }
    else if (state == SPARKLE) {
        // At this point, all LEDs should be at max brightness.

        if (animeSparkleTheta / 360 <= ANIME_MAX_NUM_SPARKLE_TWIRLS) {
            // Will need to set frame to all off, draw sparkle, and invert.
            fillPWMFrame(0, EMPTY_PIXEL);
            cosTheta = cos(animeSparkleTheta * DEG_TO_RAD);
            sinTheta = sin(animeSparkleTheta * DEG_TO_RAD);
            x0a =  ANIME_SPARKLE_RADIUS * sinTheta + ANIME_SPARKLE_ORIGIN_X;
            y0a = -ANIME_SPARKLE_RADIUS * cosTheta + ANIME_SPARKLE_ORIGIN_Y;
            x1a = -ANIME_SPARKLE_RADIUS * sinTheta + ANIME_SPARKLE_ORIGIN_X;
            y1a =  ANIME_SPARKLE_RADIUS * cosTheta + ANIME_SPARKLE_ORIGIN_Y;
            x0b = -ANIME_SPARKLE_RADIUS * cosTheta + ANIME_SPARKLE_ORIGIN_X;
            y0b = -ANIME_SPARKLE_RADIUS * sinTheta + ANIME_SPARKLE_ORIGIN_Y;
            x1b =  ANIME_SPARKLE_RADIUS * cosTheta + ANIME_SPARKLE_ORIGIN_X;
            y1b =  ANIME_SPARKLE_RADIUS * sinTheta + ANIME_SPARKLE_ORIGIN_Y;

            wuLine(x0a, y0a, x1a, y1a);
            wuLine(x0b, y0b, x1b, y1b);
            wuEllipse(ANIME_SPARKLE_ORIGIN_X, ANIME_SPARKLE_ORIGIN_Y, 2, 2);

            invertPWMFrame(0);

            writePWMFrame(0);

            animeSparkleTheta += 25;
            delay(5);
        }
        else {
            fillPWMFrame(0, EMPTY_PIXEL);
            state = BEFORE_FLASH;
        }
    }
}
