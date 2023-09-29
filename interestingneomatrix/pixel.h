#include "hues.h"
#include "gamma.h"

#define nFlickers 3
const uint8_t flickerIntensities[nFlickers] = { 255, 120, 70 };
const uint8_t flickerDurations[nFlickers] = { 2, 3, 5 }; // relative
const uint8_t totalFlickerDuration = 10;

uint16_t nonChanges = 0;
uint32_t lastChange;

class Pixel {
  public:
    Pixel() {
      this->nextHue = random();
      this->steadyEnd = millis();
    };
    virtual ~Pixel() {};
    CRGB getColor(uint32_t t) {
      // gamma correction is last: http://renderwonk.com/blog/index.php/archive/adventures-with-gamma-correct-rendering/
      CRGB raw = this->getRawColor(t);
      return CONFIG.CORRECT_GAMMA ? gamma32(raw) : raw;
    }
    uint16_t index;
    uint8_t animation = 0;

  protected:
    // 4 states:
    // * steady: trans < t < steady
    // * transstart: trans < steady < t (increase trans)
    // * trans: steady < t < trans
    // * transend: steady < trans < t (increase steady => back to first state)
    uint32_t transitionEnd = 0;
    uint32_t steadyEnd = 0;

    #ifdef FASTLED
    byte hue;
    byte nextHue;
    #else
    uint16_t nextHue;
    uint16_t hue;
    #endif

    CRGB getRawColor(uint32_t t) {
      if (t >= this->steadyEnd && t >= this->transitionEnd) {
        // state change between transition <-> steady. which one?

        if (this->steadyEnd > this->transitionEnd) {
          // transition start
          // TODO could even disallow transitioning altogether and stay non-transitioning
          for (byte i = 0; i < 2; i++) {
            if (this->shouldTransition(i)) {
              if (i == 0) {
//                LOG.printf("%u -> %u on %uth attempt (after %u ms, %u attempts/s)\n", this->animation, i, nonChanges, millis() - lastChange, 1000*nonChanges/(millis() - lastChange));
                lastChange = millis();
                nonChanges = 0;
              } else {
                nonChanges++;
              }
              this->animation = i;
              this->hue = this->nextHue;
              this->nextHue = randomHue(this->hue, this->animation);

              uint16_t transitionDuration = CONFIG.ANIMATION[this->animation].TRANSITION_DURATION
                + (CONFIG.ANIMATION[this->animation].TRANSITION_EXTRA > 0 ? random(CONFIG.ANIMATION[this->animation].TRANSITION_EXTRA+1) : 0);
              if (CONFIG.ANIMATION[this->animation].PACE_TRANSITIONS) {
//                Serial.printf("%u %u %u %u ", this->hue, this->nextHue,  (this->hue < this->nextHue ? this->nextHue - this->hue : this->hue - this->nextHue), transitionDuration);
                transitionDuration = (this->hue < this->nextHue ? this->nextHue - this->hue : this->hue - this->nextHue) * transitionDuration / 127;
//                Serial.println(transitionDuration);
              }

              // transitioning styles can decide how to handle the OFF_TIME internally
              this->transitionEnd = this->steadyEnd + transitionDuration + CONFIG.ANIMATION[this->animation].OFF_TIME;
              break;
            }
          }

        } else {
          // transition end: we've arrived at the next steady state
          this->steadyEnd = this->transitionEnd + CONFIG.ANIMATION[this->animation].ON_TIME + (CONFIG.ANIMATION[this->animation].ON_EXTRA > 0 ? random(CONFIG.ANIMATION[this->animation].ON_EXTRA+1) : 0);
        }
      }

      if (this->transitionEnd < t && t < this->steadyEnd) {
        // drift?
//        if (CONFIG.ANIMATION[this->animation].HUE_DRIFT > 0) {
//            this->nextHue = this->nextHue + random(-CONFIG.ANIMATION[this->animation].HUE_DRIFT, CONFIG.ANIMATION[this->animation].HUE_DRIFT+1);
//        }
        return this->effectiveColor(this->nextHue);
      }

      // transitioning
      uint16_t transitionDuration = this->transitionEnd - this->steadyEnd;
      uint16_t intoTransition = t - this->steadyEnd;

      switch (CONFIG.ANIMATION[this->animation].PIXEL_TYPE) {
        case FadeToBlack:
          return 0; // TODO implement
        case Flicker:
          return this->getFlickering(t);

        case Radial:
        case Linear:
          // blending
            // drift
//            Serial.print('d');
          // scale transition time to [0,255] -- TODO use sine transform instead of linear?
          // just add OFF_TIME to transition duration
          byte progress = 255 * intoTransition / transitionDuration;
          return (CONFIG.ANIMATION[this->animation].PIXEL_TYPE == Radial) ? this->getRadialBlend(progress) : this->getLinearBlend(progress);
      }
      return this->effectiveColor(this->nextHue);
    }

