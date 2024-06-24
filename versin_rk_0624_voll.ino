#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>
#include <FastLED.h>

#define MIC_PIN A0
#define LED_PIN 2  // Data pin to LEDS
#define NUM_LEDS 60
#define BRIGHTNESS 90  // LED information
#define LED_TYPE WS2812B
#define COLOR_ORDER RGB
#define HALF 30
#define SAMPLES 128
#define SAMPLING_FREQ 40000

double vReal[SAMPLES];  //vReal gets sampled data and provides transormed data
double vImag[SAMPLES];  //Always 0, but needed for instantiation

int Intensity[HALF] = {};  //Intensity setting for first half of LEDs

CRGB leds[NUM_LEDS];                                                                //object containing all leds
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);  //create object for frequency measurement


void setup() {
  pinMode(MIC_PIN, INPUT);  // mic input pin
  Serial.begin(115200);
  delay(3000);  // power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  doVisualizing();
}

void doVisualizing() {
  collectSamplesAndStore();

  defineDisplay();

  FastLED.show();

  memset(Intensity, 0, sizeof Intensity);  //reset for next loop
}

void collectSamplesAndStore() {
  //for all samples, store value from mic in vReal[i]
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(MIC_PIN);
    Serial.println("vReal");   //**********remove
    Serial.println(vReal[i]);  //**********remove
    vImag[i] = 0;              //always 0, because of real input
  }

  //use samples in Fast Fourrier Transformation
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);
  cleanUpvReal();  //*** check if needed or could be done differently, some unusually high numbers result from FFT for some parts

  

  int index = 0;
  for (int i = 0; i < SAMPLES - 8; i += 4) {
    vReal[i] = constrain(vReal[i], 0, 511);          // set max value for input data
    vReal[i + 1] = constrain(vReal[i + 1], 0, 511);  // set max value for input data
    vReal[i + 2] = constrain(vReal[i + 2], 0, 511);  // set max value for input data
    vReal[i + 3] = constrain(vReal[i + 3], 0, 511);  // set max value for input data

    vReal[i] = map(vReal[i], 0, 511, 0, HALF);
    vReal[i + 1] = map(vReal[i + 1], 0, 511, 0, HALF);
    vReal[i + 2] = map(vReal[i + 2], 0, 511, 0, HALF);
    vReal[i + 3] = map(vReal[i + 3], 0, 511, 0, HALF);


    int avg = (vReal[i] + vReal[i + 1] + vReal[i + 2] + vReal[i + 3]);
    avg = (int)avg / 4;
    Intensity[index] = avg;
    index += 1;
  }
}

void defineDisplay() {
  int color = 0;
  leds[0] = CHSV(0, 255, BRIGHTNESS);  //lowest LED is always on
  
  int max = 0;
  for (int i = 1; i < HALF; i++) {
    if (Intensity[i] > max) {
      max = Intensity[i];
 
    }
  }
  for (int i = 1; i < HALF+1; i++){
  if (i <= max) {
    leds[i] = CHSV(color, 255, BRIGHTNESS);
    leds[HALF * 2 - i] = CHSV(color, 255, BRIGHTNESS);
  } else {  // Everything outside the range goes dark
    leds[i] = CHSV(color, 255, 0);
    leds[HALF * 2 - i] = CHSV(color, 255, 0);
  }
  color += 255 / 10;  //question of taste
  }
}

//remove unnecessary low and high first two values from vReal
void cleanUpvReal() {
  int i;
  for (i = 0; i < SAMPLES - 2; i++) {
    vReal[i] = vReal[i + 2];
  }
}


//void showPeakLeds(int ledCount) {
//  if (ledCount < HALF - 2) {
//    leds[ledCount + 2] = CHSV(10, 255, BRIGHTNESS);
//    leds[NUM_LEDS - (ledCount + 2)] = CHSV(10, 255, BRIGHTNESS);
//  } else {
//    leds[HALF] = CHSV(10, 255, BRIGHTNESS);
//  }
//}
