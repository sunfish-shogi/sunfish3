#!/usr/bin/perl -w

$from = $ARGV[0];
$to   = $ARGV[1];
$num  = $ARGV[2];

while( <$from/*.csa> ){
	$files[@files] = $_;
}

while( $num < @files ){
	$rand = int( rand @files );
	splice( @files, $rand, 1 );
}

foreach( @files ){
	print "$_\n";
	system "cp $_ $to/";
}
