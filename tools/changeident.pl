#!/usr/bin/perl

#
# changeident.pl can be used to change the oer+MySQL ident
#

# usage: changeident.pl <old bot ident> <new bot ident> 

$args = $#ARGV + 1;

if ($args != 2) {
    print "insufficient number of arguments\n";
    print "usage: changeident.pl <old bot ident> <new bot ident>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];
$old_ident = $arg1;
$new_ident = $arg2;

print "UPDATE oer_admins SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_adverts SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_autheds SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_bantype SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_banvars SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_channels SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_conf SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_ext SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_floodvars SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_hostmasks SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_kickreasons SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_last SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_nickbks SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_output SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_passwords SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_permbans SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_raw SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_seen SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_servers SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_services SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_timestamps SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_topics SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_trusted SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_users SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
print "UPDATE oer_wordbks SET ident = '$new_ident' WHERE ident = '$old_ident';\n";
