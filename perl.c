/*

oer and oer+MySQL perl scripting support

Copyright (C) 2004 Riku Nurminen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "oer+MySQL-common.h"
#include "oer+MySQL.h"
#include "ds.h"
#include "misc.h"

static XS(oerperl_add_event_handler);
static XS(oerperl_register_startup_callback);
static XS(oerperl_register_shutdown_callback);
static XS(oerperl_oer_debug);
static XS(oerperl_sendto);
static XS(oerperl_add_command_handler);
static XS(oerperl_add_timer);
static XS(oerperl_is_valid_channel);
static void xs_init(pTHX);
int oerperl_init(void);
void oerperl_shutdown(void);
int oerperl_loadscript(struct script *);
void oerperl_deletescript(struct script *);
static int execute_perl(SV *, char *, struct script *, int, char *, int, int, int);
static int check_eventtype(char *);
int oerperl_call_eventhandlers(char *, char *, char *, char *);
int oerperl_call_commandhandlers(char *, char *, char *, char *, char *, int);
static void oerperl_get_package_name(char *, char *, int);
void oerperl_process_scripttimers(void);

EXTERN_C void boot_DynaLoader(pTHX_ CV *cv);

static struct script *active_script;

#define OERPERL_LOADSCRIPT      "OerPerl::load_script"
#define OERPERL_DELSCRIPT       "OerPerl::delete_script"
#define OERPERL_GET_PKG_NAME    "OerPerl::get_package_name"
#define OERPERL_PERL_WARNINGS   1 /* 1 = same as running perl with -w, 0 = without */
#define OERPERL_SCRIPTDIR       "scripts"

/* work around broken dXSARGS in perl 5.6 */
#define MY_dXSARGS \
    dSP; \
    dMARK; \
    long ax = mark - PL_stack_base + 1; \
    long items = sp - mark

/*
** OerPerl::add_event_handler(event_type, callback)
*/
static XS(oerperl_add_event_handler)
{
    char *eventtype;
    char *callbackstr;
    char package[BIGSTRINGLEN + 1];
    char fullcb[WRITE_BUFFER_LENGTH + 1];
    MY_dXSARGS;
    if(items != 2) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_eventhandler()->expected 2 arguments, called with %d\n", items);
#endif
        XSRETURN_EMPTY;
    } else if(active_script == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_eventhandler()->called with active_script == NULL\n");
#endif
        XSRETURN_EMPTY;
    } else {
        eventtype = SvPV_nolen(ST(0));
        if(!check_eventtype(eventtype)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_eventhandler()->invalid eventtype (%s)\n", eventtype);
#endif
            XSRETURN_EMPTY;
        }
        callbackstr = SvPV_nolen(ST(1));
        if(!strlen(callbackstr)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_eventhandler()->invalid callback\n");
#endif
        }
        oerperl_get_package_name(active_script->filename, package, BIGSTRINGLEN);
        snprintf(fullcb, WRITE_BUFFER_LENGTH, "%s::%s", package, callbackstr);
        if(addneweventhandler(active_script, eventtype, sv_2mortal(newSVpvn(fullcb, strlen(fullcb)))) == NULL) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_eventhandler()->addneweventhandler() failed\n");
#endif
            XSRETURN_EMPTY;
        }
    }
    XSRETURN_EMPTY;
}

