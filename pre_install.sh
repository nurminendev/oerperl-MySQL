#!/bin/sh

CURRDIR=`pwd`
LOGTO="pre_install.log"

case `uname` in
  SunOS) ECHO="/usr/ucb/echo" ;;
  *) ECHO="echo" ;;
esac

readln () {
	$ECHO -n "$1"
        IFS='@' read ans || exit 1
        [ -z "$ans" ] && ans=$2
}

if [ $# -ne 1 ]; then
        $ECHO "wrong number of arguments"
        $ECHO ""
        exit 1
fi

rm -f $LOGTO 2>&1 >/dev/null
touch $LOGTO

$ECHO "Now configuring MySQL for oer+MySQL, see $LOGTO in case of errors"
$ECHO ""
$ECHO -n "Checking if I can find all the necesarry binaries... "

if [ ! -x $1/mysqladmin ]; then
	$ECHO "mysqladmin not found in $1"
	$ECHO ""
        exit 1
fi

if [ ! -x $1/mysql ]; then
	$ECHO "mysql not found in $1"
        $ECHO ""
        exit 1
fi

$ECHO "ok"
$ECHO ""

HOST="localhost"
DB="oerdb"
DBUSER="oer"
DBUSERPW=""

readln "Do you want to create the database (+ user) for oer+MySQL? [y/N] " "N"
CREATEDB=$ans

if [ "$CREATEDB" = "y" -o "$CREATEDB" = "Y" ]; then
    readln "Please specify the host where MySQL is running [$HOST] " "$HOST"
    HOST=$ans
    readln "Please specify the MySQL admin user [root] " "root"
    USER=$ans
    stty -$ECHO
    readln "Please specify the MySQL admin password [] " ""
    PW=$ans
    stty $ECHO
    $ECHO ""
    $ECHO ""
    $ECHO -n "Attempting to connect to $HOST as $USER... "

    if [ "$PW" = "" ]; then
	LOGTHIS=`$1/mysqladmin -h $HOST -u $USER ping 2>&1`
	RC=$?
	$ECHO $LOGTHIS >> $LOGTO
    else
	LOGTHIS=`$1/mysqladmin -h $HOST -u $USER -p$PW ping 2>&1`
	RC=$?
	$ECHO $LOGTHIS >> $LOGTO
    fi

    if [ $RC -eq 1 ]; then
	$ECHO "failed"
	$ECHO ""
        exit 1
    fi

    $ECHO "ok"
    $ECHO
    readln "What should your database be named [$DB] " "$DB"
    DB=$ans
    $ECHO ""
    $ECHO -n "Attempting to create database... "

    if [ "$PW" = "" ]; then
	LOGTHIS=`$1/mysqladmin -h $HOST -u $USER create $DB 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    else
	LOGTHIS=`$1/mysqladmin -h $HOST -u $USER -p$PW create $DB 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    fi

    if [ $RC -eq 1 ]; then
	$ECHO "failed"
        $ECHO ""
	$ECHO "Check that the database hasn't already been created"
        $ECHO "If it has, either drop it or skip this phase"
	$ECHO ""
        exit 1
    fi

    $ECHO "ok"
    $ECHO ""

    readln "What database user will be granted rights to $DB [$DBUSER] " "$DBUSER"
    DBUSER=$ans
    stty -$ECHO
    readln "Please specify password for $DBUSER [] " ""
    DBUSERPW=$ans
    stty $ECHO
    $ECHO ""
    HOSTNAME=`hostname`
    GRANTFOR="$HOSTNAME localhost"
    $ECHO ""
    for i in $GRANTFOR; do
	$ECHO -n "Attempting to grant rights to $DBUSER coming from $i... "

	if [ "$PW" = "" ]; then

	    if [ "$DBUSERPW" = "" ]; then
		LOGTHIS=`( $ECHO "GRANT ALL PRIVILEGES ON $DB.* TO $DBUSER@$i" | $1/mysql -h $HOST -u $USER ) 2>&1`
		RC=$?
		$ECHO $LOGTHIS >> $LOGTO
	    else
		LOGTHIS=`( $ECHO "GRANT ALL PRIVILEGES ON $DB.* TO $DBUSER@$i IDENTIFIED BY '$DBUSERPW'" | $1/mysql -h $HOST -u $USER ) 2>&1`
		RC=$?
		$ECHO $LOGTHIS >> $LOGTO
	    fi

	else

	    if [ "$DBUSERPW" = "" ]; then
		LOGTHIS=`( $ECHO "GRANT ALL PRIVILEGES ON $DB.* TO $DBUSER@$i" | $1/mysql -h $HOST -u $USER -p$PW ) 2>&1`
		RC=$?
		$ECHO $LOGTHIS >> $LOGTO
	    else
		LOGTHIS=`( $ECHO "GRANT ALL PRIVILEGES ON $DB.* TO $DBUSER@$i IDENTIFIED BY '$DBUSERPW'" | $1/mysql -h $HOST -u $USER -p$PW ) 2>&1`
		RC=$?
		$ECHO $LOGTHIS >> $LOGTO
	    fi

	fi

	if [ $RC -eq 1 ]; then
	    $ECHO "failed"
	    $ECHO ""
	    exit 1
	fi

	$ECHO "ok"
	$ECHO ""
    done
    $ECHO -n "Attempting to FLUSH PRIVILEGES... "

    if [ "$PW" = "" ]; then
	LOGTHIS=`( $ECHO "FLUSH PRIVILEGES" | $1/mysql -h $HOST -u $USER ) 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    else
	LOGTHIS=`( $ECHO "FLUSH PRIVILEGES" | $1/mysql -h $HOST -u $USER -p$PW ) 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    fi

    if [ $RC -eq 1 ]; then
	$ECHO "failed"
	$ECHO ""
	exit 1
    fi

    $ECHO "ok"
    $ECHO ""
else
    $ECHO ""
    readln "Please specify the host where MySQL is running [$HOST] " "$HOST"
    HOST=$ans
    readln "Please specify the database [$DB] " "$DB"
    DB=$ans
    readln "Please specify the database user [$DBUSER] " "$DBUSER"
    DBUSER=$ans
    stty -$ECHO
    readln "Please specify password for $DBUSER [] " ""
    DBUSERPW=$ans
    stty $ECHO
    $ECHO ""
    $ECHO ""
fi

if [ "$CREATEDB" = "y" -o "$CREATEDB" = "Y" ]; then
    readln "Do you want to create the initial tables for oer+MySQL? [Y/n] " "Y"
else
    readln "Do you want to create the initial tables for oer+MySQL? [y/N] " "N"
fi

CREATETABLES=$ans

if [ "$CREATETABLES" = "y" -o "$CREATETABLES" = "Y" ]; then
    if [ "$CREATEDB" = "n" -o "$CREATEDB" = "N" ]; then
	stty $ECHO
	$ECHO ""
	$ECHO ""
	$ECHO -n "Attempting to connect to $HOST as $DBUSER... "
	
	if [ "$DBUSERPW" = "" ]; then
	    LOGTHIS=`( $1/mysqladmin -h $HOST -u $DBUSER ping ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	else
	    LOGTHIS=`( $1/mysqladmin -h $HOST -u $DBUSER -p$DBUSERPW ping ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	fi
	
	if [ $RC -eq 1 ]; then
	    $ECHO "failed"
	    $ECHO ""
	    exit 1
	fi
	
	$ECHO "ok"
	$ECHO ""
    fi

    $ECHO ""
    $ECHO -n "Creating initial tables... "

    if [ "$DBUSERPW" = "" ]; then
	LOGTHIS=`$1/mysql -h $HOST -u $DBUSER $DB < $CURRDIR/scripts/initial.sql 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    else
	LOGTHIS=`$1/mysql -h $HOST -u $DBUSER -p$DBUSERPW $DB < $CURRDIR/scripts/initial.sql 2>&1`
	RC=$?
        $ECHO $LOGTHIS >> $LOGTO
    fi
    
    if [ $RC -eq 1 ]; then
	$ECHO "failed"
	$ECHO ""
	$ECHO "Check that the tables haven't already been created"
	$ECHO "If they have, either drop them or skip this phase"
	$ECHO ""
	exit 1
    fi

    $ECHO "ok"
    $ECHO ""
fi

if [ "$CREATEDB" = "y" -o "$CREATEDB" = "Y" ]; then
    readln "Do you want to create the initial configuration for oer+MySQL? [Y/n] " "Y"
else
    readln "Do you want to create the initial configuration for oer+MySQL? [y/N] " "N"
fi

CREATECONF=$ans

if [ "$CREATECONF" = "y" -o "$CREATECONF" = "Y" ]; then
    $ECHO ""
    $ECHO "oer+MySQL configuration follows"
    $ECHO ""
    readln "Please specify bot ident (identifies this oer+MySQL in the database) [oer] " "oer"
    HANDLE=$ans
    readln "Please specify default IRC nick [oer] " "oer"
    NICK=$ans
    readln "Please specify alternate IRC nick [oeroer] " "oeroer"
    ALTNICK=$ans
    readln "Please specify user ID [oer] " "oer"
    USERID=$ans
    readln "Please specify user modes [+i] " "+i"
    USERMODES=$ans
    readln "Please specify bot flags [fn] " "fn"
    BOTFLAGS=$ans
    readln "Please specify command prefix [!] " "!"
    PREFIX=$ans
    readln "Please specify IRC REALNAME [http://oer.equnet.org] " "http://oer.equnet.org"
    REALNAME=$ans
    readln "Please specify optional virtual host [] " ""
    VIRTHOST=$ans
    readln "Please specify optional proxy setup string [] " ""
    PROXYSETUP=$ans
    
    INSERT_CONF="INSERT INTO oer_conf VALUES ('$NICK','$ALTNICK','$USERID','$USERMODES','$BOTFLAGS','$PREFIX','$REALNAME','$VIRTHOST',NULL,NULL,NULL,'$PROXYSETUP','$HANDLE')"
	
    readln "Please specify bot admin handle [EQU] " "EQU"
    ADMIN=$ans
    readln "Please specify bot admin flags (n = unremovable, d = dynamic) [n] " "n"
    ADMINFLAGS=$ans
    ISDYN=`$ECHO "$ADMINFLAGS" | sed 's/[0-9A-Za-ce-z]//g'`
    if [ "$ISDYN" = "d" ]; then
        stty -$ECHO
        readln "Please specify your password [] " ""
        PW=$ans
        stty $ECHO
        $ECHO ""
        $ECHO ""
        $ECHO -n "Crypting password ... "
        CRYPT=`./mycrypt $PW`

        if [ $RC -eq 1 ]; then
                $ECHO "failed"
                $ECHO ""
            exit 1
        fi

        $ECHO "ok"
        $ECHO ""
    fi
    
    INSERT_ADMINS="INSERT INTO oer_admins VALUES ('$ADMIN','$ADMINFLAGS','$HANDLE')"

    readln "Please specify bot admin hostmask [equ@*.equnet.org] " "equ@*.equnet.org"
    ADMINHOSTMASK=$ans
    INSERT_HOSTMASKS="INSERT INTO oer_hostmasks VALUES ('$ADMINHOSTMASK','-',1,'$ADMIN','$HANDLE')"

    if [ "$ISDYN" = "d" ]; then
	INSERT_PASSWORDS="INSERT INTO oer_passwords VALUES ('$ADMIN','',2,'$CRYPT','$HANDLE')"
    fi

    readln "Please specify default IRC server to connect to [irc.equnet.org] " "irc.equnet.org"
    SERVER=$ans
    readln "Please specify port [6667] " "6667"
    PORT=$ans
    readln "Please specify amount of allowed server modes [6] " "6"
    MODES=$ans
    readln "Please specify ping frequency in seconds on $SERVER [120] " "120"
    PINGFREQ=$ans
    readln "Please specify whether IRC operators are protected on $SERVER (0 = no, 1 = yes) [0] " "0"
    PROT=$ans
    readln "Please specify whether you want line noise for $SERVER (0 = off, >>1 = on) [0] " "0"
    NOISE=$ans
    readln "Please specify optional server password for $SERVER [] " ""
    SERVERPW=$ans

    INSERT_SERVERS="INSERT INTO oer_servers VALUES ('$SERVER','$PORT','$MODES','$PINGFREQ','$PROT','$NOISE','$SERVERPW','$HANDLE',NULL)"

    $ECHO ""
    $ECHO -n "Importing data into database... "

    for i in 1 2 3 4 5; do
	case $i in
	    1)
	    INSERT_COMMAND=$INSERT_CONF
	    ;;
	    2)
	    INSERT_COMMAND=$INSERT_ADMINS
	    ;;
	    3)
	    INSERT_COMMAND=$INSERT_HOSTMASKS
	    ;;
	    4)
	    INSERT_COMMAND=$INSERT_SERVERS
	    ;;
	    5)
	    INSERT_COMMAND=$INSERT_PASSWORDS
	    ;;
	esac

	if [ $i = "5" -a "$ISDYN" != "d" ]; then
	    continue
	fi

	if [ "$DBUSERPW" = "" ]; then
	    LOGTHIS=`( $ECHO "$INSERT_COMMAND" | $1/mysql -h $HOST -u $DBUSER $DB ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	else
	    LOGTHIS=`( $ECHO "$INSERT_COMMAND" | $1/mysql -h $HOST -u $DBUSER -p$DBUSERPW $DB ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	fi
    
	if [ $RC -eq 1 ]; then
	    $ECHO "failed"
	    $ECHO ""
	    $ECHO "Check that the settings haven't already been stored into the database"
	    $ECHO "If they have, either delete them or skip this phase"
	    $ECHO ""
	    exit 1
	fi
	
    done

    KICKREASONS=`wc -l scripts/kickreasons.txt | awk '{print $1}'`
    KICKREASONS=`expr $KICKREASONS + 1`
    COUNTER=1

    while [ $COUNTER -ne $KICKREASONS ]; do
	REASON=`head -$COUNTER scripts/kickreasons.txt | tail -1`
	REASON2=`echo $REASON | sed "s/'/''/"`
        NREASON="INSERT INTO oer_kickreasons VALUES ('$REASON2', '$HANDLE')"
	if [ "$DBUSERPW" = "" ]; then
	    LOGTHIS=`( $ECHO "$NREASON" | $1/mysql -h $HOST -u $DBUSER $DB ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	else
	    LOGTHIS=`( $ECHO "$NREASON" | $1/mysql -h $HOST -u $DBUSER -p$DBUSERPW $DB ) 2>&1`
	    RC=$?
	    $ECHO $LOGTHIS >> $LOGTO
	fi

	if [ $RC -eq 1 ]; then
	    $ECHO "failed"
	    $ECHO ""
	    $ECHO "Check that the settings haven't already been stored into the database"
	    $ECHO "If they have, either delete them or skip this phase"
	    $ECHO ""
	    exit 1
	fi

	COUNTER=`expr $COUNTER + 1`
    done

    $ECHO "ok"
    $ECHO ""

    $ECHO -n "Creating oer+MySQL.conf... "
    $ECHO "mysql::$HOST::$DB::$DBUSER::$DBUSERPW" > sample-configuration/oer+MySQL.conf

    RC=$?

    if [ $RC -eq 1 ]; then
	$ECHO "failed"
	$ECHO ""
	exit 1
    fi

    $ECHO "ident::$HANDLE" >> sample-configuration/oer+MySQL.conf

    RC=$?

    if [ $RC -eq 1 ]; then
	$ECHO "failed"
	$ECHO ""
	exit 1
    fi

    $ECHO "ok"
fi

$ECHO ""

exit 0

