#!/usr/bin/perl -w

my $dir1  = $ARGV[0];
my $dir2  = $ARGV[1];
my $num = 0;
my $dummy;

while( <$dir2/*.csa> ){
	( $dummy, $fname ) = split( '/', $_ );
	if( -f "$dir1/$fname" ){
		print "$fname\n";
		system "rm $dir1/$fname";
		$num++;
	}
}

print "$num files.\n";
