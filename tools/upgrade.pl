#!/usr/bin/perl

#
# upgrade.pl can be used to transfer the different oer configurations
# all at once from the normal version to oer+MySQL
#

# usage: upgrade.pl <oer+MySQL ident> <oer base directory>

@configs = ( "conf",
             "admins",
             "channels",
             "chaninfo",
             "kickreasons",
             "trusted",
             "services" );

$args = $#ARGV + 1;

if ($args != 2) {
    print STDERR "insufficient number of arguments.\n";
    print STDERR "usage: upgrade.pl <oer+MySQL ident> <oer base directory>\n";
    exit;
}

$arg1 = @ARGV[0];
$arg2 = @ARGV[1];

$botident = $arg1;
$serverid = 1;

for $i ( 0 .. $#configs ) {
    $config = $configs[$i];
    $filename = $arg2 . "/" . "oer." . $config;
    if(open(fd, "$filename")) {
	$got_conf = 0;
	while (<fd>) {
	    chop;
	    $query = "";
	    if($config eq "admins") {
		if(/^admin::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_admins VALUES ('$1', '$2', '$botident')";
		}
		if(/^mask::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_hostmasks VALUES ('$2', '', 1, '$1', '$botident')";
		}
	    } elsif($config eq "chaninfo") {
		if(/^chaninfo::(\S*)::user::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_users VALUES ('$2', '$1', '$3', '$botident')";
		}
		if(/^chaninfo::(\S*)::usermask::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_hostmasks VALUES ('$3', '$1', 2, '$2', '$botident')";
		}
		if(/^chaninfo::(\S*)::password::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_passwords VALUES ('$2', '$1', 1, '$3', '$botident')";
		}
		if(/^chaninfo::(\S*)::wordbk::(\S*)::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_wordbks VALUES ('$1', '$2', '$3', '$4', 0, '$botident')";
		}
		if(/^chaninfo::(\S*)::nickbk::(\S*)::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_nickbks VALUES ('$1', '$2', '$3', '$4', 0, '$botident')";
		}
		if(/^chaninfo::(\S*)::permban::(\S*)::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_permbans VALUES ('$1', '$2', '$3', '$4', 0, '$botident')";
		}
		if(/^chaninfo::(\S*)::floodvars::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)$/) {
		    $query = "INSERT INTO oer_floodvars VALUES ('$1', $2, $3, $4, $5, $6, $7, $8, '$botident')";
		}
		if(/^chaninfo::(\S*)::banvars::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)$/) {
		    $query = "INSERT INTO oer_banvars VALUES ('$1', $2, $3, $4, $5, $6, $7, $8, '$botident')";
		}
		if(/^chaninfo::(\S*)::advert::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_adverts VALUES ('$1', '$2', '$3', '$botident')";
		}
		if(/^chaninfo::(\S*)::bantype::(\d*)$/) {
		    $query = "INSERT INTO oer_bantype VALUES ('$1', $2, '$botident')";
		}
	    } elsif($config eq "channels") {
		if(/^channel::(\S*)::(\S*)::(\S*)::(\S*)::(\S*)$/) {
		    $query = "INSERT INTO oer_channels VALUES ('$1', '$3', '$2', '$5', '$4', '$botident')";
		}
	    } elsif($config eq "conf") {
		if(/^nick::(\S*)$/) {
		    $got_conf = 1;
		    $conf_nick = $1;
		}
		if(/^altnick::(\S*)$/) {
		    $got_conf = 1;
		    $conf_altnick = $1;
		}
		if(/^user::(\S*)$/) {
		    $got_conf = 1;
		    $conf_user = $1;
		}
		if(/^usermode::(\S*)$/) {
		    $got_conf = 1;
		    $conf_usermode = $1;
		}
		if(/^flags::(\S*)$/) {
		    $got_conf = 1;
		    $conf_flags = $1;
		}
		if(/^prefix::(\S*)$/) {
		    $got_conf = 1;
		    $conf_prefix = $1;
		}
		if(/^realname::(.*)$/) {
		    $got_conf = 1;
		    $conf_realname = $1;
		}
		if(/^vhost::(\S*)$/) {
		    $got_conf = 1;
		    $conf_vhost = $1;
		}
		if(/^qhost::(\S*)$/) {
		    $got_conf = 1;
		    $conf_qhost = $1;
		}
		if(/^qname::(\S*)$/) {
		    $got_conf = 1;
		    $conf_qname = $1;
		}
		if(/^qpassword::(\S*)$/) {
		    $got_conf = 1;
		    $conf_qpassword = $1;
		}
		if(/^server::(\S*)::(\d*)::(\d*)::(\d*)::(\d*)::(\d*)::(\S*)$/) {
		    $query = "INSERT INTO oer_servers VALUES ('$1', $2, $3, $4, $5, $6, '$7', '$botident', $serverid)";
		    $serverid++;
		}
	    } elsif($config eq "kickreasons") {
		if(/^kickreason::(.*)$/) {
		    $kr = $1;
		    $kr =~ s/'/\''/g;
		    $query = "INSERT INTO oer_kickreasons VALUES ('$kr', '$botident')";
		}
	    } elsif($config eq "trusted") {
		if(/^trusted::(\S*)$/) {
		    $query = "INSERT INTO oer_trusted VALUES ('$1', '$botident')";
		}
	    } elsif($config eq "services") {
		if(/^service::(\S*)$/) {
		    $query = "INSERT INTO oer_services VALUES ('$1', '$botident')";
		}
	    }
	    if($query ne "") {
		print "$query;\n";
	    }
	}
	if($got_conf == 1) {
	    $query = "INSERT INTO oer_conf VALUES ('$conf_nick', '$conf_altnick', '$conf_user', '$conf_usermode', '$conf_flags', '$conf_prefix', '$conf_realname', '$conf_vhost', '$conf_qhost', '$conf_qname', '$conf_qpassword', '$botident')";
	    print "$query;\n";
	}
	close (fd);
    }
}
