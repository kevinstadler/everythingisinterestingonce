// inspired by WS2812 / WS2812b Led Matrix Diffuser by malebuffy (https://www.thingiverse.com/thing:3373935)

LEDS_HORIZONTAL = 64;
LEDS_VERTICAL = 8;
// all measurements in mm
OUTER_WALL_THICKNESS = 2;
WALL_THICKNESS = 1;
DIFFUSER_HEIGHT = 3;

// basic spacing
LED_SIZE = 5;
LED_DISTANCE = 5;

// extra spacer and resistor cutout
SPACER_WIDTH = 1;
SPACER_HEIGHT = 1;
RESISTOR_HEIGHT = 1;
RESISTOR_LENGTH = 2;
//RESISTOR_OFFSET = 0; // from center of led

// some epsilon that's < WALL_THICKNESS
eps = .5;

UNIT_SIZE = LED_SIZE + LED_DISTANCE;
CUTOUT_SIZE = UNIT_SIZE - WALL_THICKNESS;
SPACER_LENGTH = LED_DISTANCE / 2;

difference() {
    translate([-OUTER_WALL_THICKNESS, -OUTER_WALL_THICKNESS, 0])
    cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS, LEDS_VERTICAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS, DIFFUSER_HEIGHT]);
    
    for (y = [0:LEDS_VERTICAL-1]) {
        // resistor cutouts
        translate([eps, y*UNIT_SIZE + CUTOUT_SIZE/2 - RESISTOR_LENGTH/2, -eps])
        cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS - 2*eps, RESISTOR_LENGTH, RESISTOR_HEIGHT+eps]);

        // led cutouts
        translate([0, y*UNIT_SIZE, 0])
        for (x = [0:LEDS_HORIZONTAL-1]) {
            translate([x*UNIT_SIZE, 0, -eps])
            cube([CUTOUT_SIZE, CUTOUT_SIZE, DIFFUSER_HEIGHT + 2*eps]);
        }
    }
}

// spacers
for (x = [0:LEDS_HORIZONTAL-1]) {
    translate([x*UNIT_SIZE, 0, -eps])
    for (y = [0:LEDS_VERTICAL-1]) {
        translate([CUTOUT_SIZE/2 - SPACER_WIDTH/2, y*UNIT_SIZE + CUTOUT_SIZE-SPACER_LENGTH, eps])
        cube([SPACER_WIDTH, SPACER_LENGTH + eps, SPACER_HEIGHT]);
    }
}
