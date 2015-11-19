#!/usr/bin/perl -w

# author: Marcel Wirth <mawirth@student.ethz.ch>
# interprets the benchmark results csv file and generates an xhtml-file with inline scalable vector graphics graphs as output

# general settings
##################

$resultsfile = "results.csv";	# csv file containing benchmark results

# title (both html title and document title)
$title = "Java Grande Forum Benchmark Results";

# xhtml header text
$header = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN\" \"http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\"
      xmlns:svg=\"http://www.w3.org/2000/svg\"
      xml:lang=\"en\">
  <head>
    <title>${title}</title>
    <object id=\"AdobeSVG\" classid=\"clsid:78156a80-c6a1-4bbf-8e6a-3cd390eeb4e2\"></object>
    <?import namespace=\"svg\" implementation=\"#AdobeSVG\"?>
  </head>
  <body>
	<h2>${title}</h2>
	<p>Each benchmark is run once with fastbt (using identity transform) and once without. Times are measured in seconds. <br />
	We would like to minimize the overhead of fastbt, so that execution times with fastbt are almost as fast as without.<br />
	File creation date: ". `date` . "</p>\n\n";


# xhtml footer text
$footer = "\n\n\t<p style=\"font-size: 8pt\">Requires xml capable browser with svg implementation. Known to work with Firefox &gt;= 1.5, Opera &gt;= 9.0, Internet Explorer with Adobe SVG plugin (remove doctype and rename to .html), probably Safari &gt;= 3.0<br />
This file is <a href=\"http://validator.w3.org/#validate_by_upload+with_options\">valid</a> XHTML 1.1 plus MathML 2.0 plus SVG 1.1</p>\n\n  </body>\n</html>\n";


# svg settings
##############

$hor_axis_y = 350;	# y coordinate of the horizontal axis

$vert_axis_x = 110;	# x coordinate of the vertical axis

$vert_axis_height = 290;	# height of the vertical axis

$max_bar_height = 250;	# height of the higher bar in pixel

$tick_length = 10; 	# length of the ticks on the vertical axis

# constant parts of svg graphs
$svg_header = "\t<p><svg:svg width=\"500\" height=\"400\">
		
		<!-- draw coordinate system axes -->
		<svg:line x1=\"70\" y1=\"${hor_axis_y}\" x2=\"450\" y2=\"${hor_axis_y}\" stroke=\"#000000\" stroke-width=\"1\" />
		<svg:line x1=\"${vert_axis_x}\" y1=\"${hor_axis_y}\" x2=\"$vert_axis_x\" y2=\"60\" stroke=\"#000000\" stroke-width=\"1\" />
		
		<!-- write static text -->
		<svg:text x=\"195\" y=\"370\" font-size=\"16\" text-anchor=\"middle\">time without fastbt</svg:text>
		<svg:text x=\"365\" y=\"370\" font-size=\"16\" text-anchor=\"middle\">time with fastbt</svg:text>
		<svg:text x=\"20\" y=\"205\" font-size=\"16\">[s]</svg:text>\n\n";


# subroutines
#############

# draw_graph(benchmark_name, errors_native, time_native, errors_fastbt, time_fastbt)
sub draw_graph {
	my $benchmark_name = $_[0];
	my $time_native = $_[2];
	my $errors_native = $_[1];
	my $time_fastbt = $_[4];
	my $errors_fastbt = $_[3];
	
	$time_native = 0 unless is_numeric($time_native);
	$time_fastbt = 0 unless is_numeric($time_fastbt);
	
	my $maxtime = 0;	# maximum of ($time_native, $time_fastbt)
	my $native_height = 0;	# height of native result bar in pixels
	my $fastbt_height = 0;	# height of fastbt result bar in pixels
	
	print $svg_header;	# print constant parts of graph (coordinate axes, some labels)
	
	# draw result bars
	if ($time_fastbt > $time_native) {
		$fastbt_height = $max_bar_height;
		$native_height = int($time_native / $time_fastbt * $fastbt_height);
		$maxtime = $time_fastbt;
	} else {
		# conditional statements to prevent drawing artifacts, division by zero
		$native_height = ($time_native > 0) ? $max_bar_height : 0;
		$fastbt_height = ($time_native > 0) ? int($time_fastbt / $time_native * $native_height) : 0;
		$maxtime = $time_native;
	}
	print "\t\t<!-- draw result bars -->
		<svg:rect x=\"170\" y=\"", $hor_axis_y - $native_height, "\" width=\"50\" height=\"${native_height}\" fill=\"#2d2ded\" stroke=\"#1a1a99\" stroke-width=\"1\" />
		<svg:rect x=\"340\" y=\"", $hor_axis_y - $fastbt_height, "\" width=\"50\" height=\"${fastbt_height}\" fill=\"#2d2ded\" stroke=\"#1a1a99\" stroke-width=\"1\" />\n";
	print "\t\t<svg:text x=\"195\" y=\"", $hor_axis_y - $native_height - 5, "\" font-size=\"16\" text-anchor=\"middle\">${time_native}</svg:text>	<!-- time value without binary translation -->
		<svg:text x=\"365\" y=\"", $hor_axis_y - $fastbt_height - 5, "\" font-size=\"16\" text-anchor=\"middle\">${time_fastbt}</svg:text>	<!-- time value with binary translation -->\n\n";

	# draw graph title
	print "\t\t<svg:text x=\"280\" y=\"50\" font-size=\"24\" text-anchor=\"middle\" font-weight=\"bold\" stroke-width=\"1\" stroke=\"#000000\" fill=\"#dddddd\">${benchmark_name}</svg:text>	<!-- benchmark name -->\n\n";

	# draw error messages
	print "\t\t<!-- write error messages -->
		<svg:text x=\"195\" y=\"390\" font-size=\"16\" text-anchor=\"middle\" fill=\"#ff0000\">${errors_native}</svg:text>
		<svg:text x=\"365\" y=\"390\" font-size=\"16\" text-anchor=\"middle\" fill=\"#ff0000\">${errors_fastbt}</svg:text>\n\n";
   	
	# draw time axis ticks
	draw_ticks($maxtime);
	
	# close svg graph
	print "\t</svg:svg>\n\t</p>\n";
}

