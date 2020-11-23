#!/usr/bin/perl

#
# adminmove.pl can be used to move admins, hostmasks and
# passwords from one ident to another
#

# usage: adminmove.pl <ident from> <ident to>

$args = $#ARGV + 1;

if ($args != 2) {
    print "insufficient number of arguments\n";
    print "usage: adminmove.pl <ident from> <ident to>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];

print "UPDATE oer_admins SET ident = '$arg2' WHERE ident = '$arg1';\n";
print "UPDATE oer_hostmasks SET ident = '$arg2' WHERE type = 1 AND ident = '$arg1';\n";
print "UPDATE oer_passwords SET ident = '$arg2' WHERE type = 2 AND ident = '$arg1';\n";
print "UPDATE oer_timestamps SET twhen = twhen + 1 WHERE ttype = 'admins' WHERE ident = '$arg2';\n";
