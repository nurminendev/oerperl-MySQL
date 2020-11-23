#!/usr/bin/perl

#
# delchannel.pl can be used to delete a channel of a oer+MySQL installation
# from the datebase
#

# usage: delchannel.pl <bot ident> <channel>

$args = $#ARGV + 1;

if ($args != 2) {
    print "insufficient number of arguments\n";
    print "usage: delchannel.pl <bot ident> <channel>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];
$botident = $arg1;
$channel = $arg2;

print "DELETE FROM oer_adverts WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_autheds WHERE tchannel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_bantype WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_banvars WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_channels WHERE name = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_floodvars WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_hostmasks WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_nickbks WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_passwords WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_permbans WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_timestamps WHERE tchannel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_topics WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_users WHERE channel = '$channel' AND ident = '$botident';\n";
print "DELETE FROM oer_wordbks WHERE channel = '$channel' AND ident = '$botident';\n";
