#!/usr/bin/perl -w

my $emp_line_cnt = 0;
my %result;
while (<STDIN>) {
	my $line = $_;
	chomp($line);
	if ($line =~ /^$/) {
		$emp_line_cnt++;
		if ($emp_line_cnt == 2) {
			last;
		}
	} elsif ($emp_line_cnt == 1 && $line =~ /^ *([0-9.]+) +[^ ]+ +[^ ]+ +[^ ]+ +[^ ]+ +[^ ]+ +([^ ].+)$/) {
		my $percentage = 0.0 + $1;
		if ($percentage == 0) {
			next;
		}
		my $name = $2;
		my $namespace = 'none';
		$name =~ s/^((<[^>]+>)|([^()]))+ +//;
		if ($name =~ s/^(((<[^>]+>)|([^<>()]))+):://) {
			$namespace = $1;
			$namespace =~ s/<.*$//;
		}
		$result{$namespace} = ($result{$namespace} or 0) + $percentage;
	}
}

foreach my $key (keys(%result)) {
	printf "%2.2f %s\n", $result{$key}, $key;
}
