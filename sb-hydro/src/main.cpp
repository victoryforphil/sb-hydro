/* ----------------------------------------------------------------------
"Pixel dust" Protomatter library example. As written, this is
SPECIFICALLY FOR THE ADAFRUIT MATRIXPORTAL M4 with 64x32 pixel matrix.
Change "HEIGHT" below for 64x64 matrix. Could also be adapted to other
Protomatter-capable boards with an attached LIS3DH accelerometer.

PLEASE SEE THE "simple" EXAMPLE FOR AN INTRODUCTORY SKETCH,
or "doublebuffer" for animation basics.
------------------------------------------------------------------------- */

#include <Wire.h> // For I2C communication

#include <Adafruit_PixelDust.h> // For sand simulation
#include <FastLED.h>

#define WIDTH 8    // Matrix height (pixels) - SET TO 64 FOR 64x64 MATRIX!
#define HEIGHT 32  // Matrix width (pixels)
#define MAX_FPS 45 // Maximum redraw rate, frames/second

#define LED_PIN 3
#define COLOR_ORDER GRB
#define CHIPSET WS2812
#define BRIGHTNESS 30

#define N_GRAINS 50 // Number of grains of sand

Adafruit_PixelDust sand(WIDTH, HEIGHT, N_GRAINS, 1, 160, true);

const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = true;

#define NUM_LEDS (WIDTH * HEIGHT)
CRGB leds[NUM_LEDS];

uint8_t pixelBuf[WIDTH * HEIGHT];

uint32_t prevTime = 0;  // Used for frames-per-second throttle
uint8_t backbuffer = 0; // Index for double-buffered animation
// SETUP - RUNS ONCE AT PROGRAM START --------------------------------------
uint16_t XY(dimension_t x, dimension_t y)
{
  uint16_t i;

  // LED index is linear for serpintine layout, bottom to top
  if (y % 2 == 0)
  {
    i = x + (y * WIDTH);
  }
  else
  {
    i = (WIDTH - x) + (y * WIDTH);
  }

  return i;
}

void err(int x)
{
  Serial.println("Couldn't start sand");
  int i;
  pinMode(LED_BUILTIN, OUTPUT); // Using onboard LED
  for (i = 1;; i++)
  {                                   // Loop forever...
    digitalWrite(LED_BUILTIN, i & 1); // LED on/off blink to alert user
    delay(x);
  }
}

void setup(void)
{
  Serial.begin(9600);
  while (!Serial)
    delay(5);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  if (!sand.begin())
  {
    Serial.println("Couldn't start sand");
    err(1000); // Slow blink = malloc error
  }
  Serial.println("Starting owo");

  /*
    if (!accel.begin(0x19))
  {
    Serial.println("Couldn't find accelerometer");
    err(250); // Fast bink = I2C error
  }
   accel.setRange(LIS3DH_RANGE_4_G); // 2, 4, 8 or 16 G!
  */
  uint8_t pixelBuf[WIDTH * HEIGHT];

  memset(pixelBuf, 0, sizeof(pixelBuf)); // Clear pixel buffer

  for (int i = 0; i < WIDTH; i++)
  {
    sand.setPixel(i, -1);
  }

  sand.randomize();
  sand.iterate(0, -100000, 0);
}

// MAIN LOOP - RUNS ONCE PER FRAME OF ANIMATION ----------------------------
long test = 0;
void loop()
{
  // Limit the animation frame rate to MAX_FPS.  Because the subsequent sand
  // calculations are non-deterministic (don't always take the same amount
  // of time, depending on their current states), this helps ensure that
  // things like gravity appear constant in the simulation.
  uint32_t t;
  while (((t = micros()) - prevTime) < (1000000L / MAX_FPS))
    ;
  prevTime = t;

  backbuffer = 1 - backbuffer; // Swap front/back buffer index

  // Erase old grain positions in pixelBuf[]
  uint8_t i;
  dimension_t x, y;
  for (i = 0; i < N_GRAINS; i++)
  {
    sand.getPosition(i, &x, &y);

    leds[XY(x, y)] = CRGB::Black;
  }

  // Run one frame of the simulation
  // X & Y axes are flipped around here to match physical mounting

  sand.iterate(0, -3000, 0);
  test += 5;
  // Draw new grain positions in pixelBuf[]
  for (i = 0; i < N_GRAINS; i++)
  {
    sand.getPosition(i, &x, &y);

    leds[XY(x, y)] = CRGB::Blue;
  }

  // Set LEDS from pixel buff.

  FastLED.show();
}