#!/usr/bin/perl

#
# last2mysql.pl can be used to transfer last logs from normal
# oer to oer+MySQL, you will have to specify the channel on
# the command line since it is not included in the data itself
#

# usage: last2mysql.pl <bot ident> <channel> <filename>

$args = $#ARGV + 1;

if ($args != 3) {
    print "insufficient number of arguments\n";
    print "usage: last2mysql.pl <bot ident> <channel> <filename>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];
$arg3 = @ARGV[2];

$botident = $arg1;
$channel = $arg2;

open (fd, "$arg3") || die "couldn't open $arg3 for reading\n";

while (<fd>) {
    chop;
    ($timestamp, $nick, $host, $message) = split(/::/, $_, 4);
    $query = "INSERT INTO oer_last VALUES ('$channel', '$nick', $timestamp, '$host', '$botident', '$message')";
    print "$query;\n";
}

close (fd);