/*
** OerPerl::register_startup_callback(callback)
*/
static XS(oerperl_register_startup_callback)
{
    char *callbackstr;
    char package[BIGSTRINGLEN + 1];
    char fullcb[WRITE_BUFFER_LENGTH + 1];
    MY_dXSARGS;
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_register_startup_callback()\n");
#endif
    if(items != 1) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_register_startup_callback()->expected 1 argument, called with %d\n", items);
#endif
        XSRETURN_EMPTY;
    } else if(active_script == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_register_startup_callback()->called with active_script == NULL\n");
#endif
        XSRETURN_EMPTY;
    } else {
        callbackstr = SvPV_nolen(ST(0));
        if(!strlen(callbackstr)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_register_startup_callback()->invalid callback\n");
#endif
            XSRETURN_EMPTY;
        }
        oerperl_get_package_name(active_script->filename, package, BIGSTRINGLEN);
        snprintf(fullcb, WRITE_BUFFER_LENGTH, "%s::%s", package, callbackstr);
        active_script->startup_callback = sv_2mortal(newSVpvn(fullcb, strlen(fullcb)));
        SvREFCNT_inc(active_script->startup_callback);
    }
    XSRETURN_EMPTY;
}

/*
** OerPerl::register_shutdown_callback(callback)
*/
static XS(oerperl_register_shutdown_callback)
{
    char *callbackstr;
    char package[BIGSTRINGLEN + 1];
    char fullcb[WRITE_BUFFER_LENGTH + 1];
    MY_dXSARGS;
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_register_shutdown_callback()\n");
#endif
    if(items != 1) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_register_shutdown_callback()->expected 1 argument, called with %d\n", items);
#endif
        XSRETURN_EMPTY;
    } else if(active_script == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_register_shutdown_callback()->called with active_script == NULL\n");
#endif
        XSRETURN_EMPTY;
    } else {
        callbackstr = SvPV_nolen(ST(0));
        if(!strlen(callbackstr)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_register_shutdown_callback()->invalid callback\n");
#endif
            XSRETURN_EMPTY;
        }
        oerperl_get_package_name(active_script->filename, package, BIGSTRINGLEN);
        snprintf(fullcb, WRITE_BUFFER_LENGTH, "%s::%s", package, callbackstr);
        active_script->shutdown_callback = sv_2mortal(newSVpvn(fullcb, strlen(fullcb)));
        SvREFCNT_inc(active_script->shutdown_callback);
    }
    XSRETURN_EMPTY;
}

/*
** OerPerl::oer_debug(debuglevel, message)
*/
static XS(oerperl_oer_debug)
{
    int debuglevel;
    char *msg;
    MY_dXSARGS;
    if(items != 2) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_debug()->expected 2 arguments, called with %d\n", items);
#endif
    } else {
        debuglevel = (int) SvIV(ST(0));
        if(debuglevel < 0 || debuglevel > 9) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_debug()->invalid debuglevel %d (must be 0-9)\n", debuglevel);
#endif
            XSRETURN_EMPTY;
        }
        msg = SvPV_nolen(ST(1));
        oer_debug(debuglevel, msg);
        XSRETURN_EMPTY;
    }
}

/*
** OerPerl::sendto(target, target_type, message)
*/
static XS(oerperl_sendto)
{
    char *target;
    int tochan;
    char *message;
    MY_dXSARGS;
    if(items != 3) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_sendto()->expected 3 arguments, called with %d\n", items);
#endif
    } else {
        target = SvPV_nolen(ST(0));
        if(!strlen(target)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_sendto()->invalid target, must be atleast 1 character long\n");
#endif
            XSRETURN_EMPTY;
        }
        tochan = (int) SvIV(ST(1));
        if(tochan != 0 && tochan != 1) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_sendto()->invalid target_type %d, must be either OerPerl::CHANNEL (1) or OerPerl::USER (0)\n", tochan);
#endif
            XSRETURN_EMPTY;
        }
        message = SvPV_nolen(ST(2));
        sendreply(target, tochan, 0, OER_TIMED_PRIORITY_NORMAL, message);
        /* must count this as server activity or otherwise oer will quit with "stoned server" if e.g.
         a script timer uses sendto() every 15 seconds or so */
        mystate->current_server->lastping = mystate->now;
        XSRETURN_EMPTY;
    }
}

