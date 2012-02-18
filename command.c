#include <string.h>
#include <ncurses.h>
#include "command.h"
#include "jabber.h"
#include "windows.h"
#include "util.h"

static int _cmd_quit(void);
static int _cmd_help(void);
static int _cmd_who(void);
static int _cmd_msg(char *cmd);
static int _cmd_close(char *cmd);
static int _cmd_default(char *cmd);
static int _valid_start_command(char *cmd);

int handle_start_command(char *inp)
{
    int result;
    
    // trim input and take a copy
    char inp_cpy[100];
    inp = trim(inp);
    strcpy(inp_cpy, inp);
    
    // get the command "/command"
    char *command = strtok(inp_cpy, " ");

    // handle invalid commands
    if (!_valid_start_command(command)) {
        cons_bad_command(command);
        gui_refresh();
        result = AWAIT_COMMAND;

    // quit
    } else if (strcmp(command, "/quit") == 0) {
        result = QUIT_PROF;

    // help
    } else if (strcmp(command, "/help") == 0) {
        cons_help();
        gui_refresh();
        result = AWAIT_COMMAND;

    // connect
    } else if (strcmp(command, "/connect") == 0) {
        if (strlen(inp) < 10) {
            cons_bad_connect();
            gui_refresh();
            result = AWAIT_COMMAND;
        } else {
            char *user;
            user = strndup(inp+9, strlen(inp)-9);

            status_bar_get_password();
            status_bar_refresh();
            char passwd[20];
            inp_get_password(passwd);
            int connect_status = jabber_connect(user, passwd);
            if (connect_status == CONNECTING)
                result = START_MAIN;
            else
                result = AWAIT_COMMAND;
            }
    } else {
        cons_bad_command(inp);
        gui_refresh();
        result = AWAIT_COMMAND;
    }

    inp_clear();

    return result;
}

int handle_command(char *cmd)
{
    int result = FALSE;
    if (strcmp(cmd, "/quit") == 0) {
        result = _cmd_quit();
    } else if (strncmp(cmd, "/help", 5) == 0) {
        result = _cmd_help();
    } else if (strncmp(cmd, "/who", 4) == 0) {
        result = _cmd_who();
    } else if (strncmp(cmd, "/msg ", 5) == 0) {
        result = _cmd_msg(cmd);
    } else if (strncmp(cmd, "/close", 6) == 0) {
        result = _cmd_close(cmd);
    } else {
        result = _cmd_default(cmd);
    }

    inp_clear();

    return result;

}

static int _cmd_quit(void)
{
    return FALSE;
}

static int _cmd_help(void)
{
    cons_help();

    return TRUE;
}

static int _cmd_who(void)
{
    jabber_roster_request();

    return TRUE;
}

static int _cmd_msg(char *cmd)
{
    char *usr_msg = NULL;
    char *usr = NULL;
    char *msg = NULL;

    usr_msg = strndup(cmd+5, strlen(cmd)-5);
    usr = strtok(usr_msg, " ");
    if (usr != NULL) {
        msg = strndup(cmd+5+strlen(usr)+1, strlen(cmd)-(5+strlen(usr)+1));
        if (msg != NULL) {
            jabber_send(msg, usr);
            win_show_outgoing_msg("me", usr, msg);
        }
    }

    return TRUE;
}

static int _cmd_close(char *cmd)
{
    if (win_in_chat()) {
        win_close_win();
    } else {
        cons_bad_command(cmd);
    }
    
    return TRUE;
}

static int _cmd_default(char *cmd)
{
    if (win_in_chat()) {
        char recipient[100] = "";
        win_get_recipient(recipient);
        jabber_send(cmd, recipient);
        win_show_outgoing_msg("me", recipient, cmd);
    } else {
        cons_bad_command(cmd);
    }

    return TRUE;
}

static int _valid_start_command(char *cmd)
{
    return ((strcmp(cmd, "/quit") == 0) || 
            (strcmp(cmd, "/help") == 0) ||
            (strcmp(cmd, "/connect") == 0));
}
