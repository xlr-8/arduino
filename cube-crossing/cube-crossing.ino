// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 144 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DEBUG false
#define SLOWLINESS 50 // Time (in milliseconds) to pause between pixels
#define CROSSINGS_QTY 6
#define CROSSINGS_MAX_SIZE 2
#define WORMS_QTY 4
#define WORMS_BODY 6

#define BACKWARD 0
#define FORWARD 1

struct Crossing {
  int index;
  int values[CROSSINGS_MAX_SIZE];
};

struct Worm {
  int index;
  int body[WORMS_BODY];
  int direction;
};

Crossing crossings[CROSSINGS_QTY] = {
  {4, {4, 81}},
  {81, {4, 81}},
  {11, {11, 51}},
  {51, {51, 11}},
  {40, {40, 90}},
  {90, {40, 90}},
};  // You can set this to whatever size you need

Worm WORMS[WORMS_QTY]; /* = {
  {0, FORWARD},
  {50, BACKWARD},
  {100, FORWARD},
};*/

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  Serial.begin(9600);//set the serial communication baudrate as 9600

  pixels.clear(); // Set all pixel colors to 'off'
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  // Print the contents of the crossings array
  if (DEBUG) {
    printCrossings();
  } 

  // Add worms to the strip paths
  for (int i = 0; i < WORMS_QTY; i++) {
    WORMS[i].index = random(0, NUMPIXELS); // SHOULD BE THE MAX LENGTH
    WORMS[i].direction = random(BACKWARD, FORWARD);  // SHOULD BE RANDOM(FWD, BCD)
    WORMS[i].body[0] = WORMS[i].index;
  }
}

void clear_strip(int d) {
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();   // Send the updated pixel colors to the hardware.
  delay(d); // Pause before next pass through loop
}

// Function to print all the crossings data
void printCrossings() {  
  for (int i = 0; i < CROSSINGS_QTY; i++) {
    Serial.print("Crossing ");
    Serial.print(crossings[i].index);
    Serial.print(": [");
    for (int j = 0; j < CROSSINGS_MAX_SIZE; j++) {
      Serial.print(crossings[i].values[j]);
      if (j < 3) Serial.print(", ");
    }
    Serial.println("]");
  }
}

void move(Worm *w) {
  int crossingIdx = -1;
 
  if (DEBUG) {
    Serial.println("=========== BEGIN ==========");
    Serial.print("Worm moves: ");
    Serial.println(w->direction);
    Serial.print("Worm position: ");
    Serial.println(w->index);
  }

  for (int i = 0; i < CROSSINGS_QTY; i++) {
    if (w->index == crossings[i].index) {
      crossingIdx = i;
      break;
    }
  }

  // A crossing has been found, so we need to get a random LED to jump on
  if (crossingIdx != -1) {
    int ridx = random(0, CROSSINGS_MAX_SIZE);
    
    if (DEBUG) {
      Serial.print("Crossing IDX: ");
      Serial.println(crossingIdx);
      Serial.print("RIDX: ");
      Serial.println(ridx);
      Serial.print("Crossing index picked: ");
      Serial.println(crossings[crossingIdx].values[ridx]);
    }

    w->index = crossings[crossingIdx].values[ridx];

    // If we reached a crossing we can go change direction either way
    // else we continue on the one we were on
    if (crossings[crossingIdx].index != w->index) {
      w->direction = random(BACKWARD, FORWARD);
    }
  }

  // Move forward the worm if direction, else backward
  if (w->direction == BACKWARD) {
    w->index = w->index - 1;
  } else {
    w->index = w->index + 1;  
  }

  // We reached the end so circle back one way or the other
  if (w->index == NUMPIXELS) {
    w->index = 0;
  } else if (w->index < 0) {
    w->index = NUMPIXELS;
  }

  for (int i=WORMS_BODY - 1; i >= 0; i--) {
    if (i == 0) {
      w->body[i] = w->index;
    } else {
      w->body[i] = w->body[i-1];
    }

    if (DEBUG) {
      Serial.println("=========== END ==========");
      Serial.print("Worm moves: ");
      Serial.println(w->direction);
      Serial.print("Worm position: ");
      Serial.println(w->index);
    }
  }
}

void loop() {
  clear_strip(100);

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  while(true) {
    pixels.clear(); // Set all pixel colors to 'off'

    for (int w=0; w < WORMS_QTY; w++) {
      move(&WORMS[w]);

      for (int i=0; i < WORMS_BODY; i++) {
        pixels.setPixelColor(WORMS[w].body[i], pixels.Color(150, 0, 0));
      }
    }

    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(SLOWLINESS); // Pause before next pass through loop
  }
}