/*
** OerPerl::add_command_handler(command, callback)
*/
static XS(oerperl_add_command_handler)
{
    char *command;
    char *callbackstr;
    char package[BIGSTRINGLEN + 1];
    char fullcb[WRITE_BUFFER_LENGTH + 1];
    MY_dXSARGS;
    if(items != 2) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_commandhandler()->expected 2 arguments, called with %d\n", items);
#endif
        XSRETURN_EMPTY;
    } else if(active_script == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_commandhandler()->called with active_script == NULL\n");
#endif
        XSRETURN_EMPTY;
    } else {
        command = SvPV_nolen(ST(0));
        if(!strlen(command) || strlen(command) > TINYSTRINGLEN) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_commandhandler()->command %s is too short (min. 1 character) or too long (max. %d characters)\n", command, TINYSTRINGLEN);
#endif
            XSRETURN_EMPTY;
        }
        callbackstr = SvPV_nolen(ST(1));
        if(!strlen(callbackstr)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_commandhandler()->invalid callback\n");
#endif
            XSRETURN_EMPTY;
        }
        oerperl_get_package_name(active_script->filename, package, BIGSTRINGLEN);
        snprintf(fullcb, WRITE_BUFFER_LENGTH, "%s::%s", package, callbackstr);
        if(addnewcommandhandler(active_script, command, sv_2mortal(newSVpvn(fullcb, strlen(fullcb)))) == NULL) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_commandhandler()->addnewcommandhandler() failed\n");
#endif
            XSRETURN_EMPTY;
        }
    }
    XSRETURN_EMPTY;
}

/*
** OerPerl::add_timer(interval, callback)
*/
static XS(oerperl_add_timer)
{
    time_t interval;
    char *callbackstr;
    char package[BIGSTRINGLEN + 1];
    char fullcb[WRITE_BUFFER_LENGTH + 1];
    MY_dXSARGS;
    if(items != 2) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_timer()->expected 2 arguments, called with %d\n", items);
#endif
        XSRETURN_EMPTY;
    } else if(active_script == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_add_timer()->called with active_script == NULL\n");
#endif
        XSRETURN_EMPTY;
    } else {
        interval = (time_t) SvIV(ST(0));
        if(interval < 1) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_timer()->invalid interval, must be atleast 1 second\n");
#endif
            XSRETURN_EMPTY;
        }
        callbackstr = SvPV_nolen(ST(1));
        if(!strlen(callbackstr)) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_timer()->invalid callback\n");
#endif
            XSRETURN_EMPTY;
        }
        oerperl_get_package_name(active_script->filename, package, BIGSTRINGLEN);
        snprintf(fullcb, WRITE_BUFFER_LENGTH, "%s::%s", package, callbackstr);
        if(addnewscripttimer(active_script, interval, sv_2mortal(newSVpvn(fullcb, strlen(fullcb)))) == NULL) {
#ifdef OER_DEBUG
            oer_debug(OER_DEBUG_ERROR, "oerperl_add_timer()->addnewscripttimer() failed\n");
#endif
            XSRETURN_EMPTY;
        }
    }
    XSRETURN_EMPTY;
}

/*
** OerPerl::is_valid_channel(channel)
*/
static XS(oerperl_is_valid_channel)
{
    char *channel;
    MY_dXSARGS;
    if(items != 1) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_is_valid_channel()->expected 1 argument, called with %d\n", items);
#endif
    } else {
        channel = SvPV_nolen(ST(0));
        if(!strlen(channel)) {
            XSRETURN_NO;
        }
        if(isvalidchannel(channel)) {
            XSRETURN_YES;
        } else {
            XSRETURN_NO;
        }
    }
}

