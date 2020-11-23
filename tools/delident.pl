#!/usr/bin/perl

#
# delident.pl can be used to delete one oer+MySQL installation
# from the datebase, be careful!
#

# usage: delident.pl <bot ident>

$args = $#ARGV + 1;

if ($args != 1) {
    print "insufficient number of arguments\n";
    print "usage: delident.pl <bot ident>\n";
    exit;
}

$arg1 = @ARGV[0];
$botident = $arg1;

print "DELETE FROM oer_admins WHERE ident = '$botident';\n";
print "DELETE FROM oer_adverts WHERE ident = '$botident';\n";
print "DELETE FROM oer_autheds WHERE ident = '$botident';\n";
print "DELETE FROM oer_bantype WHERE ident = '$botident';\n";
print "DELETE FROM oer_banvars WHERE ident = '$botident';\n";
print "DELETE FROM oer_channels WHERE ident = '$botident';\n";
print "DELETE FROM oer_conf WHERE ident = '$botident';\n";
print "DELETE FROM oer_ext WHERE ident = '$botident';\n";
print "DELETE FROM oer_floodvars WHERE ident = '$botident';\n";
print "DELETE FROM oer_hostmasks WHERE ident = '$botident';\n";
print "DELETE FROM oer_kickreasons WHERE ident = '$botident';\n";
print "DELETE FROM oer_last WHERE ident = '$botident';\n";
print "DELETE FROM oer_nickbks WHERE ident = '$botident';\n";
print "DELETE FROM oer_output WHERE ident = '$botident';\n";
print "DELETE FROM oer_passwords WHERE ident = '$botident';\n";
print "DELETE FROM oer_permbans WHERE ident = '$botident';\n";
print "DELETE FROM oer_raw WHERE ident = '$botident';\n";
print "DELETE FROM oer_seen WHERE ident = '$botident';\n";
print "DELETE FROM oer_servers WHERE ident = '$botident';\n";
print "DELETE FROM oer_services WHERE ident = '$botident';\n";
print "DELETE FROM oer_timestamps WHERE ident = '$botident';\n";
print "DELETE FROM oer_topics WHERE ident = '$botident';\n";
print "DELETE FROM oer_trusted WHERE ident = '$botident';\n";
print "DELETE FROM oer_users WHERE ident = '$botident';\n";
print "DELETE FROM oer_wordbks WHERE ident = '$botident';\n";

