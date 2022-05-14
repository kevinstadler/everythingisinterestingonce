// lessons from printing:
// * brim is a pain to remove, stick to skirt

// module parameters:
// * lr/bt_spacers is the distance that the spacer will have *from the theoretical led grid border* (which is actually inside the outer wall!)
// negative spacer values shave off the outer wall (plus the given number of mm off the inner grid) -- generally advise *against* this because loose outer frame edges can cause detachment/warping..
// * outer_spacers is [length/breadth, width/thickness, height/thickness]
module led_diffuser_grid (n_leds, grid_height=3, airgap_height=4, lr_spacers=[4, 4], tb_spacers=[16, 25], outer_spacers=[4, 3, 1], outer_spacer_every=10, led_interval=10, led_size=4.8, led_spacer_width=1, led_spacer_height=1, led_spacer_every=2, wall_thickness=.6, outer_wall_thickness=.6, resistor_height=1, resistor_length=2.5, resistor_offset=.6) {

    assert(resistor_height < grid_height, "Resistor cutout is taller than the actual grid, this is probably not what you want?");

    // main grid
    difference() {

        translate([(n_leds[0]-1)*led_interval/2, (n_leds[1]-1)*led_interval/2, grid_height/2]) // to middle of thingie
        cube([n_leds[0] * led_interval + 2*outer_wall_thickness - wall_thickness,
            n_leds[1] * led_interval + 2*outer_wall_thickness - wall_thickness, grid_height], true);

        for (y = [0:n_leds[1]-1]) {

            // led cutout
            for (x = [0:n_leds[0]-1]) {
                translate([x*led_interval, y*led_interval, grid_height/2])
                cube([led_interval - wall_thickness, led_interval - wall_thickness, 2*grid_height], true);
            }

            // resistor cutout
            translate([-n_leds[0]*led_interval, y*led_interval + resistor_height/2 + resistor_offset - resistor_length/2, -resistor_height])
            cube([2*n_leds[0]*led_interval, resistor_length, 2*resistor_height]);

        }
        
        // if spacers = negative, shave off outer wall by that amount
        for (x = [0, 1]) {
            if (lr_spacers[x] < 0) {
                translate([x*n_leds[0]*led_interval - led_interval/2, 0, 0])
                cube([outer_wall_thickness - lr_spacers[x], 2*n_leds[1]*led_interval, 3*grid_height], true);
            }
        }
        for (y = [0, 1]) {
            if (tb_spacers[y] < 0) {
                translate([0, y*n_leds[1]*led_interval - led_interval/2, 0])
                cube([2*n_leds[0]*led_interval, outer_wall_thickness - tb_spacers[y], 3*grid_height], true);
            }
        }
    }
    // main grid finished
    
    // led spacers
    led_spacer_length = (led_interval - led_size)/2;
    for (x = [0:n_leds[0]-1]) {
        for (y = [0:n_leds[1]-1]) {
            if ((x + y) % led_spacer_every == 0) {
                // lower
                translate([x*led_interval, (y-.5)*led_interval+led_spacer_length/2, led_spacer_height/2])
                cube([led_spacer_width, led_spacer_length, led_spacer_height], true);
                // upper
                translate([x*led_interval, (y+.5)*led_interval-led_spacer_length/2, led_spacer_height/2])
                cube([led_spacer_width, led_spacer_length, led_spacer_height], true);
            }
        }
    }
    
    // outer spacers: left/right
    for (x = [0, 1]) {
        if (lr_spacers[x] > 0) {
            for (y = [floor((n_leds[1]-1)%outer_spacer_every/2):outer_spacer_every:n_leds[1]-1]) {
                toouteredge = (y+.5)*led_interval;// + sign(y-.5)*(led_interval + outer_wall_thickness - outer_spacer_width - 3)/2;
                
                translate([sign(x-.5)*(led_interval + lr_spacers[x])/2 + x*(n_leds[0]-1)*led_interval, toouteredge, outer_spacers[2]/2])
                cube([lr_spacers[x], outer_spacers[0], outer_spacers[2]], true);
                // vertical (airgap) spacer
                translate([sign(x-.5)*(led_interval/2 + lr_spacers[x]) + x*(n_leds[0]-1)*led_interval - sign(x)*outer_spacers[1], toouteredge - outer_spacers[0]/2, 0])
                cube([outer_spacers[1], outer_spacers[0], grid_height + airgap_height]);
            }
        }
    }
    // outer spacers: top/bottom
    for (y = [0, 1]) {
        if (tb_spacers[y] > 0) {
            for (x = [floor((n_leds[0]-1)%outer_spacer_every/2):outer_spacer_every:n_leds[0]-1]) {
                toouteredge = (x+.5)*led_interval;// + sign(x-.5)*(led_interval + outer_wall_thickness - outer_spacer_width)/2;
                
                translate([toouteredge, sign(y-.5)*(led_interval + tb_spacers[y])/2 + y*(n_leds[1]-1)*led_interval, outer_spacers[2]/2])
                cube([outer_spacers[0], tb_spacers[y], outer_spacers[2]], true);
                // vertical (airgap) spacer
                translate([toouteredge - outer_spacers[0]/2, sign(y-.5)*(led_interval/2 + tb_spacers[y]) + y*(n_leds[1]-1)*led_interval - sign(y)*outer_spacers[1], 0])
                cube([outer_spacers[0], outer_spacers[1], grid_height + airgap_height]);
                // extra (on-grid) spacer
                translate([toouteredge - outer_spacers[0]/2, sign(y-.5)*led_interval/2 - outer_wall_thickness/2 + y*(n_leds[1]-1)*led_interval, 0])
                cube([outer_spacers[0], outer_wall_thickness, grid_height + airgap_height]);
            }
        }
    }
}


// from left to right
led_diffuser_grid([22, 8], lr_spacers=[16, 0]);
//led_diffuser_grid([28, 8], lr_spacers=[0, 0]); // thrice
//led_diffuser_grid([22, 8], lr_spacers=[0, 16]);