static void xs_init(pTHX)
{
    char *file = __FILE__;
    char *me = "OerPerl";

    dXSUB_SYS;
    /* DynaLoader is a special case */
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
    newXS("OerPerl::add_event_handler", oerperl_add_event_handler, me);
    newXS("OerPerl::register_startup_callback", oerperl_register_startup_callback, me);
    newXS("OerPerl::register_shutdown_callback", oerperl_register_shutdown_callback, me);
    newXS("OerPerl::oer_debug", oerperl_oer_debug, me);
    newXS("OerPerl::sendto", oerperl_sendto, me);
    newXS("OerPerl::add_command_handler", oerperl_add_command_handler, me);
    newXS("OerPerl::add_timer", oerperl_add_timer, me);
    newXS("OerPerl::is_valid_channel", oerperl_is_valid_channel, me);
}

int oerperl_init(void)
{
    char *perl_args[] = { "", "-e", "0", "-w" };
    SV *eval;
    HV *stash;
    STRLEN n_a;
    char *locptr;
    const char glue[] = {
        "use strict;\n"
        "\n"
        "$SIG{__WARN__} = sub\n"
        "{\n"
        "    local $, = \"\\n\";\n"
        "    OerPerl::oer_debug(OerPerl::DEBUG_WARNING, \"OerPerl warning: @_\");\n"
        "    return 1;"
        "};\n"
        "\n"
        "sub OerPerl::get_package_name\n"
        "{\n"
        "    my $string = shift @_;\n"
        "    $string =~ s/([^A-Za-z0-9])/sprintf(\"_%2x\", unpack(\"C\", $1))/eg;\n"
        "    $string =~ s|/(\\d)|sprintf(\"/_%2x\", unpack(\"C\", $1))|eg;\n"
        "    return \"OerPerl::\" . $string;\n"
        "}\n"
        "\n"
        "sub OerPerl::load_script\n"
        "{\n"
        "    my $filename = shift @_;\n"
        "    my $scriptfile = $filename; \n"
        "    my $package;\n"
        "    my $debuglevel;\n"
        "    my $scriptdir = OerPerl::OERPERL_SCRIPTDIR;\n"
        "    local *FH;\n"
        "    if(OerPerl::OERPERL_SCRIPTDIR) {\n"
        "        $scriptfile =~ s/^@{[OerPerl::OERPERL_SCRIPTDIR]}\\///g;\n"
        "    }\n"
        "    $package = OerPerl::get_package_name($scriptfile);\n"
        "    if(open(FH, $filename)) {\n"
        "        local($/) = undef;\n"
        "        my $sub = <FH>;\n"
        "        close(FH);\n"
        "\n"
        "        my $eval = qq { package $package; $sub; };\n"
        "        {\n"
        "            my($filename, $package, $sub);\n"
        "            eval $eval;\n"
        "        }\n"
        "        if($@) {\n"
        "            OerPerl::oer_debug(OerPerl::DEBUG_ERROR, \"OerPerl::load_script()->error loading $filename: $@\\n\");\n"
        "            return 0;\n"
        "        }\n"
        "    } else {\n"
        "        OerPerl::oer_debug(OerPerl::DEBUG_ERROR, \"OerPerl::load_script()->error opening $filename: $!\\n\");\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "    eval { $package; };\n"
        "    if($@) {\n"
        "        OerPerl::oer_debug(OerPerl::DEBUG_ERROR, \"OerPerl::load_script()->error loading $filename: $@\\n\");\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "    return 1;\n"
        "}\n"
        "\n"
        "sub OerPerl::scrub_package\n"
        "{\n"
        "    no strict 'refs';\n"
        "    my $pkg = shift;\n"
        "    unless($pkg =~ /^main::.*::$/) {\n"
        "        $pkg = \"main$pkg\" if $pkg =~ /^::/;\n"
        "        $pkg = \"main::$pkg\" unless $pkg =~ /^main::/;\n"
        "        $pkg .= '::' unless $pkg =~ /::$/;\n"
        "    }\n"
        "\n"
        "    my($stem, $leaf) = $pkg =~ m/(.*::)(\\w+::)$/;\n"
        "    my $stem_symtab = *{$stem}{HASH};\n"
        "    return unless defined $stem_symtab and exists $stem_symtab->{$leaf};\n"
        "\n"
        "    my $leaf_symtab = *{$stem_symtab->{$leaf}}{HASH};\n"
        "    foreach my $name (keys %$leaf_symtab) {\n"
        "        undef *{$pkg . $name};\n"
        "    }\n"
        "\n"
        "    # delete the symbol table\n"
        "    %$leaf_symtab = ();\n"
        "    delete $stem_symtab->{$leaf};\n"
        "}\n"
        "\n"
        "sub OerPerl::delete_script\n"
        "{\n"
        "    my $filename = shift @_;\n"
        "    my $package = OerPerl::get_package_name($filename);\n"
        "    OerPerl::scrub_package($package);\n"
        "}\n"
    };
    active_script = NULL;
    n_a = 0;
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_init()\n");
#endif
#ifdef HAVE_LOCALE_H
    /* hack from the xchat perl plugin (is this really necessary?) */
    if((locptr = setlocale(LC_NUMERIC, "C")) == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_init()->setlocale() failed\n");
#endif
    }
#endif
    if((my_perl = perl_alloc()) == NULL) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_NOISE, "oerperl_init()->failed to allocate memory for perl interpreter\n");
