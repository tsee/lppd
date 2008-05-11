#!/usr/bin/perl -w 
 use PDL;
 use PDL::Graphics::PGPLOT::Window;
 use PDL::GSLSF::BESSEL;
 $w=pgwin('t2.jpg/jpg',{Size=>3.5});
 $x=zeroes(1000)->xlinvals(0,20);
 $y=bessj0($x);
 $w->line($x,$y,{Title=>'Bessel',XTitle=>'x',YTitle=>'y'});
 $w->close;
