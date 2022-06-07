class Pixel {
  public:
    Pixel() {
      this->hue = transitionP;
    };
    virtual ~Pixel() {};
    CRGB getColor(uint32_t t) {
      // gamma correction is last: http://renderwonk.com/blog/index.php/archive/adventures-with-gamma-correct-rendering/
//      return CONFIG.CORRECT_GAMMA ? matrix.gamma32(getRawColor(t)) : getRawColor(t);
      return getRawColor(t);
    }
    uint16_t index;
  protected:
    #ifdef FASTLED
    byte hue;
    #else
    uint16_t hue;
    #endif
    virtual CRGB getRawColor(uint32_t t);
    uint16_t effectiveHue(uint16_t hue) {
      #ifdef FASTLED
      return CONFIG.HUE_OFFSET + hue << (8 - CONFIG.HUE_BITS);
      #else
      return CONFIG.HUE_OFFSET + hue << (16 - CONFIG.HUE_BITS);
      #endif
    }
    CRGB effectiveColor(uint16_t hue) {
      // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
      return CHSV(this->effectiveHue(hue), CONFIG.SATURATION, CONFIG.VALUE);
    }
    boolean shouldChange(uint32_t p = transitionP) {
      return (p == PROBABILITY_BASE) || (random(PROBABILITY_BASE) <= p);
    }
    uint32_t getNewStartTime(uint32_t t) {
      return t + CONFIG.TRANSITION_DURATION;
    }
    uint32_t getNewEndTime(uint32_t t) {
      return t + CONFIG.TRANSITION_DURATION +
        (CONFIG.RANDOM_INTERVAL ? random(min(CONFIG.ON_MIN, CONFIG.ON_MAX), max(CONFIG.ON_MIN, CONFIG.ON_MAX)) : CONFIG.ON_MIN);
    }
};

class SyncedPixel : public Pixel {
  protected:
    static uint32_t timeout;
    static uint16_t nSynced;
  public:
    SyncedPixel() {
      this->timeout = millis();
    }
    CRGB getRawColor(uint32_t t) {
      if (t >= this->timeout) {// + CONFIG.OFF_TIME) {
        this->timeout = this->getNewEndTime(t);
        this->nSynced = 0;
      } else if (t >= this->timeout) {
        return 0;
      }
      if (this->nSynced != nLit) {
        this->hue = random();
        this->nSynced++;
      }
      return this->effectiveColor(this->hue);
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
    CRGB getRawColor(uint32_t t) {
      if (t >= this->timeout) {// + CONFIG.OFF_TIME) {
        this->hue = random();
        this->timeout = this->getNewEndTime(t);
      }
//      if (t >= this->timeout) {
//        return 0;
//      } else {
        // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
        return this->effectiveColor(this->hue);
//      }
    }
};

class BlendingPixel : public Pixel {
  protected:
    uint32_t startTime;
    #ifdef FASTLED
    byte nextHue;
    #else
    uint16_t nextHue;
    #endif
    uint32_t endTime;
    virtual CRGB getBlendedColor(byte progress);
  public:
    CRGB getRawColor(uint32_t t) {
      if (t >= this->endTime && this->shouldChange()) {
        this->startTime = this->getNewStartTime(t);
        this->endTime = this->getNewEndTime(t);
        this->hue = this->nextHue;
        this->nextHue = random();
      }
      // scale transition time to [0,255] -- TODO use sine transform instead of linear
      byte progress = (t < this->startTime) ? 255 - (255 * (this->startTime - t)) / CONFIG.TRANSITION_DURATION : 255;
      return this->getBlendedColor(progress);
    }
};

// pure hues
class RadialBlendingPixel : public BlendingPixel {
  protected:
    CRGB getBlendedColor(byte progress) {
      // linearly interpolate hue
      // FIXME this breaks with bytes because of overflow
      return this->effectiveColor(((255-progress)*this->hue + progress*this->nextHue) / 255);
    }
};

// peachy
class LinearBlendingPixel : public BlendingPixel {
  protected:
    CRGB getBlendedColor(byte progress) {
      // weighted average of the *colors*
      CRGB col1 = this->effectiveColor(this->hue);
      CRGB col2 = this->effectiveColor(this->nextHue);
      // TODO bitwise each
//      uint32_t col;
      for (byte i = 0; i < 3; i++) {
        col1[i] = ((255-progress)*col1[i] + progress*col2[i]) / 255;
//        col |= ((((255-progress)*((col1 >> 8*i) & 0xFF) + progress*((col2 >> 8*i) & 0xFF))) / 255) << 8*i;
      }
      return col1;
    }
};

// TODO implement permanently shifting type

// steadystate can be: fixed or wandering (permanent hue-walk, in either direction?)

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