#endif
        return 0;
    }
    PL_perl_destruct_level = 1;
    perl_construct(my_perl);
    if(OERPERL_PERL_WARNINGS)
        perl_parse(my_perl, xs_init, 4, perl_args, NULL);
    else
        perl_parse(my_perl, xs_init, 3, perl_args, NULL);
    if((stash = gv_stashpv("OerPerl", 0)) == NULL) {
#ifdef DEBUG
        oer_debug(OER_DEBUG_FATAL, "xs_init()->gv_stashpv() failed\n");
#endif
        exit(EXIT_FAILURE);
    }
    newCONSTSUB(stash, "DEBUG_NONE", newSViv(OER_DEBUG_NONE));
    newCONSTSUB(stash, "DEBUG_FATAL", newSViv(OER_DEBUG_FATAL));
    newCONSTSUB(stash, "DEBUG_ERROR", newSViv(OER_DEBUG_ERROR));
    newCONSTSUB(stash, "DEBUG_WARNING", newSViv(OER_DEBUG_WARNING));
    newCONSTSUB(stash, "DEBUG_INFO", newSViv(OER_DEBUG_INFO));
    newCONSTSUB(stash, "DEBUG_NOISE", newSViv(OER_DEBUG_NOISE));
    newCONSTSUB(stash, "DEBUG_FLOOD", newSViv(OER_DEBUG_FLOOD));
#ifdef OER_DEBUG
    newCONSTSUB(stash, "DEBUG", newSViv(1));
#else
    newCONSTSUB(stash, "DEBUG", newSViv(0));
#endif
    newCONSTSUB(stash, "CHANNEL", newSViv(1));
    newCONSTSUB(stash, "USER", newSViv(0));
    newCONSTSUB(stash, "BLOCK_OER_COMMAND", newSViv(OERPERL_BLOCK_OER_COMMAND));
    newCONSTSUB(stash, "NONE", newSViv(0));
    if(strlen(OERPERL_SCRIPTDIR)) {
        newCONSTSUB(stash, "OERPERL_SCRIPTDIR", newSVpvn(OERPERL_SCRIPTDIR, strlen(OERPERL_SCRIPTDIR)));
    } else {
        newCONSTSUB(stash, "OERPERL_SCRIPTDIR", newSViv(0));
    }
    eval = eval_pv(glue, TRUE);
    if(!SvTRUE(eval)) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_FATAL, "oerperl_init()->eval error: %s\n", SvPV(eval, n_a));
#endif
        return 0;
    }
    return 1;
}

void oerperl_shutdown(void)
{
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_shutdown()\n");
#endif
    if(my_perl) {
        perl_destruct(my_perl);
        perl_free(my_perl);
    }
}

