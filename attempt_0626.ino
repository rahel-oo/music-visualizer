#include <arduinoFFT.h>
#include <FastLED.h>

#define MIC_PIN A0           //Input from microphone
#define LED_PIN 2            //Data pin to LEDs
#define NUM_LEDS 60          //Number of LEDs
#define HALF 30              //Half of the LED count
#define BRIGHTNESS 150       //Brightness setting for LED (question of taste)
#define LED_TYPE WS2812B     //LED type of NeoPixel-Ring
#define COLOR_ORDER RGB      //Red Green Blue LED color order
#define SAMPLES 128          //Number of samples that are taken for FFT
#define SAMPLING_FREQ 40000  //Frequency of sampling for FFT

double vReal[SAMPLES];  //vReal gets sampled data and provides transormed data
double vImag[SAMPLES];  //Always 0 (because no complex numbers as input), but needed for instantiation

int Intensity[SAMPLES];

CRGB leds[NUM_LEDS];                                                                //Intantiate all LEDs of the NeoPixel-Ring
ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQ);  //Create object for frequency measurement

void setup() {
  pinMode(MIC_PIN, INPUT);  //Define mic input pin
  Serial.begin(115200);
  delay(3000);  //Power-up safety delay

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);  //Add LEDs to FastLED
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  doVisualizing();
}

void doVisualizing() {
  collectSamplesAndStore();

  defineDisplay();

  FastLED.show();

  memset(Intensity, 0, sizeof Intensity);  //Reset for next loop
}

//Collect and compute samples for FFT, define the Intensity depending on the frequency
void collectSamplesAndStore() {
  //For all samples, store value from mic in vReal[i]
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0;  //Always 0, because of real input (no complex numbers)
  }

  //Use samples in Fast Fourrier Transformation
  FFT.windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, SAMPLES);

  for (int i = 0; i < SAMPLES; i++) {
    Serial.println(vReal[i]);
  }
      cleanUpvReal();


  //Update Intensity Array
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = constrain(vReal[i], 100, 1023);     // Set max value for input data
    vReal[i] = map(vReal[i], 100, 1023, 0, HALF);  // Map data to fit display

    Intensity[i] = vReal[i];
  }
}


void defineDisplay() {
  int color = 0;
  leds[0] = CHSV(0, 255, BRIGHTNESS);  //Lowest LED is always on

  for (int i = 1; i < HALF + 1; i++) {
    if (i <= Intensity[i]) {
      leds[i] = CHSV(color, 255, BRIGHTNESS);
      leds[HALF * 2 - i] = CHSV(color, 255, BRIGHTNESS);
    } else {  // Everything outside the range goes dark
      leds[i] = CHSV(color, 255, 0);
      leds[HALF * 2 - i] = CHSV(color, 255, 0);
    }
    color += 255 / 10;
  }
}


//Remove unnecessary low and high first two values from vReal
void cleanUpvReal() {
  for (int i = 0; i < SAMPLES - 2; i++) {
    vReal[i] = vReal[i + 2];
  }
}
