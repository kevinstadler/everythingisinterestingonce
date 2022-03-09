// inspired by WS2812 / WS2812b Led Matrix Diffuser by malebuffy (https://www.thingiverse.com/thing:3373935)

// lessons from printing:
// * brim is a pain to remove, stick to skirt
// * ABS: stringy/messy, at 240deg a 200mm space shrunk down to 197.5mm (1+% shrinkage)
// * PLA: 200mm shrunk down to 199mm (<.5% shrinkage)

LEDS_HORIZONTAL = 8;
LEDS_VERTICAL = 8;
// all measurements in mm
OUTER_WALL_THICKNESS = 1;
WALL_THICKNESS = 1;
DIFFUSER_HEIGHT = 3;

// basic spacing
UNIT_SIZE = 10.1; // add 1% to counter shrinkage
LED_DISTANCE = 4.9; // this determines the length of the spacers

// extra spacer and resistor cutout
SPACER_WIDTH = 1;
SPACER_HEIGHT = 1;

RESISTOR_HEIGHT = 1;
RESISTOR_LENGTH = 2.5;
RESISTOR_OFFSET = 1; // DOWN from center of led

// some epsilon that's < WALL_THICKNESS
eps = WALL_THICKNESS / 2;

CUTOUT_SIZE = UNIT_SIZE - WALL_THICKNESS;

difference() {
    translate([-OUTER_WALL_THICKNESS, -OUTER_WALL_THICKNESS, 0])
    cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS - WALL_THICKNESS, LEDS_VERTICAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS - WALL_THICKNESS, DIFFUSER_HEIGHT]);
    
    for (y = [0:LEDS_VERTICAL-1]) {
        // resistor cutouts
        translate([-OUTER_WALL_THICKNESS-eps, y*UNIT_SIZE + CUTOUT_SIZE/2 + RESISTOR_OFFSET - RESISTOR_LENGTH/2, -eps])
        cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS + 2*eps, RESISTOR_LENGTH, RESISTOR_HEIGHT+eps]);

        // led cutouts
        translate([0, y*UNIT_SIZE, -eps])
        for (x = [0:LEDS_HORIZONTAL-1]) {
            translate([x*UNIT_SIZE, 0, 0])
            cube([CUTOUT_SIZE, CUTOUT_SIZE, DIFFUSER_HEIGHT + 2*eps]);
        }
    }
}

// spacers
for (x = [0:LEDS_HORIZONTAL-1]) {
    for (y = [0:LEDS_VERTICAL]) {
        translate([x*UNIT_SIZE + CUTOUT_SIZE/2 - SPACER_WIDTH/2, y*UNIT_SIZE - LED_DISTANCE / 2 - WALL_THICKNESS / 2, 0])
        cube([SPACER_WIDTH, LED_DISTANCE, SPACER_HEIGHT]);
    }
}