int oerperl_loadscript(struct script *script)
{
    int retval;
    char stringbuffer[BIGSTRINGLEN + 1 + strlen(OERPERL_SCRIPTDIR) + 1];
    if(strlen(OERPERL_SCRIPTDIR)) {
        snprintf(stringbuffer, BIGSTRINGLEN + strlen(OERPERL_SCRIPTDIR), "%s/%s", OERPERL_SCRIPTDIR, script->filename);
    }
    retval = execute_perl(newSVpvn(OERPERL_LOADSCRIPT, strlen(OERPERL_LOADSCRIPT)), stringbuffer, script, 0, "", 0, 0, 1);
    if(retval == -1 || retval == 0) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_INFO, "oerperl_loadscript()->failed to load script %s\n", script->filename);
#endif
    } else if(SvTRUE(script->startup_callback)) {
        execute_perl(script->startup_callback, "", script, 0, "", 0, 0, 0);
    }
    return (retval == -1 || retval == 0) ? 0 : 1;
}

void oerperl_deletescript(struct script *script)
{
    struct eventhandler *eventhandler;
    struct commandhandler *commandhandler;
    struct scripttimer *timer;
    if(SvTRUE(script->shutdown_callback)) {
        execute_perl(script->shutdown_callback, "", script, 0, "", 0, 0, 0);
    }
    SvREFCNT_dec(script->shutdown_callback);
    execute_perl(newSVpvn(OERPERL_DELSCRIPT, strlen(OERPERL_DELSCRIPT)), script->filename, script, 0, "", 0, 0, 0);
    /* decref on all callbacks */
    eventhandler = script->eventhandlers;
    while(eventhandler != NULL) {
        SvREFCNT_dec(eventhandler->callback);
        eventhandler = eventhandler->next;
    }
    commandhandler = script->commandhandlers;
    while(commandhandler != NULL) {
        SvREFCNT_dec(commandhandler->callback);
        commandhandler = commandhandler->next;
    }
    timer = script->timers;
    while(timer != NULL) {
        SvREFCNT_dec(timer->callback);
        timer = timer->next;
    }
}

static int execute_perl(SV *function, char *args, struct script *script, int splitargs, char *splitdelim, int numsplits, int pushrest, int retvals2expect)
{
    int count;
    int splitcount;
    int retval = 0;
    SV *sv;
    int ppos, ppos2;
    char p[STRINGLEN + 1];
    SV *notused; /* keep compiler happy about POPs */

    dSP;
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "execute_perl()\n");
#endif
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    if(args && strcasecmp(args, "") != 0) {
        if(splitargs) {
            /* separate args by 'splitdelim' and push the first 'numsplits' args as separate variables */
            ppos = ppos2 = 0;
            splitcount = 0;
            while(1) {
                ppos = parse(args, ppos, splitdelim, p, STRINGLEN, 0);
                if(ppos == ppos2 || ppos < 0) {
                    break;
                }
                if(splitcount >= numsplits) {
                    break;
                }
                ppos2 = ppos;
                XPUSHs(newSVpvn(p, strlen(p)));
                splitcount++;
            }
            if(pushrest) {
                /* push rest of args as one variable */
                XPUSHs(newSVpvn(args + ppos2, strlen(args + ppos2)));
            }
        } else {
            /* push args as one variable */
            XPUSHs(newSVpvn(args, strlen(args)));
        }
    }
    PUTBACK;
    active_script = script;
    count = call_sv(function, (retvals2expect == 0) ? (G_EVAL | G_KEEPERR | G_DISCARD) : (G_EVAL | G_KEEPERR | G_SCALAR));
    active_script = NULL;
    SPAGAIN;

    sv = GvSV(gv_fetchpv("@", TRUE, SVt_PV));
    if(SvTRUE(sv)) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "execute_perl()->when calling %s: %s\n", SvPV_nolen(function), SvPV(sv, count));