    CRGB getRadialBlend(byte progress) {
      // linearly interpolate hue
      // FIXME this breaks with bytes because of overflow
      return this->effectiveColor(((255-progress)*this->hue + progress*this->nextHue) / 255);
    }
    CRGB getLinearBlend(byte progress) {
      // weighted average of the *colors*
      CRGB col1 = this->effectiveColor(this->hue);
      CRGB col2 = this->effectiveColor(this->nextHue);
      return col1.lerp8(col2, progress);
    }
    CRGB getFlickering(uint32_t t) {
      uint8_t value = CONFIG.VALUE;
      uint32_t intoFlicker = t - this->steadyEnd;
      uint16_t animationLength = this->transitionEnd - this->steadyEnd - CONFIG.ANIMATION[this->animation].OFF_TIME;

      if (intoFlicker > animationLength) {
        // flicker is wrapping up/fading into next hues -- this is limited by OFF_TIME
        uint16_t postFlicker = intoFlicker - animationLength;
        // use a FULL cycle for fade-in, but no more than .8s (too slow a pickup)
        int16_t fadeIn = constrain(animationLength / nFlickers, 1, 800); // FIXME non-0 fadeIn avoids a division by zero in sinus fade map() below?
        int16_t offOff = constrain(CONFIG.ANIMATION[this->animation].OFF_TIME - fadeIn, 0, CONFIG.ANIMATION[this->animation].OFF_TIME);
//        Serial.printf("%u + %u = %u\n", fadeIn, offOff, CONFIG.ANIMATION[this->animation].OFF_TIME);
        if (postFlicker <= offOff) {
          value = 0;
        } else {
          // #1: sinus fade to value
//          value = ( fadeInOut(map(postFlicker - offOff, 0, fadeIn, 0, 127)) * CONFIG.VALUE ) >> 8;
          // #2: linear fade
          value = CONFIG.VALUE * (postFlicker - offOff) / fadeIn;
          // #3: TODO could fade in faster by abruptly stopping cos-transition at value?
          return this->effectiveColor(this->nextHue, value);
        }
      } else {
        // during flicker
        // length of the current flicker (start with #0)
        uint16_t flickerLength = animationLength * flickerDurations[0] / totalFlickerDuration;

        // how far into the first cosine should we jump to start off smoothly from the original brightness value?
        float skippedStart = HALF_PI - acos(CONFIG.VALUE / 255.0);
        // that's in cycles, now map onto ms
        uint16_t flickerStart = skippedStart * flickerLength / HALF_PI;
        intoFlicker += flickerStart;

        // figure out which flicker we're in
        uint8_t cycle = 0;
        while (intoFlicker >= flickerStart + flickerLength) {
          if (++cycle == nFlickers) {
            // avoid array overflow
            return 0;
          }
          flickerStart += flickerLength;
          flickerLength = animationLength * flickerDurations[cycle] / totalFlickerDuration;
        }
//        Serial.printf("%u/%u %u: %u %u\n", intoFlicker, animationLength, cycle, flickerStart, flickerLength);

        uint16_t progress = map(intoFlicker - flickerStart, 0, flickerLength, 0, 255);
        uint16_t intensity = fadeInOut(progress)*flickerIntensities[cycle];
        if (CONFIG.ANIMATION[this->animation].HUE_DRIFT) {
          // make sure noise doesn't lead to over- or underflow
          int16_t noise = random(-CONFIG.ANIMATION[this->animation].HUE_DRIFT, CONFIG.ANIMATION[this->animation].HUE_DRIFT+1) << 8;
          intensity += constrain(noise, -intensity, 65535 - intensity);
        }
        value = intensity >> 8;
      }
      return this->effectiveColor(this->hue, value);
    }

