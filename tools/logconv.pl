#!/usr/bin/perl

#
# logconv.pl can be used to convert last and seen logs from
# the old format (last_#channel and seen_#channel) to the new
# format (tables last and seen)
#

# usage: logconv.pl <type> <channel>

$args = $#ARGV + 1;

if ($args != 3) {
    print "insufficient number of arguments\n";
    print "usage: logconv.pl <type> <channel> <ident>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];
$arg3 = @ARGV[2];

if($arg1 ne "last" && $arg1 ne "seen") {
    print "required argument missing.\n";
    print "supported types: last and seen\n";
    exit;
}

$tablename = "$arg1\_$arg2";
$tablename =~ s/\-//g;
$tablename =~ s/\.//g;
$tablename =~ s/\#//g;

if($arg1 eq "last") {
    print "INSERT INTO oer_$arg1 (channel,nick,twhen,hostmask,ident,message) SELECT '$arg2',nick,twhen,hostmask,'$arg3',message FROM $tablename;\n";
} else {
    print "INSERT INTO oer_$arg1 (channel,nick,twhen,hostmask,ident) SELECT '$arg2',nick,twhen,hostmask,'$arg3' FROM $tablename;\n";
}