#endif
        notused = POPs;
        retval = -1;
    } else if(count != retvals2expect) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_WARNING, "execute_perl()->expected %d return value(s) from %s, got %d\n", retvals2expect, SvPV_nolen(function), count);
#endif
    } else if(count > 0) {
        retval = POPi;
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return retval;
}

static int check_eventtype(char *string)
{
    char stringbuffer[WRITE_BUFFER_LENGTH + 1];
    int i, length;
    int eventnum;

    strncpy(stringbuffer, string, WRITE_BUFFER_LENGTH);
    if(!strcasecmp(stringbuffer, "JOIN") ||
       !strcasecmp(stringbuffer, "PRIVMSG") ||
       !strcasecmp(stringbuffer, "QUIT") ||
       !strcasecmp(stringbuffer, "PART") ||
       !strcasecmp(stringbuffer, "NICK") ||
       !strcasecmp(stringbuffer, "KILL") ||
       !strcasecmp(stringbuffer, "KICK") ||
       !strcasecmp(stringbuffer, "NOTICE") ||
       !strcasecmp(stringbuffer, "MODE") ||
       !strcasecmp(stringbuffer, "TOPIC") ||
       !strcasecmp(stringbuffer, "INVITE")) {
        return 1;
    } else {
        /* must be numeric, check that each character is a digit */
        length = strlen(stringbuffer);
        for(i = 0; i < length; i++) {
            if(!isdigit(stringbuffer[i])) {
                /* non-digit found -> invalid eventtype */
                return 0;
            }
        }
        eventnum = atoi(stringbuffer);
        if(eventnum >= 0 && eventnum <= 999)
            return 1;
    }
    return 0;
}

int oerperl_call_eventhandlers(char *prefix, char *command, char *params, char *trailing)
{
    int num_sendout = 0;
    struct script *script;
    struct eventhandler *eventhandler;
    char stringbuffer[READ_BUFFER_LENGTH + 1];

#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_NOISE, "oerperl_call_eventhandlers(\"%s\", \"%s\", \"%s\", \"%s\")\n", (prefix) ? prefix : "(null)", (command) ? command : "(null)", (params) ? params : "(null)", (trailing) ? trailing : "(null)");
#endif
    script = mystate->scripts;
    while(script != NULL) {
        eventhandler = script->eventhandlers;
        while(eventhandler != NULL) {
            if(!strcasecmp(eventhandler->eventtype, command)) {
                snprintf(stringbuffer, READ_BUFFER_LENGTH, "%s %s %s %s", (prefix) ? prefix : "(null)", (command) ? command : "(null)", (params) ? params : "(null)", (trailing) ? trailing : "(null)");
                if(execute_perl(eventhandler->callback, stringbuffer, script, 1, " ", 3, 1, 0) == -1) {
#ifdef OER_DEBUG
                    oer_debug(OER_DEBUG_ERROR, "oerperl_call_eventhandlers()->execute_perl() failed for eventhandler %s in script %s\n", SvPV_nolen(eventhandler->callback), script->filename);
#endif
                } else {
                    num_sendout++;
                }
                memset(stringbuffer, 0, READ_BUFFER_LENGTH + 1);
            }
            eventhandler = eventhandler->next;
        }
        script = script->next;
    }
    return num_sendout;
}

