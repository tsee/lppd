#!/usr/bin/perl
use strict;
use warnings;
use PDL;
use PDL::IO::Pic;
use PDL::Reduce;
use Math::Orthonormalize qw(normalize orthogonalize orthonormalize);

my $file = shift||'t3.jpg';
my $image = rpic($file);

#print $image;

my $target = [1,0,0];
my @orthosys = make_orthonormal_color_system($target);

my $tvec = pdl($orthosys[0]);
my $o1vec = pdl($orthosys[1]);
my $o2vec = pdl($orthosys[2]);

my $projected = inner($image, $tvec);
my $other = inner($image, $o1vec) + inner($image, $o2vec);
$other /= 2;
my $projectedlight = $projected->copy();

$projected -= $other;

print $projected;

my $max = max($projected);
my $min = min($projected);
#my $max = maximum(maximum($projected));
#my $min = minimum(minimum($projected));

my $d = $max-$min;

#foreach my $step (0..10) {
#
#}

my $threshold = $min+$d*0.8;

print "Threshold: $threshold\n";

my $coords = whichND($projected > $threshold);
#print $coords;

my $maxarea = $projected->indexND($coords)->copy();
$maxarea -= $min;
$maxarea *= 255/$d;

$projected .= 0;
$projected->indexND($coords) .= $maxarea;
#print $projected;

my ($xc, $yc) = centroid($projected);

print "$xc - $yc\n";

my $ofile = $file;
$ofile =~ s/\.\w+$//;
wpic($projected, "$file.proj.png");
wpic($projectedlight, "$file.projlight.png");

sub make_orthonormal_color_system {
  my $t = shift;

  if ($t->[1] == 0) {
    if ($t->[0] == 0) {
      return( [0,0,1], [0,1,0], [1,0,0] );
    }
    elsif ($t->[2] == 0) {
      return( [1,0,0], [0,1,0], [0,0,1] );
    }
  }

  my $ang = 0.3;
  my $cos = cos($ang);
  my $sin = sin($ang);

  my @system = (
    $t,
    [$t->[0]*$cos-$t->[1]*$sin, $t->[0]*$sin+$t->[1]*$cos, $t->[2]],
    [$t->[0], $t->[1]*$cos-$t->[2]*$sin, $t->[1]*$sin+$t->[2]*$cos],
  );
  my @orthosys = orthonormalize(@system);
  return @orthosys;
}


# threaded mult to calculate centroid coords, works for stacks as well
sub centroid {
  my $im = shift;
  my $sum_of_values = sumover($im->clump(2));

  my $xc = sumover(($im*xvals(($im->dims)[0]))->clump(2)) / $sum_of_values;
  my $yc = sumover(($im*yvals(1, ($im->dims)[1]))->clump(2)) / $sum_of_values;
  return($xc, $yc);
}

