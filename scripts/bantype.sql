#
# Table structure for table 'bantype'
#

CREATE TABLE bantype (
  channel varchar(64) default '',
  type int(11) NOT NULL default '1',
  ident varchar(64) NOT NULL default ''
) TYPE=MyISAM;