int oerperl_call_commandhandlers(char *command, char *args, char *target, char *nick, char *userhost, int oercommand)
{
    int retval = 0;
    int retval2 = 0;
    struct script *script;
    struct commandhandler *commandhandler;
    char stringbuffer[READ_BUFFER_LENGTH + 1];

#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_call_commandhandlers(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\")\n", (command) ? command : "(null)", (args) ? args : "(null)", (target) ? target : "(null)", (nick) ? nick : "(null)", (userhost) ? userhost : "(null)");
#endif
    script = mystate->scripts;
    while(script != NULL) {
        commandhandler = script->commandhandlers;
        while(commandhandler != NULL) {
            if(!strcasecmp(commandhandler->command, command)) {
                snprintf(stringbuffer, READ_BUFFER_LENGTH, "%s %s %s %s %s", (command) ? command : "(null)", (target) ? target : "(null)", (nick) ? nick : "(null)", (userhost) ? userhost : "(null)", (args) ? args : "(null)");
                retval = execute_perl(commandhandler->callback, stringbuffer, script, 1, " ", 4, 1, (oercommand) ? 1 : 0);
                if(retval == -1) {
#ifdef OER_DEBUG
                    oer_debug(OER_DEBUG_ERROR, "oerperl_call_commandhandlers()->execute_perl() failed for command %s in commandhandler %s in script %s\n", commandhandler->command, SvPV_nolen(commandhandler->callback), script->filename);
#endif
                } else if(oercommand && retval == OERPERL_BLOCK_OER_COMMAND) {
                    retval2 = retval;
                } else if(!oercommand) {
                    retval2 = 1;
                }
                memset(stringbuffer, 0, READ_BUFFER_LENGTH + 1);
            }
            commandhandler = commandhandler->next;
        }
        script = script->next;
    }
    return retval2;
}

static void oerperl_get_package_name(char *filename, char *buf, int buflen)
{
    int count;
    int retval = 1;
    SV *sv;
    STRLEN n_a;
    SV *notused; /* keep compiler happy about POPs */

    dSP;
#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_INFO, "oerperl_get_package_name(\"%s\")\n", filename);
#endif
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(newSVpvn(filename, strlen(filename)));
    PUTBACK;
    count = call_sv(newSVpvn(OERPERL_GET_PKG_NAME, strlen(OERPERL_GET_PKG_NAME)), G_EVAL | G_KEEPERR | G_SCALAR);
    SPAGAIN;

    sv = GvSV(gv_fetchpv("@", TRUE, SVt_PV));
    if(SvTRUE(sv)) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_ERROR, "oerperl_get_package_name()->%s\n", SvPV(sv, count));
#endif
        notused = POPs;
        retval = 0;
    } else if(count != 1) {
#ifdef OER_DEBUG
        oer_debug(OER_DEBUG_WARNING, "oerperl_get_package_name()->expected 1 return value, got %d\n", count);
#endif
    } else {
        strncpy(buf, POPpx, buflen);
    }

    PUTBACK;
    FREETMPS;
    LEAVE;
}

void oerperl_process_scripttimers(void)
{
    struct script *script;
    struct scripttimer *timer;
    time_t now;
    int retval;

#ifdef OER_DEBUG
    oer_debug(OER_DEBUG_NOISE, "oerperl_process_scripttimers()\n");
#endif
    now = time(NULL);
    script = mystate->scripts;
    while(script != NULL) {
        timer = script->timers;
        while(timer != NULL) {
            /* first run, special case */
            if(timer->lastrun == 0) {
                timer->lastrun = now;
                retval = execute_perl(timer->callback, "", script, 1, " ", 0, 0, 0);
                if(retval == -1) {
#ifdef OER_DEBUG
                    oer_debug(OER_DEBUG_ERROR, "oerperl_process_scripttimers()->execute_perl() failed for timer %s in script %s\n", SvPV_nolen(timer->callback), script->filename);
#endif
                }
            } else { /* normal case */
                if((now - timer->lastrun) >= timer->interval) {
                    timer->lastrun = now;
                    retval = execute_perl(timer->callback, "", script, 1, " ", 0, 0, 0);
                    if(retval == -1) {
#ifdef OER_DEBUG
                        oer_debug(OER_DEBUG_ERROR, "oerperl_process_scripttimers()->execute_perl() failed for timer %s in script %s\n", SvPV_nolen(timer->callback), script->filename);
#endif
                    }
                }
            }
            timer = timer->next;
        }
        script = script->next;
    }
}
