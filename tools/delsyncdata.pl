#!/usr/bin/perl

#
# delident.pl can be used to delete all data that is used
# in a shared oer+MySQL setup - useful to remove obsolete data
#

# usage: delsyncdata.pl <bot ident>

$args = $#ARGV + 1;

if ($args != 1) {
    print "insufficient number of arguments\n";
    print "usage: delsyncdata.pl <bot ident>\n";
    exit;
}

$arg1 = @ARGV[0];
$botident = $arg1;

print "DELETE FROM oer_admins WHERE ident = '$botident';\n";
print "DELETE FROM oer_autheds WHERE ident = '$botident';\n";
print "DELETE FROM oer_hostmasks WHERE ident = '$botident';\n";
print "DELETE FROM oer_passwords WHERE ident = '$botident';\n";
print "DELETE FROM oer_timestamps WHERE ident = '$botident';\n";
print "DELETE FROM oer_users WHERE ident = '$botident';\n";
