#!/usr/bin/perl
use strict;
use warnings;
use PDL;
use PDL::IO::Pic;
use PDL::Reduce;
use Math::Orthonormalize qw(normalize orthogonalize orthonormalize);
use Benchmark qw(:hireswallclock);
use vars qw/$StartTime @Benches/;
BEGIN {$StartTime = Benchmark->new(); push @Benches, $StartTime;}
END{print_timings()}

my $file = shift||'t3.jpg';
my $image = rpic($file);

mark_save("read image");

#print $image;

my $target = [1,0,0];
my @orthosys = make_orthonormal_color_system($target);

mark_save("orthonormalization");

my $tvec = pdl($orthosys[0]);
my $o1vec = pdl($orthosys[1]);
my $o2vec = pdl($orthosys[2]);

my $projected = inner($image, $tvec);
mark_save("projection");
my $other = inner($image, $o1vec) + inner($image, $o2vec);
$other /= 2;
mark_save("other projection");
my $projectedlight = $projected->copy();
mark_save("copying");

$projected -= $other;
mark_save("difference");

#print $projected;

my $max = max($projected);
my $min = min($projected);
#my $max = maximum(maximum($projected));
#my $min = minimum(minimum($projected));
mark_save("minimum, maximum");

my $d = $max-$min;

#foreach my $step (0..10) {
#
#}

my $threshold = $min+$d*0.8;

print "Threshold: $threshold\n";

my $coords = whichND($projected > $threshold);
mark_save("find above threshold coords");
#print $coords;

my $maxarea = $projected->indexND($coords)->copy();
mark_save("get selected pixels");
$maxarea -= $min;
$maxarea *= 255/$d;
mark_save("scale selected pixels");

$projected .= 0;
mark_save("reset image");
$projected->indexND($coords) .= $maxarea;
mark_save("replace selected pixels");
#print $projected;

my ($xc, $yc) = centroid($projected);
mark_save("find centroid");

print "$xc - $yc\n";

my $ofile = $file;
$ofile =~ s/\.\w+$//;
wpic($projected, "$ofile.proj.png");
mark_save("write_files");
wpic($projectedlight, "$ofile.projlight.png");
mark_save("write_files");

sub mywpnm {
  my $pdl = shift;
  my $type = shift;
  my @dims = $pdl->dims;
    # check the data
    die "mywpnm: exspecting 3D (3,w,h) input"
	if ($type =~ /PPM/) && (($#dims != 2) || ($dims[0] != 3));
    die "mywpnm: exspecting 2D (w,h) input"
	if ($type =~ /PGM/) && ($#dims != 1);
    shift @dims if $type eq 'PPM';

    my $magic = "P5"; # PGM;
    $magic = "P6" if $type eq 'PPM';
    my $out;
    open(OUT, '>', \$out) or die $!;
    print OUT "$magic\n";

  print  OUT join(" ", @dims) . "\n";

    die "mywpnm: unknown format '$type'" if $type !~ /P[PG]M/;

    my $max =$pdl->max;
    print OUT "$max\n";

#use PDL::IO::FlexRaw;
if ($type eq 'PPM') {
print OUT ${$pdl->slice(':,:,-1:0')->get_dataref()};
}
else {
print OUT ${$pdl->slice(':,-1:0')->get_dataref()};
}
#print OUT ${$pdl->slice(':,:,-1:0')->xchg(0,2)->get_dataref()};
#  writeflex(*OUT, $pdl->slice(':,:'));
  #writeflex(*OUT, $pdl->slice(':,-1:0'));
    return $out;
}
my $bin = mywpnm($image, 'PPM');
print "PPM size: " . length($bin) . "\n";
open my $fh, '>', 't.pnm' or die $!;
binmode $fh;
print $fh $bin;
close $fh;

use SDL::App;
use SDL::Surface;
use SDL::Rect;


my $res = [800, 600];

my $app = SDL::App->new(
  -width  => $res->[0],
  -height => $res->[1],
  -depth  => 24,
);

$app->fullscreen();
open my $foo, '<', 't2.jpg' or die $!;
my $jpg = do {local $/=undef; <$foo>};
close $foo;
my $frame = SDL::Surface->new(
  -from => $jpg,
#${$image->slice(':,-1:0')->get_dataref()},
  -width => 1152, height => 864,
);


my $frame_rect = SDL::Rect->new(
  -height => $frame->height(),
  -width  => $frame->width(),
  -x      => 0,
  -y      => 0,
);

my $dest_rect  = SDL::Rect->new(
    -height => $frame->height(),
    -width  => $frame->width(),
    -x      => 0,
    -y      => 0,
    );

$frame->blit( $frame_rect, $app, $dest_rect );
$app->update( $dest_rect );
sleep 2;




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

sub mark_time {
  shift @Benches if @Benches > 1;
  push @Benches, Benchmark->new();
  return();  
}
sub mark_save {
  mark_time();
  save_timing(@_);
}

{
  my @timings;
  sub save_timing {
    my $msg = shift;
    push @timings, [$msg, Benchmark::timediff($Benches[1], $Benches[0])];
  }
  sub print_timings {
    my $cumulative;
    foreach my $timing (@timings) {
      if (not defined $cumulative) { $cumulative = $timing->[1] }
      else { $cumulative = Benchmark::timesum($cumulative, $timing->[1]) }

      print sprintf('%.3f', $cumulative->[0]) . "s - " . Benchmark::timestr($timing->[1]) . " : " . $timing->[0] . "\n";
    }
  }
}


