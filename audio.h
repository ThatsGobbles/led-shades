// Interface with MSGEQ7 chip for audio analysis

#define AUDIO_DELAY 7

// Pin definitions
#define AUDIO_ANALOG_PIN 1
#define AUDIO_STROBE_PIN 5
#define AUDIO_RESET_PIN A2

// Smooth/average settings
#define AUDIO_SPECTRUM_SMOOTH 0.08
#define AUDIO_PEAK_DELAY 0.01
#define AUDIO_NOISE_FLOOR 60

// AGC settings
#define AUDIO_AGC_SMOOTH 0.005
#define AUDIO_GAIN_UPPER_LIMIT 10.0
#define AUDIO_GAIN_LOWER_LIMIT 0.1

#define NUM_FREQ_BINS 7

// // Code for beat detection, integrate this!
// int spectrumValue[7];
// const int FRAME_WINDOW_LENGTH = 50;
// int currFrame = 0;
// int bass[FRAME_WINDOW_LENGTH];
// int snare[FRAME_WINDOW_LENGTH];
// int hat[FRAME_WINDOW_LENGTH];

// ...

// int find_average(int ary[], int siz){
//   double sum = 0;
//   for (int i = 0; i < siz; i++){
//     sum += ary[i];
//   }
//   return sum/siz;
// }

// void loop(){
//   readEq() //im not including this function here on the forums
//   int new_bass = spectrumValue[0] + spectrumValue[1];
//   int new_snare = spectrumValue[3];
//   int new_hat = spectrumValue[6] + spectrumValue[5];

//   if ((new_snare/find_average(snare, FRAME_WINDOW_LENGTH)) > 1){
//     <stuff>
//   }
//   if ((new_hay/find_average(hat, FRAME_WINDOW_LENGTH)) > 1){
//     <stuff>
//   }
//   if ((new_bass/find_average(bass, FRAME_WINDOW_LENGTH)) > 1){
//     <stuff>
//   }
//   hat[currFrame] = new_hat;
//   snare[currFrame] = new_snare;
//   bass[currFrame] = new_bass;
//   currFrame++;
//   if (currFrame >= FRAME_WINDOW_LENGTH) currFrame=0;
// }

// Global variables
int spectrumValue[NUM_FREQ_BINS];         // holds raw adc values
float spectrumDecay[NUM_FREQ_BINS] = {0}; // holds time-averaged values
float spectrumPeaks[NUM_FREQ_BINS] = {0}; // holds peak values

void doAnalogs() {
    // Static variables
    static float beatAvg = 0.0;
    static float gainAGC = 0.0;
    static PROGMEM const float spectrumFactors[NUM_FREQ_BINS] = {1.0, 1.0, 1.0, 1.0, 1.25, 1.5, 1.75};

    // reset MSQEQ7 to first frequency bin
    digitalWrite(AUDIO_RESET_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(AUDIO_RESET_PIN, LOW);

    // store sum of values for AGC
    int analogsum = 0;

    // cycle through each MSGEQ7 bin and read the analog values
    for (int i = 0; i < NUM_FREQ_BINS; i++) {
        // set up the MSGEQ7
        digitalWrite(AUDIO_STROBE_PIN, LOW);
        delayMicroseconds(40); // to allow the output to settle

        // read the analog value
        spectrumValue[i] = analogRead(AUDIO_ANALOG_PIN);
        digitalWrite(AUDIO_STROBE_PIN, HIGH);

        // noise floor filter
        if (spectrumValue[i] < AUDIO_NOISE_FLOOR) spectrumValue[i] = 0;
        else spectrumValue[i] -= AUDIO_NOISE_FLOOR;

        // apply correction factor per frequency bin
        spectrumValue[i] *= pgm_read_float_near(spectrumFactors + i);

        // prepare average for AGC
        analogsum += spectrumValue[i];

        // apply current gain value
        spectrumValue[i] *= gainAGC;

        // process time-averaged values
        spectrumDecay[i] = (1.0 - AUDIO_SPECTRUM_SMOOTH) * spectrumDecay[i] + AUDIO_SPECTRUM_SMOOTH * spectrumValue[i];

        // process peak values
        if (spectrumPeaks[i] < spectrumDecay[i]) spectrumPeaks[i] = spectrumDecay[i];
        spectrumPeaks[i] = spectrumPeaks[i] * (1.0 - AUDIO_PEAK_DELAY);
    }

  // Calculate audio levels for automatic gain
  beatAvg = (1.0 - AUDIO_AGC_SMOOTH) * beatAvg + AUDIO_AGC_SMOOTH * (analogsum / 7.0);

  // Calculate gain adjustment factor
  gainAGC = 250.0 / beatAvg;
  if (gainAGC > AUDIO_GAIN_UPPER_LIMIT) gainAGC = AUDIO_GAIN_UPPER_LIMIT;
  if (gainAGC < AUDIO_GAIN_LOWER_LIMIT) gainAGC = AUDIO_GAIN_LOWER_LIMIT;

}
