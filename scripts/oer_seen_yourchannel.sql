# MySQL dump 8.12
#
# Host: localhost    Database: oer_quakenet
#--------------------------------------------------------
# Server version        3.23.32

#
# Table structure for table 'oer_seen_yourchannel'
#

CREATE TABLE oer_seen_yourchannel (
  twhen INT UNSIGNED NOT NULL,
  nick CHAR(20) NOT NULL,
  hostmask CHAR(80) NOT NULL,
  INDEX (nick)
) TYPE=MyISAM PACK_KEYS=1;

