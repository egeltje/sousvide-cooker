$fn=60;

difference () {
	// plug and conducts
	cylinder(h = 15, r1 = 31.5, r2 = 33.5, center = true);

	// heater
	translate([-25, -9.75, -8]) {
		cube([11, 19.5, 16]);
	}

	// pump
	// inlet
	translate([0, -22, 0]) {
		cylinder(h = 16, r = 6, center = true);
	}
	// outlet
	translate([0, 22, 0]) {
		cylinder(h = 16, r = 3, center = true);
	}
	// sensor
	translate([0, 29, 0]) {
		cylinder(h = 16, r = 2, center = true);
	}
}

// heater
difference() {
	translate([-27, -11.75, -17.5]) {
		cube([15, 23.5, 10]);
	}
	translate([-25, -9.75, -18]) {
		cube([11, 19.5, 11]);
	}
};

// pump inlet tube
translate([0, 0, -47.5]) {
	translate([0, -22, 0]) {
		difference() {
			cylinder(h = 40, r = 8);
			cylinder(h = 40, r = 6);
		}
	}
	translate([0, -22, 0]) {
			intersection() {
				translate([-15, -8, -30]) {
					cube([30, 30, 30]);
				}
				translate([0, 22, 0]) {
					rotate([0, 90, 0]) {
						difference() {
							rotate_extrude()translate([22,0,0])circle(r=8);
							rotate_extrude()translate([22,0,0])circle(r=6);
						}
					}
				}
		}
	}
}
difference() {
	translate([0, 0, -69.5]) {
		rotate([-90, 0, 0]) {
			difference() {
				cylinder(h = 27, r1 = 8, r2 = 18);
				cylinder(h = 27, r1 = 6, r2 = 16);
			}
		}
	}
	translate([0, 22, -87.5]) {
		cylinder(h = 80, r = 3);
	}
}


// pump outlet tube
translate([0, 0, -97.5]) {
	translate([0, 22, 0]) {
		difference() {
			cylinder(h = 90, r = 5);
			cylinder(h = 90, r = 3);
		}
	}
	intersection() {
		translate([-15, 10, -30]) {
			cube([30, 30, 30]);
		}
		rotate([0, 90, 0]) {
			difference() {
				rotate_extrude()translate([22,0,0])circle(r=5);
				rotate_extrude()translate([22,0,0])circle(r=3);
			}
		}
	}
}


//	// tiewrap
//	translate([-17.5, 11, -8]) {
//		cube([3, 6, 16]);
//	}
//	translate([14.5, 11, -8]) {
//		cube([3, 6, 16]);
//	}