    uint16_t effectiveHue(uint16_t hue) {
      #ifdef FASTLED
      return CONFIG.HUE_OFFSET + hue << (8 - CONFIG.HUE_BITS);
      #else
      return CONFIG.HUE_OFFSET + hue << (16 - CONFIG.HUE_BITS);
      #endif
    }
    CRGB effectiveColor(uint16_t hue, uint8_t value = CONFIG.VALUE) {
      // https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-use
      return CHSV(this->effectiveHue(hue), CONFIG.SATURATION, value);
    }
    boolean shouldTransition(byte animation) {
      return (transitionPs[animation] == PROBABILITY_BASE) || (random(PROBABILITY_BASE) <= transitionPs[animation]);
    }
    uint32_t getNewStartTime(uint32_t t) {
      return t + CONFIG.ANIMATION[this->animation].TRANSITION_DURATION;
    }
    uint32_t getNewEndTime(uint32_t t) {
      return t + CONFIG.ANIMATION[this->animation].TRANSITION_DURATION + CONFIG.ANIMATION[this->animation].ON_TIME + 
        (CONFIG.ANIMATION[this->animation].ON_EXTRA > 0 ? random(CONFIG.ANIMATION[this->animation].ON_EXTRA+1) : 0);
    }
};

/*class SyncedPixel : public Pixel {
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
        this->hue = randomHue(this->hue);
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
        this->hue = randomHue(this->hue);
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
*/

/*class BlendingPixel : public Pixel {
  protected:
    uint32_t startTime;
    #ifdef FASTLED
    byte nextHue;
    #else
    uint16_t nextHue;
    #endif
    uint32_t endTime;
    virtual CRGB getBlendedColor(byte progress);
    virtual void init() override {
      Pixel::init();
      this->nextHue = random();
    }
  public:
    CRGB getRawColor(uint32_t t) {
      if (t >= this->endTime && this->shouldTransition()) {
        this->startTime = this->getNewStartTime(t);
        this->endTime = this->getNewEndTime(t);
        this->hue = this->nextHue;
        this->nextHue = randomHue(this->hue);
      } else {
        // drift
        this->nextHue = this->nextHue + random(-CONFIG.ANIMATION[this->animation].HUE_DRIFT, CONFIG.ANIMATION[this->animation].HUE_DRIFT+1);
      }
      // scale transition time to [0,255] -- TODO use sine transform instead of linear
      byte progress = (t < this->startTime) ? 255 - (255 * (this->startTime - t)) / CONFIG.ANIMATION[this->animation].TRANSITION_DURATION : 255;
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
      return col1.lerp8(col2, progress);

      // TODO bitwise each
//      uint32_t col;
//      for (byte i = 0; i < 3; i++) {
//        col1[i] = ((255-progress)*col1[i] + progress*col2[i]) / 255;
//        col |= ((((255-progress)*((col1 >> 8*i) & 0xFF) + progress*((col2 >> 8*i) & 0xFF))) / 255) << 8*i;
//      }
//      return col1;
    }
};
/*
 * 
 */
// dying
//class FlickeringPixel : public UnsyncedPixel {


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
