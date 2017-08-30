/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                            IMAP Client Commands                            *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/

// https://tools.ietf.org/html/rfc3501#section-6

#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "error_handler.h"
#include "imap_commands.h"

char *username = "Goku";

char *IMAP_noop ()
{
    return "OK NOOP completed\n";
}

char *IMAP_logout ()                                                     // TODO
{
    return "BYE\nOK LOGOUT completed\n";
}

char *IMAP_starttls ()                                                   // TODO
{
    return "OK Begin TLS negotiation now\n";
}

char *IMAP_authenticate (char *machanism)                                // TODO
{
    return "OK authentication successful\n";
}

char *IMAP_login (char *username, char *password)                        // TODO
{
    return "OK LOGIN completed\n";
}

char *IMAP_create (char *mailbox)
{
    add_to_stack("IMAP -> create()");
    char path[128];
    strcpy(path, "mail/");
    strcat(path, username);
    strcat(path, "/");
    strcat(path, mailbox);
    struct stat st = {0};
	if (stat(path, &st) == -1) {
		if (mkdir(path, 0777) == 0) {
            char cur[128]; strcpy(cur, path); strcat(cur, "/cur");
            char new[128]; strcpy(new, path); strcat(new, "/new");
            char tmp[128]; strcpy(tmp, path); strcat(tmp, "/tmp");
            if (mkdir(cur, 0777) == 0
             && mkdir(new, 0777) == 0
             && mkdir(tmp, 0777) == 0) {
                pop_stack();
                return "OK CREATE completed\n";
            }
		}
	}
    pop_stack();
    return "NO create failure: cant create mailbox with that name\n";
}

char *IMAP_delete (char *mailbox)                                        // TODO
{
    return "OK DELETE completed\n";
}

char *IMAP_rename (char *existing_mailbox, char *new_mailbox)            // TODO
{
    return "OK RENAME completed\n";
}

char *IMAP_list (char *reference, char *mailbox)                         // TODO
{
    return "OK LIST completed\n";
}

char *IMAP_expunge ()                                                    // TODO
{
    return "OK EXPUNGE completed\n";
}
