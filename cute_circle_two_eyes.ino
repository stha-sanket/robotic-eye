#include <U8g2lib.h>

// SH1106 1.3" 128x64 I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

/* -------------------------------------------------------------
   Eye geometry (centered on the 128x64 screen)
   Plain filled rounded-rectangle eyes, no iris/pupil.
   ------------------------------------------------------------- */
#define LEFT_EYE_X    40          // center x of left eye
#define RIGHT_EYE_X   88          // center x of right eye
#define EYE_Y         32          // vertical center (same for both)
#define EYE_W         26          // full width of each eye
#define EYE_H         34          // full height of each eye (open state)
#define EYE_RADIUS    10          // corner rounding

/* -------------------------------------------------------------
   Blink timing
   ------------------------------------------------------------- */
unsigned long prevBlink = 0;
const unsigned long BLINK_INTERVAL = 3200;   // avg time between blinks (ms)
const unsigned long BLINK_DURATION = 180;    // full close+open cycle (ms)

bool blinking = false;
unsigned long blinkStart = 0;

/* -------------------------------------------------------------
   Easing function: smooth in/out instead of linear motion
   ------------------------------------------------------------- */
float easeInOutQuad(float t) {
  if (t < 0.5f) return 2.0f * t * t;
  return 1.0f - pow(-2.0f * t + 2.0f, 2) / 2.0f;
}

/* -------------------------------------------------------------
   Draw one eye as a filled rounded rectangle.
   lidFrac: 1.0 = fully open, 0.0 = fully closed
   ------------------------------------------------------------- */
void drawEye(int cx, float lidFrac) {
  // Height shrinks toward a thin closed sliver; width stays constant.
  int h = (int)(EYE_H * lidFrac);
  if (h < 4) h = 4;                 // keep a visible closed-lid line

  int x = cx - EYE_W / 2;
  int y = EYE_Y - h / 2;

  // Clamp corner radius so it never exceeds half of width/height
  int r = EYE_RADIUS;
  if (r > h / 2) r = h / 2;
  if (r > EYE_W / 2) r = EYE_W / 2;
  if (r < 1) r = 1;

  u8g2.drawRBox(x, y, EYE_W, h, r);
}

/* ------------------------------------------------------------- */
void setup() {
  u8g2.begin();
  randomSeed(analogRead(0));
}

/* ------------------------------------------------------------- */
void loop() {
  unsigned long now = millis();

  /* ---------- BLINK CONTROL ---------- */
  if (!blinking && now - prevBlink >= BLINK_INTERVAL + random(-800, 1200)) {
    blinking = true;
    blinkStart = now;
    prevBlink = now;
  }

  float lid = 1.0f;  // 1 = fully open, 0 = fully closed

  if (blinking) {
    float prog = (float)(now - blinkStart) / BLINK_DURATION;
    if (prog >= 1.0f) {
      blinking = false;
      lid = 1.0f;
    } else {
      // triangle wave 0->1->0 over the blink, eased for smoothness
      float tri = (prog < 0.5f) ? (prog * 2.0f) : (2.0f - prog * 2.0f);
      lid = easeInOutQuad(tri);
    }
  }

  /* ---------- DRAW ---------- */
  u8g2.clearBuffer();
  drawEye(LEFT_EYE_X,  lid);
  drawEye(RIGHT_EYE_X, lid);
  u8g2.sendBuffer();

  delay(10);
}
