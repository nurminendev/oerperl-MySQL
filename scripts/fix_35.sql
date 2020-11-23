CREATE TABLE oer_autheds (
  ttype VARCHAR(32) NOT NULL,
  twhen INT UNSIGNED NOT NULL,
  thandle VARCHAR(32) NOT NULL,
  tchannel VARCHAR(50) NOT NULL,
  thostmask VARCHAR(80) NOT NULL,
  ident VARCHAR(20) NOT NULL,
  PRIMARY KEY (ttype,twhen,thandle,tchannel,thostmask,ident)
) TYPE=MyISAM PACK_KEYS=1;
