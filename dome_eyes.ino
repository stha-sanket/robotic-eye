#include <U8g2lib.h>

/* -------------------------------------------------------------
   TWO displays, ONE eye each.
   Left eye  -> hardware I2C  (A4=SDA, A5=SCL)
   Right eye -> software I2C  (D2=SDA, D3=SCL)
   Both are SH1106 128x64, address 0x3C by default — fine here
   since they're on physically separate buses.
   ------------------------------------------------------------- */
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2_left(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2_right(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* reset=*/ U8X8_PIN_NONE);

/* -------------------------------------------------------------
   Eye geometry — each screen shows ONE capsule/pill-shaped eye,
   centered. Both the top AND bottom are fully rounded caps,
   connected by a short straight section — a true capsule shape.
   ------------------------------------------------------------- */
#define EYE_CX        64          // horizontal center (screen is 128 wide)
#define EYE_CY        32          // vertical center
#define EYE_W         46          // width of the eye
#define EYE_H         60          // height of the eye (fully open) — nearly fills the 64px screen height

/* -------------------------------------------------------------
   Blink timing (shared by both eyes so they blink in sync)
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
   Draw one capsule/pill-shaped eye.
   lidFrac: 1.0 = fully open, 0.0 = fully closed
   Width stays constant; height shrinks to blink. Both the top
   and bottom caps stay fully rounded — never just one dome.
   ------------------------------------------------------------- */
void drawEye(U8G2 &display, float lidFrac) {
  int h = (int)(EYE_H * lidFrac);
  if (h < 6) h = 6;                 // keep a visible closed-lid capsule

  int x = EYE_CX - EYE_W / 2;
  int y = EYE_CY - h / 2;

  int r = (min(EYE_W, h) / 2) - 1;  // fully rounded top AND bottom caps
  if (r < 1) r = 1;

  display.drawRBox(x, y, EYE_W, h, r);
}

/* ------------------------------------------------------------- */
void setup() {
  u8g2_left.begin();
  u8g2_right.begin();
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

  float lid = 1.0f;

  if (blinking) {
    float prog = (float)(now - blinkStart) / BLINK_DURATION;
    if (prog >= 1.0f) {
      blinking = false;
      lid = 1.0f;
    } else {
      float tri = (prog < 0.5f) ? (prog * 2.0f) : (2.0f - prog * 2.0f);
      lid = easeInOutQuad(tri);
    }
  }

  /* ---------- DRAW BOTH EYES ---------- */
  u8g2_left.clearBuffer();
  drawEye(u8g2_left, lid);
  u8g2_left.sendBuffer();

  u8g2_right.clearBuffer();
  drawEye(u8g2_right, lid);
  u8g2_right.sendBuffer();

  delay(10);
}
