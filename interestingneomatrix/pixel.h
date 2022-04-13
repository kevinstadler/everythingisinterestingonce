#define MIN_HUE_DISTANCE 3640 // ~20 degrees
uint16_t pseudoRandomHue(uint16_t lastHue) {
  uint16_t n = lastHue + MIN_HUE_DISTANCE + random(0, 65535 - 2*MIN_HUE_DISTANCE);
  Serial.printf("%u > %u\n", lastHue, n);
  return n;
}

class Pixel {
  public:
    virtual ~Pixel() {};
    virtual uint32_t getColor(uint32_t t);
    uint16_t index;
  protected:
    uint16_t hue;
};

enum PixelType { Synced, Unsynced, Shifting, Adding };

class SyncedPixel : public Pixel {
  protected:
    static uint32_t timeout;
    static uint16_t nSynced;
  public:
    uint32_t getColor(uint32_t t) {
      if (t >= this->timeout + OFF_TIME) {
        this->timeout = t + (RANDOM_INTERVAL ? random(ON_MIN, ON_MAX) : ON_MIN);
        this->nSynced = 0;
      } else if (t >= this->timeout) {
        return 0;
      }
      if (nSynced != nLit) {
        this->hue = random();
        this->nSynced++;
      }
      // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
      uint32_t col = matrix.ColorHSV(HUE_OFFSET + this->hue << (16 - HUE_BITS), SATURATION, VALUE);
      return CORRECT_GAMMA ? matrix.gamma32(col) : col;
    }
};

uint32_t SyncedPixel::timeout;
uint16_t SyncedPixel::nSynced;

class UnsyncedPixel : public Pixel {
  protected:
    uint32_t timeout;
  public:
    UnsyncedPixel() {
      this->timeout = millis();
    }
    uint32_t getColor(uint32_t t) {
      if (t >= this->timeout + OFF_TIME) {
        this->hue = random();
        this->timeout = t + (RANDOM_INTERVAL ? random(ON_MIN, ON_MAX) : ON_MIN);
      }
//      if (t >= this->timeout) {
//        return 0;
//      } else {
        // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
        uint32_t col = matrix.ColorHSV(HUE_OFFSET + this->hue << (16 - HUE_BITS), SATURATION, VALUE);
        return CORRECT_GAMMA ? matrix.gamma32(col) : col;
//      }
    }
};

class BlendingPixel : public UnsyncedPixel {
  protected:
    uint32_t startTime;
    uint16_t nextHue;
    uint32_t nextTime;
    virtual uint32_t getBlendedColor(byte progress);
  public:
    uint32_t getColor(uint32_t t) {
    if (t >= this->nextTime) {
      this->startTime = t;
      this->nextTime = t + (RANDOM_INTERVAL ? random(ON_MIN, ON_MAX) : ON_MIN);
      this->hue = this->nextHue;
      this->nextHue = random();
    }
    // scale transition time to [0,255]
    byte progress = (255 * (t - this->startTime)) / (this->nextTime - this->startTime);
    // TODO use sine transform
    return this->getBlendedColor(progress);
  }
};

class ShiftingPixel : public BlendingPixel {
  protected:
    uint32_t getBlendedColor(byte progress) {
      // linearly interpolate hue
      uint32_t col = matrix.ColorHSV(HUE_OFFSET + ((255-progress)*this->hue + progress*this->nextHue) / 255, SATURATION, VALUE);
      return CORRECT_GAMMA ? matrix.gamma32(col) : col;
    }
};

class AddingPixel : public BlendingPixel {
  protected:
    uint32_t getBlendedColor(byte progress) {
      // weighted average of the *colors*
      uint32_t col1 = matrix.ColorHSV(HUE_OFFSET + this->hue, SATURATION, VALUE);
      uint32_t col2 = matrix.ColorHSV(HUE_OFFSET + this->nextHue, SATURATION, VALUE);
      // TODO bitwise each
      uint32_t col;
      for (byte i = 0; i < 3; i++) {
        col |= ((((255-progress)*((col1 >> 8*i) & 0xFF) + progress*((col2 >> 8*i) & 0xFF))) / 255) << 8*i;
      }
      return col;
    }
};

/*
      if (FADE_IN) {
        // fade in -- linearly interpolate from 0 up to VALUE
        while (millis() < t + FADE_IN) {
          draw((VALUE * (millis() - t)) / FADE_IN);
          WAIT();
        }
      }
      if (FADE_OUT) {
        // fade out -- linearly interpolate from VALUE down to 0
        t += FADE_OUT;
        while (millis() < t) {
          draw((VALUE * (t - millis())) / FADE_OUT);
          WAIT();
        }
      }
    
      if (OFF_TIME) {
        draw(0);
        t += OFF_TIME;
        while (millis() < t) {
          WAIT();
        }
      }

 */
