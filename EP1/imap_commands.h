/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                                TCP Handler                                 *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

#ifndef __IMAP_COMMANDS_H__
#define __IMAP_COMMANDS_H__

/******************************************************************************
 *                         Client Commands - Any State                        *
 ******************************************************************************/
// NOOP Command
char *IMAP_noop ();

// LOGOUT Command
char *IMAP_logout ();

/******************************************************************************
 *                 Client Commands - Not Authenticated State                  *
 ******************************************************************************/
// STARTTLS Command
char *IMAP_starttls ();

// AUTHENTICATE Command
char *IMAP_authenticate (char *machanism);

// LOGIN Command
char *IMAP_login (char *username, char *password);

/******************************************************************************
 *                   Client Commands - Authenticated State                    *
 ******************************************************************************/
// CREATE Command
char *IMAP_create (char *mailbox);

// DELETE Command
char *IMAP_delete (char *mailbox);

// RENAME Command
char *IMAP_rename (char *existing_mailbox, char *new_mailbox);

// LIST Command
char *IMAP_list (char *reference, char *mailbox);

/******************************************************************************
 *                        Client Commands - Selected State                    *
 ******************************************************************************/

// EXPUNGE Command
char *IMAP_expunge ();

#endif

/******************************************************************************/
