$fn=6;
dot=.5*2.54; // 1/20th of an inch

module hole_button(x, y) {
	translate([x*dot, y*dot, 0]) cube([8, 8, 10], center=true);
}

module hole_led(x, y) {
	translate([x*dot, y*dot, 0]) cylinder(r=2,5, h=10, center=true);
}

module hole_screw(x, y) {
	translate([x*dot, y*dot, 0]) cylinder(r=1,5, h=10, center=true);
}

module holes() {
	translate([32*dot, 47.5*dot, 0]) cube([56*dot, 19*dot, 10], center=true);	// display
	
	hole_button(11, 18);	// cursor left
	hole_button(21, 28);	// cursor up
	hole_button(21, 8);		// cursor down
	hole_button(31, 18);	// cursor right

	hole_button(53, 28);	// fn right top
	hole_button(53, 18);	// fn right middle
	hole_button(53, 8);		// fn right bottom

	hole_led(44, 28);		// led top
	hole_led(44, 18);		// led middle
	hole_led(44, 8);			// led bottom

	hole_screw(4, 4);		// screw left bottom
	hole_screw(60, 4);		// screw right bottom
	hole_screw(4, 32);		// screw left top
	hole_screw(60, 32);		// screw right top
}

difference() {
	union() {
		difference() {
			minkowski(){
				difference() {
					cube([153.64, 90, 90]);
					translate([90,0,90]) rotate([0, 45, 0]) cube([90, 90, 90]);
				}
				cylinder(h=2, r=2);
			}
			// remove interior
			cube([90, 90, 90]);
			translate([26.36, 0, 26.36]) rotate([0, 45, 0]) cube([90, 90, 90]);
			translate([90,0,0]) cube([63.64, 90, 26.36]);
		}
		// add 2 mm ridge for bottom support
		difference() {
			translate([0, 0, 2]) cube([153.64, 90, 5]);
			translate([2, 2, 2]) cube([149.64, 88, 5]);
		}
	}
	// remove holes for display, buttons and leds
	translate([153.64, 4.375 ,26.36]) rotate([45, 0, 90]) holes();
}

