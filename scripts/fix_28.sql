ALTER TABLE channels DROP chanlimit;
ALTER TABLE channels ADD chanlimit VARCHAR(32) DEFAULT '' AFTER chankey;
