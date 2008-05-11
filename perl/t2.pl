  use Math::Orthonormalize qw(:all);
  
  my @base_of_r_2 = (
    [2, 1],
    [2, 1]
  );
  
  my @orthogonalized  = orthogonalize(@base_of_r_2);
  use Data::Dumper;
  print Dumper \@orthogonalized;
  