# draw ticks on time axis
# draw_ticks(maxtime)
sub draw_ticks {
	my $maxtime = $_[0];
	my $timeticks = 0;
	my $tickdist = 0;
	
	if ($maxtime < 5) {
		if ($maxtime < 1) {
			if ($maxtime < 0.5) {
				$timeticks = 0.1;
			} else {
				$timeticks = 0.2;
			}
		} else {
			if ($maxtime < 2) {
				$timeticks = 0.5;
			} else {
				$timeticks = 1;
			}
		}
		
	} else {
		if ($maxtime < 100) {
			if ($maxtime < 20) {
				if ($maxtime < 10) {
					$timeticks = 2;
				} else {
					$timeticks = 5;
				}
			} else {
				if ($maxtime < 50) {
					$timeticks = 10;
				} else {
					$timeticks = 20;
				}
			}
		} else {
			if ($maxtime < 500) {
				if ($maxtime < 200) {
					$timeticks = 50;
				} else {
					$timeticks = 100;
				}
			} else {
				if ($maxtime < 1000) {
					$timeticks = 200;
				} else {
					$timeticks = 500;
				}
			}
		}
	}
	
	
	if (0 == $maxtime) {
		$tickdist=$vert_axis_height;
		$timeticks = 0;
	} else {
		$tickdist = $max_bar_height / $maxtime * $timeticks;
	}
	
	my $i = 0;
	for ($i = 1; $i < ($vert_axis_height / $tickdist); $i++) {
		print "\t\t<svg:line x1=\"", $vert_axis_x - $tick_length, "\" y1=\"", $hor_axis_y - int($tickdist * $i), "\" x2=\"${vert_axis_x}\" y2=\"", $hor_axis_y - int($tickdist * $i), "\" stroke=\"#000000\" stroke-width=\"1\" />
		<svg:text x=\"", $vert_axis_x - $tick_length - 5, "\" y=\"", $hor_axis_y - int($tickdist * $i) + 5, "\" font-size=\"12\" text-anchor=\"end\">", $i * $timeticks, "</svg:text>\n\n";
   	}



}

# returns 1 if argument is a float or integer number, returns 0 otherwise
# is_numeric(str)
sub is_numeric {
	if ($_[0] =~ /^([+-]?)(?=\d|\.\d)\d*(\.\d*)?([Ee]([+-]?\d+))?$/) {
		return 1;
	} else {
		return 0;
	}
}


# program
#########


print $header;

open(RESULTS, '<', $resultsfile) or die "can't open benchmark results file<br />\n";

while(<RESULTS>) {
	if(/^([^,]*),([^,]*),([^,]*),([^,]*),([^,]*)$/) {
		if ((is_numeric($3) || "" eq $3) && (is_numeric($5) || "" eq $5)) {
			draw_graph($1, $2, $3, $4, $5);
		}	
	} else {
		print "<p>invalid line in results file</p>\n";
	}
}

close(RESULTS);

#draw_graph("TEST", "segfault", "", "", "3");
#draw_graph("TEST2", "", "1.90", "", "3");

print $footer;
