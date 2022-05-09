// inspired by WS2812 / WS2812b Led Matrix Diffuser by malebuffy (https://www.thingiverse.com/thing:3373935)

// lessons from printing:
// * brim is a pain to remove, stick to skirt
// * ABS: stringy/messy, at 240deg a 200mm space shrunk down to 197.5mm (1+% shrinkage)
// * PLA: 200mm shrunk down to 199mm (<.5% shrinkage), but very good spacer fit (with 5mm LED distance)
// * PLA#2: 10.1 unit size (taken off while still hot) too large
// * EcoPLA @ 210/60 on Ender 5

LEDS_HORIZONTAL = 16;
LEDS_VERTICAL = 8;
// all measurements in mm
WALL_THICKNESS = .6;
OUTER_WALL_THICKNESS = 1;
GRID_HEIGHT = 3;
AIRGAP_HEIGHT = 4;
// 0 for a closed grid, 1 to leave one side jagged open
OPEN_SIDE = 1;

// basic spacing
// 10.05 on Ender was a bit too large
UNIT_SIZE = 10.03; // add 1% to counter shrinkage
LED_DISTANCE = 4.95; // this determines the length of the spacers
LED_SIZE = UNIT_SIZE - LED_DISTANCE;

// extra spacer and resistor cutout
SPACER_WIDTH = 1;
SPACER_HEIGHT = 1;
// 2 or more for skipping spacers every other row/column
SPACER_EVERY = 2;

RESISTOR_HEIGHT = 1;
RESISTOR_LENGTH = 2.5;
RESISTOR_OFFSET = 1; // DOWN from center of led

OUTER_SPACER_LENGTH = 5;
// that's the height of the finger that sticks out to the side, at the end of it is a wall that's GRID_HEIGHT+AIRGAP_HEIGHT tall
OUTER_SPACER_HEIGHT = 1;
OUTER_SPACER_WIDTH = 3;

// some epsilon that's < WALL_THICKNESS
eps = WALL_THICKNESS / 2;

CUTOUT_SIZE = UNIT_SIZE - WALL_THICKNESS;

// basic grid
difference() {
    translate([-OUTER_WALL_THICKNESS, -OUTER_WALL_THICKNESS, 0])
    cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS - WALL_THICKNESS - OPEN_SIDE*(OUTER_WALL_THICKNESS+.01), LEDS_VERTICAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS - WALL_THICKNESS, GRID_HEIGHT]);
    
    for (y = [0:LEDS_VERTICAL-1]) {
        // resistor cutouts
        translate([-OUTER_WALL_THICKNESS-eps, y*UNIT_SIZE + CUTOUT_SIZE/2 + RESISTOR_OFFSET - RESISTOR_LENGTH/2, -eps])
        cube([LEDS_HORIZONTAL*UNIT_SIZE + 2*OUTER_WALL_THICKNESS + 2*eps, RESISTOR_LENGTH, RESISTOR_HEIGHT+eps]);

        // led cutouts
        translate([0, y*UNIT_SIZE, -eps])
        for (x = [0:LEDS_HORIZONTAL-1]) {
            translate([x*UNIT_SIZE, 0, 0])
            cube([CUTOUT_SIZE, CUTOUT_SIZE, GRID_HEIGHT + 2*eps]);
        }
    }
}

// LED spacers
for (x = [0:SPACER_EVERY:LEDS_HORIZONTAL-1]) {
    difference() {
        translate([x*UNIT_SIZE + CUTOUT_SIZE/2 - SPACER_WIDTH/2, -eps, 0])
        cube([SPACER_WIDTH, LEDS_VERTICAL*UNIT_SIZE, SPACER_HEIGHT]);
        for (y = [0:LEDS_VERTICAL-1]) {
            translate([x*UNIT_SIZE + LED_DISTANCE/2, y*UNIT_SIZE + LED_DISTANCE/2 - WALL_THICKNESS/2, -eps])
            cube([LED_SIZE, LED_SIZE, SPACER_HEIGHT + 2*eps]);
        }
    }
}

// TODO move spacers outward
// air gap spacers (every 4 cells at the top+bottom of the outer wall)
for (y_offset = [0, LEDS_VERTICAL*UNIT_SIZE + OUTER_WALL_THICKNESS - WALL_THICKNESS]) {
    for (x_offset = [0:4:(LEDS_HORIZONTAL - OPEN_SIDE)]) {
        translate([x_offset * UNIT_SIZE - OUTER_WALL_THICKNESS, y_offset - OUTER_WALL_THICKNESS, GRID_HEIGHT - eps])
        // make it twice as long so it's actually stable enough
        cube([OUTER_SPACER_LENGTH, OUTER_WALL_THICKNESS, AIRGAP_HEIGHT + eps]);
    }
}

// render: 4x 26, 1x 24