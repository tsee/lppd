#!/usr/bin/perl
use strict;
use warnings;
use SDL::App;
use SDL::Surface;
use SDL::Rect;


my $img = shift || 't2.jpg';

my $res = [800, 600];

my $app = SDL::App->new(
  -width  => $res->[0],
  -height => $res->[1],
  -depth  => 24,
);

$app->fullscreen();
open my $fh, '<', $img;
binmode $fh;
my $binary = do{local $/=undef; <$fh>};
close $fh;
my $frame = SDL::Surface->new(
  -from => $binary,
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
