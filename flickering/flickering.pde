void setup() {
  size(600, 600);
  colorMode(HSB, 1.0);  
  noStroke();
}

int msPerCycle = 1200;
float intensities[] = { 1.0, .7, .3 }; 
float noise[] = { .08, .1, .1 };
int loopLength = noise.length * msPerCycle;

int cycle = 0;
float hue = 0;

void draw() {
  background(0);
  int newCycle = millis() / loopLength;
  if (newCycle > cycle) {
    cycle = newCycle;
    hue = random(1.0);
  }
  int progress = millis() % loopLength;
  int i = progress / msPerCycle;
  // use intensities[i]
  float intensity = foo(progress % msPerCycle, intensities[i]);
  fill(hue, 1.0, intensity + random(-noise[i], noise[i])); //*randomGaussian());
  ellipse(width/2, height/2, height/5, height/5);
}

// step is out of msPerCycle
float foo(float step, float mx) {
  return map(cos(PI+step*TWO_PI/msPerCycle), -1, 1, 0, mx);
}
