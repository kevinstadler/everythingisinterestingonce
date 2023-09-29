//#define MIN_HUE_DISTANCE 14 //3640 // ~20 degrees

uint8_t randomHue(uint16_t lastHue = 0, uint8_t animation = 0) {
  return lastHue + CONFIG.ANIMATION[animation].DHUE_MIN + random(0, 255 - 2*CONFIG.ANIMATION[animation].DHUE_MIN);
//  Serial.printf("%u > %u\n", lastHue, n);
}


// there are 3*sum hues available
uint8_t sum = 4;

// when I have time to overengineer it, could create a 16 bit number
// which stores values up to the least common multiple (7560 for 30 hues)
// that would allow to change the number of hues without having to reset/recalculate them
// https://www.calculatorsoup.com/calculators/math/lcm.php

//byte randomHue() {
//  return random(0, 3*sum);
//}


uint8_t shiftHue(byte currentHue) {
  return (currentHue + random()) % (3*sum);
}

uint8_t fadeInOut(byte progress) {
  return cos8(127 + progress);
}
