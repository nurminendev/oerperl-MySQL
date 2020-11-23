-- MySQL dump 9.09
--
-- Host: gate.rakkisnet.rakkisnetworks.org    Database: oerdev2
-- ------------------------------------------------------
-- Server version	4.0.16-log

--
-- Table structure for table `oer_admins`
--

CREATE TABLE oer_admins (
  handle varchar(32) NOT NULL default '',
  flags varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_admins_sync`
--

CREATE TABLE oer_admins_sync (
  handle varchar(32) NOT NULL default '',
  flags varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_adverts`
--

CREATE TABLE oer_adverts (
  channel varchar(50) NOT NULL default '',
  type varchar(32) NOT NULL default '',
  message text NOT NULL,
  ident varchar(20) NOT NULL default ''
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_autheds`
--

CREATE TABLE oer_autheds (
  ttype varchar(32) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  thandle varchar(32) NOT NULL default '',
  tchannel varchar(50) NOT NULL default '',
  thostmask varchar(80) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (ttype,twhen,thandle,tchannel,thostmask,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_autheds_sync`
--

CREATE TABLE oer_autheds_sync (
  ttype varchar(32) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  thandle varchar(32) NOT NULL default '',
  tchannel varchar(50) NOT NULL default '',
  thostmask varchar(80) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (ttype,twhen,thandle,tchannel,thostmask,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_bantype`
--

CREATE TABLE oer_bantype (
  channel varchar(50) NOT NULL default '',
  type tinyint(3) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_banvars`
--

CREATE TABLE oer_banvars (
  channel varchar(50) NOT NULL default '',
  b_auto_rejoin smallint(5) unsigned NOT NULL default '0',
  b_part_rejoin smallint(5) unsigned NOT NULL default '0',
  b_flood smallint(5) unsigned NOT NULL default '0',
  b_flood_repeat smallint(5) unsigned NOT NULL default '0',
  b_bad_word smallint(5) unsigned NOT NULL default '0',
  b_bad_nick smallint(5) unsigned NOT NULL default '0',
  b_normal_ban smallint(5) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_channels`
--

CREATE TABLE oer_channels (
  name varchar(50) NOT NULL default '',
  chanmodes varchar(12) default NULL,
  chankey varchar(32) default NULL,
  chanlimit varchar(32) default NULL,
  chanflags varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (name,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_conf`
--

CREATE TABLE oer_conf (
  nick varchar(20) NOT NULL default '',
  altnick varchar(20) NOT NULL default '',
  user varchar(16) default NULL,
  usermode varchar(8) default NULL,
  flags varchar(8) default NULL,
  prefix varchar(32) NOT NULL default '',
  realname varchar(64) NOT NULL default '',
  vhost varchar(63) default NULL,
  qhost varchar(80) default NULL,
  qname varchar(32) default NULL,
  qpassword varchar(32) default NULL,
  proxysetup varchar(128) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_ext`
--

CREATE TABLE oer_ext (
  par1 varchar(20) NOT NULL default '',
  par2 varchar(20) default NULL,
  par3 varchar(20) default NULL,
  par4 varchar(20) default NULL,
  par5 varchar(20) default NULL,
  ident varchar(20) NOT NULL default '',
  vartext text NOT NULL
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_floodvars`
--

CREATE TABLE oer_floodvars (
  channel varchar(50) NOT NULL default '',
  f_expire smallint(5) unsigned NOT NULL default '0',
  f_limit smallint(5) unsigned NOT NULL default '0',
  f_interval smallint(5) unsigned NOT NULL default '0',
  f_lines smallint(5) unsigned NOT NULL default '0',
  f_chars smallint(5) unsigned NOT NULL default '0',
  f_nickflood_expire smallint(5) unsigned NOT NULL default '0',
  f_nickflood_changes smallint(5) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_hostmasks`
--

CREATE TABLE oer_hostmasks (
  hostmask varchar(80) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  type tinyint(3) unsigned NOT NULL default '0',
  handle varchar(32) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (hostmask,channel,type,handle,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_hostmasks_sync`
--

CREATE TABLE oer_hostmasks_sync (
  hostmask varchar(80) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  type tinyint(3) unsigned NOT NULL default '0',
  handle varchar(32) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (hostmask,channel,type,handle,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_kickreasons`
--

CREATE TABLE oer_kickreasons (
  reason varchar(255) NOT NULL default '',
  ident varchar(20) NOT NULL default ''
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_last`
--

CREATE TABLE oer_last (
  channel varchar(50) NOT NULL default '',
  nick varchar(20) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  hostmask varchar(80) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  message text NOT NULL,
  KEY channel (channel,nick,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_nickbks`
--

CREATE TABLE oer_nickbks (
  channel varchar(50) NOT NULL default '',
  pattern varchar(32) NOT NULL default '',
  reason varchar(64) default NULL,
  setby varchar(32) default NULL,
  twhen int(10) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,pattern,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_output`
--

CREATE TABLE oer_output (
  twhen int(10) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  message text NOT NULL
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_passwords`
--

CREATE TABLE oer_passwords (
  handle varchar(32) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  type tinyint(3) unsigned NOT NULL default '0',
  password varchar(32) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,channel,type,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_passwords_sync`
--

CREATE TABLE oer_passwords_sync (
  handle varchar(32) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  type tinyint(3) unsigned NOT NULL default '0',
  password varchar(32) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,channel,type,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_permbans`
--

CREATE TABLE oer_permbans (
  channel varchar(50) NOT NULL default '',
  mask varchar(64) NOT NULL default '',
  reason varchar(64) default NULL,
  setby varchar(32) default NULL,
  twhen int(10) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,mask,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_raw`
--

CREATE TABLE oer_raw (
  twhen int(10) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  message text NOT NULL
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_scripts`
--

CREATE TABLE oer_scripts (
  filename varchar(64) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (filename,ident),
  UNIQUE KEY filename (filename)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_seen`
--

CREATE TABLE oer_seen (
  channel varchar(50) NOT NULL default '',
  nick varchar(20) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  hostmask varchar(80) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  KEY channel (channel,nick,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_servers`
--

CREATE TABLE oer_servers (
  hostname varchar(63) NOT NULL default '',
  port smallint(5) unsigned NOT NULL default '0',
  servermodes tinyint(3) unsigned NOT NULL default '0',
  pingfreq tinyint(3) unsigned NOT NULL default '0',
  protopers tinyint(3) unsigned NOT NULL default '0',
  linenoise tinyint(3) unsigned NOT NULL default '0',
  password varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  id tinyint(3) unsigned NOT NULL auto_increment,
  PRIMARY KEY  (hostname,port,servermodes,pingfreq,protopers,linenoise,ident),
  KEY id (id)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_services`
--

CREATE TABLE oer_services (
  host varchar(63) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (host,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_timestamps`
--

CREATE TABLE oer_timestamps (
  ttype varchar(32) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  tchannel varchar(50) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (ttype,tchannel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_timestamps_sync`
--

CREATE TABLE oer_timestamps_sync (
  ttype varchar(32) NOT NULL default '',
  twhen int(10) unsigned NOT NULL default '0',
  tchannel varchar(50) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (ttype,tchannel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_topics`
--

CREATE TABLE oer_topics (
  channel varchar(50) NOT NULL default '',
  pos tinyint(3) unsigned NOT NULL default '0',
  twhen int(10) unsigned NOT NULL default '0',
  setby varchar(32) NOT NULL default '',
  message varchar(255) NOT NULL default '',
  ident varchar(20) NOT NULL default ''
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_trusted`
--

CREATE TABLE oer_trusted (
  host varchar(63) NOT NULL default '',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (host,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_users`
--

CREATE TABLE oer_users (
  handle varchar(32) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  flags varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,channel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_users_sync`
--

CREATE TABLE oer_users_sync (
  handle varchar(32) NOT NULL default '',
  channel varchar(50) NOT NULL default '',
  flags varchar(32) default NULL,
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (handle,channel,ident)
) TYPE=MyISAM PACK_KEYS=1;

--
-- Table structure for table `oer_wordbks`
--

CREATE TABLE oer_wordbks (
  channel varchar(50) NOT NULL default '',
  pattern varchar(32) NOT NULL default '',
  reason varchar(64) default NULL,
  setby varchar(32) default NULL,
  twhen int(10) unsigned NOT NULL default '0',
  ident varchar(20) NOT NULL default '',
  PRIMARY KEY  (channel,pattern,ident)
) TYPE=MyISAM PACK_KEYS=1;

