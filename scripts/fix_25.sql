ALTER TABLE conf ADD qhost VARCHAR(80);
ALTER TABLE conf ADD qname VARCHAR(32);
ALTER TABLE conf ADD qpassword VARCHAR(32);
ALTER TABLE floodvars ADD f_nickflood_expire SMALLINT UNSIGNED NOT NULL DEFAULT '7200' AFTER f_chars;
ALTER TABLE floodvars ADD f_nickflood_changes SMALLINT UNSIGNED NOT NULL DEFAULT '5' AFTER f_nickflood_expire;
UPDATE floodvars SET type = 'normals' WHERE type = 'lamers';
