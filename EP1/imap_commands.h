/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                            IMAP Server Commands                            *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#ifndef __IMAP_COMMANDS_H__
#define __IMAP_COMMANDS_H__

#define REPLACE 0
#define ADD 1
#define REMOVE 2

/******************************************************************************
 *                         Client Commands - Any State                        *
 ******************************************************************************/
// CAPABILITY Command
char *IMAP_capability (char *tag);

// NOOP Command
char *IMAP_noop ();

// LOGOUT Command
char *IMAP_logout (char *tag);

/******************************************************************************
 *                 Client Commands - Not Authenticated State                  *
 ******************************************************************************/
// LOGIN Command
char *IMAP_login (char *username, char *password);

/******************************************************************************
 *                   Client Commands - Authenticated State                    *
 ******************************************************************************/
// SELECT command
char *IMAP_select (char *tag);

// LIST Command
char *IMAP_list (char *tag);

// LSUB Command
char *IMAP_lsub ();

/******************************************************************************
 *                        Client Commands - Selected State                    *
 ******************************************************************************/
// CLOSE Command
char *IMAP_close ();

// EXPUNGE Command
char *IMAP_expunge (char *tag);

// FETCH Command
char *IMAP_fetch (char *tag, char *seq, char *macro);

// STORE Command
char *IMAP_store (char *tag, char *seq, char *macro, char *flag_list);

// UID FETCH Command
char *IMAP_uid_fetch (char *tag, char *seq, char *macro);

// UID STORE Command
char *IMAP_uid_store (char *tag, char *seq, char *macro, char *flag_list);

// UID MOVE Command
char *IMAP_uid_move (char *seq, char *to);

/******************************************************************************
 *                         Auxiliar Server Functions                          *
 ******************************************************************************/
// Gets the name of the message file
char *_get_file_name (char *msg_name);

// Gets the flags from a message
char *_get_flags (char *msg_name);

// action 0 : Replace the flags for the message with the argument
//        1 : Add the argument to the flags for the message
//        2 : Remove the argument from the flags for the message
int _edit_flags (int action, char *msg_name, char *flag_list);

#endif

/******************************************************************************/
