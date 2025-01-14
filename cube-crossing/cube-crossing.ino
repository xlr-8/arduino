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

// Whether or not you want some debugging displayed to console
// Please note that depending on the amount of debug this can
// slow down the execution of your program considerably
#define DEBUG false

/*
** Definitions of constant & structure used for the "worms" mode
*/
#define SLOWLINESS 50         // Time (in milliseconds) to pause between pixels
#define CROSSINGS_QTY 6       // Quantity of crossing on the strip LED
#define CROSSINGS_MAX_SIZE 2  // Max size of the crossing - amount of direction possibles
#define WORMS_QTY 4           // Amount of worms desired on the strip LED
#define WORMS_BODY 6          // Size of the worms moving around in LED

// Direction in which the worms move
#define BACKWARD 0
#define FORWARD 1

// Crossing indicated on which LED the worm can switch directions
struct Crossing {
  int index;                        // Position of the crossing
  int values[CROSSINGS_MAX_SIZE];   // Possible direction to take
};

// Worm represents a single worm: position, size & direction
struct Worm {
  int index;                        // Position of the head of the worm
  int body[WORMS_BODY];             // Size of the worms body to move around
  int direction;                    // Direction which the worm is taking
};

// Create & define the amount of crossing possible for worms
Crossing crossings[CROSSINGS_QTY] = {
  {4, {4, 81}},
  {81, {4, 81}},
  {11, {11, 51}},
  {51, {51, 11}},
  {40, {40, 90}},
  {90, {40, 90}},
};

// Create the amount of worms desired
Worm WORMS[WORMS_QTY];

//
void setupWorms() {
  // Print the contents of the crossings array
  if (DEBUG) {
    printCrossings();
  }

  // Worms are randomerly added onto to the strip LED
  for (int i = 0; i < WORMS_QTY; i++) {
    // If not set with +1 only BACKWARD will be selected
    WORMS[i].direction = random(BACKWARD, FORWARD + 1);
    WORMS[i].index = random(0, NUMPIXELS);
    WORMS[i].body[0] = WORMS[i].index;
  }
}

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

  setupWorms();
}

// Helper to clear out the strip LED
void clear_strip(int d) {
  pixels.clear();
  pixels.show();
}

// Debgugging function to print all the crossings data instanciated
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

/*
** GLOW MODE
** This mode will simply make the strip LED glow by randomely selecting
** the intensity of the brightness and the delay. Thus making it glow
** slightly randomely both in terms of intensity and speed.
*/
void glow() {
  pixels.setBrightness(1); // Set BRIGHTNESS to about 1/5 (max = 255)
  pixels.fill(pixels.Color(150, 0, 0), 0, NUMPIXELS);
  pixels.show();   // Send the updated pixel colors to the hardware.

  int max = random(35, 125);
  int wait = random(10, 50);

  if (DEBUG) {
    Serial.print("target: ");
    Serial.println(max);
    Serial.print("Wait: ");
    Serial.println(wait);
  }

  // Start making the strip LED brighter
  for (int b=1; b < max; b++) {
    pixels.setBrightness(b);
    pixels.show();
    delay(SLOWLINESS+wait);
  }

  // Start making the strip LED darker
  for (int b=max; b >= 0; b--) {
    pixels.setBrightness(b);
    pixels.show();
    delay(SLOWLINESS+wait);
  }

  delay(2000);
}

/*
** WORMS MODE
** This mode will make the "worms" start moving around the strip led
** when meeting a crossing of LED, they will be able to move towards
** either direction by switching LED and/or direction randomly
*/
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
      w->direction = random(BACKWARD, FORWARD+1);
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

void worms() {
  while(true) {
    pixels.clear();

    for (int w=0; w < WORMS_QTY; w++) {
      move(&WORMS[w]);

      for (int i=0; i < WORMS_BODY; i++) {
        pixels.setPixelColor(WORMS[w].body[i], pixels.Color(150, 0, 0));
      }
    }

    pixels.show();
    delay(SLOWLINESS);
  }
}

/*
** Start the loop based on the mode selected
*/
void loop() {
  int mode = 1;

  if (mode == 1) {
    glow();
  } else {
    worms();
  }
}
