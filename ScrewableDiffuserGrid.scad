$fn = 20;
module led_diffuser_grid (n_leds, grid_height=3, led_interval=10, led_size=4.9, wall_thickness=.4, strengthened_wall_thickness=.8, outer_wall_thickness=1.2, resistor_height=1, resistor_length=2.5, resistor_offset=.5, attachment_every=7, attachment_thickness=2) {

    assert(resistor_height < grid_height, "Resistor cutout is taller than the actual grid, this is probably not what you want?");

    // main grid
    difference() {
        // to middle of thingie
        translate([-led_interval/2, (n_leds[1]-1)*led_interval/2, 0]) {
            union() {
                // horizontal lines
                for (y = [0:n_leds[1]]) {
                    translate([n_leds[0]*led_interval/2, (y-n_leds[1]/2)*led_interval, grid_height/2]) cube([n_leds[0]*led_interval + outer_wall_thickness, (y % n_leds[1] == 0) ? outer_wall_thickness : wall_thickness, grid_height], center=true);
                }
                // vertical lines
                for (x = [0:n_leds[0]]) {
                    translate([x*led_interval, 0, grid_height/2]) cube([(x % n_leds[0] == 0) ? outer_wall_thickness : wall_thickness, n_leds[1]*led_interval+wall_thickness, grid_height], center=true);
                }

                // partially strengthened vertical lines at attachment positions (only up to attachment_thickness)
                for (x = [floor((n_leds[0]-1) % attachment_every/2)+1:attachment_every:n_leds[0]-1]) {
                    // old (from floor)
//                    translate([x*led_interval, 0, attachment_thickness/2])
                    // new (from front, for inverted printing)
                    translate([x*led_interval, 0, grid_height/2])
                    cube([strengthened_wall_thickness, n_leds[1]*led_interval+strengthened_wall_thickness, grid_height], center=true);
                    //translate([x*led_interval, 0, grid_height-attachment_thickness/2])
                    //                    cube([strengthened_wall_thickness, n_leds[1]*led_interval+strengthened_wall_thickness, attachment_thickness], center=true);
                }
            }
        }

        // resistor cutouts
        for (y = [0:n_leds[1]-1]) {
            translate([-n_leds[0]*led_interval, y*led_interval + resistor_height/2 + resistor_offset - resistor_length/2, -resistor_height])
            cube([2*n_leds[0]*led_interval, resistor_length, 2*resistor_height]);
        }

    }
    
    // screw attachments
    for (y = [0, 1]) {
        for (x = [floor((n_leds[0]-1) % attachment_every/2)+0:attachment_every:n_leds[0]-1]) {
            toouteredge = (x+.5)*led_interval;
            
            translate([toouteredge, sign(y-.5)*(led_interval + 8)/2 + y*(n_leds[1]-1)*led_interval, attachment_thickness/2]) {
                if (grid_height >= 3) {
                    // cable tugging thing
                    tugwidth = 3;
                    for (x = [-8, 8]) {
                        translate([x, -2*sign(y-.5), -attachment_thickness/2+3/2])
                            difference() {
                                cube([tugwidth, 3, 3], center=true);
                                translate([0, -1*sign(y-.5), -1]) cube([tugwidth+1, 3, 3], center=true);
                            }
    //                    rotate([0, 90, 0]) difference() {
    //                        cylinder(tugwidth, r=4, center=true);
    //                        cylinder(tugwidth+1, r=3, center=true);
    //                        translate([1, -4, -5]) cube([tugwidth, 10, 10]);
    //                    }
                    }
                }

                // screw hole
                difference() {
                    union() {
                        translate([0, sign(.1-y)*2, 0]) cube([8, 4, attachment_thickness], true);
                        cylinder(attachment_thickness, r=4, center=true);
                    }
                    // smol screws have 5.7mm heads, radius=3 hole still too smol
                    cylinder(attachment_thickness+.01, 1.5, 3.3, center=true);
                }
            }
        }
    }
}

rotate([180, 0, 0]) {
// for kossel: 18x8
// other good: 25x8
led_diffuser_grid([18, 8], 3);
//led_diffuser_grid([25, 8], 5);
/*
translate([15, 95, 0]) led_diffuser_grid([32, 8], 5);
translate([0, 190, 0]) led_diffuser_grid([32, 8], 7);
translate([15, 285, 0]) led_diffuser_grid([32, 8], 9);
*/

}