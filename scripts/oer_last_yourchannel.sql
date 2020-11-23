# MySQL dump 8.12
#
# Host: localhost    Database: oer_quakenet
#--------------------------------------------------------
# Server version        3.23.32

#
# Table structure for table 'oer_last_yourchannel'
#

CREATE TABLE oer_last_yourchannel (
  twhen INT UNSIGNED NOT NULL,
  nick VARCHAR(20) NOT NULL,
  hostmask VARCHAR(80) NOT NULL,
  message TEXT NOT NULL,
  INDEX (nick)
) TYPE=MyISAM PACK_KEYS=1;
