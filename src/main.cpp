#include <FastLED.h>

#define ANALOG_PHOTO_INPUT A7
#define PHOTO_INPUT 2
#define RELAY_OUTPUT 3
#define LED_PIN 4
#define NUM_LEDS 6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 20
#define BRIGHTNESS  255
#define FLASH_COUNT 5

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
bool targetHit = false;
bool flashing = false;
int flashCount = 0;
int cycleCount = 0;
bool offTime = false;

void SetupPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);

    currentPalette[15] = CRGB::White;
    currentPalette[0] = CRGB::White;
    currentPalette[1] = CRGB::White;
    
    currentPalette[3] = CRGB::Red;
    currentPalette[4] = CRGB::Red;
    currentPalette[5] = CRGB::Red;
    
    currentPalette[7] = CRGB::Blue;
    currentPalette[8] = CRGB::Blue;
    currentPalette[9] = CRGB::Blue;
    
    currentPalette[11] = CRGB::Green;
    currentPalette[12] = CRGB::Green;
    currentPalette[13] = CRGB::Green;
}


void setup()
{
  SetupPalette();
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  pinMode(PHOTO_INPUT, INPUT);   
  pinMode(RELAY_OUTPUT, OUTPUT);  
  digitalWrite(RELAY_OUTPUT, LOW);
  Serial.begin(9600);
  delay(1000);
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    for( int i = 0; i < NUM_LEDS; ++i) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, BRIGHTNESS, LINEARBLEND);
        colorIndex += 3;
    }
}


void loop()
{
  static uint8_t startIndex = 0;

  if(flashing)
  {
    FillLEDsFromPaletteColors( startIndex );
    flashCount++;

    // Go to white
    if(startIndex == 0 && flashCount > 2)
    {
      startIndex = 64;
      flashCount = 0;
    }
    // Go to black
    else if(startIndex == 64 && flashCount > 2)
    {
      startIndex = 0;
      flashCount = 0;
      cycleCount++;
    }

    // After 5 flashes turn off relay and disable target
    if(cycleCount > FLASH_COUNT)
    {
      offTime = true;
      cycleCount = 0;
      flashing = false;
      startIndex = 0;
      fill_solid( currentPalette, 16, CRGB::Black);
      FillLEDsFromPaletteColors( startIndex );
      digitalWrite(RELAY_OUTPUT, LOW);
    }
  }

  else if(offTime) // disable targetr for 3 seconds
  {
    //Serial.println(digitalRead(PHOTO_INPUT));
    cycleCount++;
    if(cycleCount > UPDATES_PER_SECOND * 3)
    {
      offTime = false;
      cycleCount = 0;
      startIndex = 0;
      SetupPalette(); // Go back to color palette
    }
  }

  else //Watch for target hit
  {
    startIndex = startIndex + 1; 
    FillLEDsFromPaletteColors( startIndex );

    if(digitalRead(PHOTO_INPUT) == LOW)
    {
      targetHit = true;
    }

    // Wait for target to be released from hit
    if(digitalRead(PHOTO_INPUT) == HIGH && 
      targetHit == true)
    {
      targetHit = false;
      flashing = true;
      digitalWrite(RELAY_OUTPUT, HIGH);
      startIndex = 0;
      cycleCount = 0;
    }
  }

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);

}