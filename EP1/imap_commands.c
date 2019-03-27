/******************************************************************************
 *                               IME-USP (2017)                               *
 *       MAC0352 - Redes de Computadores e Sistemas Distribuidos - EP1        *
 *                                                                            *
 *                            IMAP Server Commands                            *
 *                                                                            *
 *                      Pedro Pereira     - NUSP 9778794                      *
 *                      Raphael R. Gusmao - NUSP 9778561                      *
 ******************************************************************************/
// IMAP4rev1 RFC: https://tools.ietf.org/html/rfc3501#section-6

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "_aux.h"
#include "error_handler.h"
#include "imap_commands.h"

#define MAX_LENGTH 128

char user[MAX_LENGTH];
int is_loggedin = 0;
char *selected_mailbox = ".";
char *maildir = "maildir";

/******************************************************************************/
char *IMAP_capability (char *tag)
{
    return format("%s\n%s %s\n",
        "* OK [CAPABILITY IMAP4rev1]",
        tag, "OK CAPABILITY completed"
    );
}

/******************************************************************************/
char *IMAP_noop ()
{
    return "OK NOOP completed\n";
}

/******************************************************************************/
char *IMAP_logout (char *tag)
{
    return format("%s\n%s %s\n",
        "* BYE IMAP4rev1 Server logging out",
        tag, "OK LOGOUT completed"
    );
}

/******************************************************************************/
char *IMAP_login (char *username, char *password)
{
    add_to_stack("imap_commands -> IMAP_login()");
    username = split(username, "\"").data[0];
    password = split(password, "\"").data[0];
    int ok = 0;
    char *line;
    size_t len = 0;
    ssize_t line_size;
    FILE *file = fopen("./users.txt", "r");
    if (file == NULL) die_with_msg("IMAP_login(): Cannot open the file");
    while ((line_size = getline(&line, &len, file)) > 0) {
        string_vector split_line = split(line, " \n\r\t");
        if (!strcmp(username, split_line.data[0])) {
            if ((ok = !strcmp(password, split_line.data[1]))) {
                strncpy(user, username, MAX_LENGTH);
                is_loggedin = 1;
            }
            break;
        }
        free_vector(split_line);
    }
    fclose(file);
    if (line) free(line);
    pop_stack();
    if (ok) return "OK LOGIN completed\n";
    return         "NO Authentication failed\n";
}

/******************************************************************************/
char *IMAP_select (char *tag)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_select()");
    char *ret = "";
    int exists = 0, first_unseen = 0;
    char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
    DIR *dr = opendir(path);
    if (dr == NULL) return 0;
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {
        if (de->d_type != 4) {
            exists++;
            if (first_unseen == 0) {
                string_vector split_cur_name = split(de->d_name, ".,:");
                char *ch_flags = split_cur_name.data[split_cur_name.size-1];
                int is_seen = 0;
                for (int i = 0; i < strlen(ch_flags); i++) {
                    if (ch_flags[i] == 'S') {
                        is_seen = 1;
                        break;
                    }
                }
                if (!is_seen) first_unseen = exists;
            }
        }
    }
    char *line;
    size_t len = 0;
    ssize_t line_size;
    char *filename = format("./%s/%s/dovecot-uidlist", maildir, user);
    FILE *file = fopen(filename, "r");
    FILE *new_file = fopen("temp", "w");
    if (file == NULL || new_file == NULL) {
        pop_stack();
        return format("%s NO select failure\n", tag);
    }
    getline(&line, &len, file);
    string_vector split_seq = split(line, " ");
    char *uid_validity = substr(split_seq.data[1], 1, strlen(split_seq.data[1])-1);
    char *uid_next = substr(split_seq.data[2], 1, strlen(split_seq.data[2])-1);
    ret = format("%s* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft Junk NonJunk)\n", ret);
    ret = format("%s* OK [PERMANENTFLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft Junk NonJunk)] Flags permitted\n", ret);
    ret = format("%s* %d EXISTS\n", ret, exists);
    ret = format("%s* 0 RECENT\n", ret);
    ret = format("%s* OK [UNSEEN %d] Message %d is first unseen\n", ret, first_unseen, first_unseen);
    ret = format("%s* OK [UIDVALIDITY %s] UIDs valid\n", ret, uid_validity);
    ret = format("%s* OK [UIDNEXT %s] Predicted next UID\n", ret, uid_next);
    ret = format("%s%s OK [READ-WRITE] Select completed\n", ret, tag);
    fclose(file);
    if (line) free(line);
    pop_stack();
    return ret;
}

/******************************************************************************/
char *IMAP_list (char *tag)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    return format("* LIST (\\HasNoChildren) \".\" INBOX\n%s OK LIST completed\n", tag);
}

/******************************************************************************/
char *IMAP_lsub ()
{
    if (!is_loggedin) return "BAD you must be logged in\n";
    return "OK LSUB completed\n";
}

/******************************************************************************/
char *IMAP_close ()
{
    IMAP_expunge("");
    return "OK CLOSE completed\n";
}

/******************************************************************************/
char *IMAP_expunge (char *tag)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_expunge()");
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char **msgs_to_remove = emalloc(MAX_LENGTH * sizeof(char *)); int k = 0;
    char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
    DIR *dr = opendir(path);
    if (dr != NULL) {
        struct dirent *de;
        while ((de = readdir(dr)) != NULL) {
            if (de->d_type != 4) {
                string_vector split_cur_name = split(de->d_name, ".,:");
                char *ch_flags = split_cur_name.data[split_cur_name.size-1];
                for (int i = 0; i < strlen(ch_flags); i++) {
                    if (ch_flags[i] == 'T') {
                        if (!remove(format("%s/%s", path, de->d_name))) {
                            msgs_to_remove[k++] = split(de->d_name, ":.").data[0];
                            break;
                        } else {
                            pop_stack();
                            return format("%s NO expunge failure\n", tag);
                        }
                    }
                }
            }
        }
        closedir(dr);
    }
    char *line;
    size_t len = 0;
    ssize_t line_size;
    char *filename = format("./%s/%s/dovecot-uidlist", maildir, user);
    FILE *file = fopen(filename, "r");
    FILE *new_file = fopen("temp", "w");
    if (file == NULL || new_file == NULL) {
        pop_stack();
        return format("%s NO expunge failure\n", tag);
    }
    getline(&line, &len, file);
    fprintf(new_file, "%s", line);
    int msgs = 0, j = 0;
    while ((line_size = getline(&line, &len, file)) > 0 && msgs < k) {
        int flag = 1;
        j++;
        string_vector split_line = split(line, " \n\r\t:.");
        char *msg_name = split_line.data[split_line.size-3];
        for (int i = 0; i < k; i++) {
            if (!strcmp(msgs_to_remove[i], msg_name)) {
                ret = format("%s* %d EXPUNGE\n", ret, j);
                flag = 0;
                msgs++;
                break;
            }
        }
        if (flag) fprintf(new_file, "%s", line);
    }
    if (line_size > 0) {
        do {
            fprintf(new_file, "%s", line);
        } while ((line_size = getline(&line, &len, file)) > 0);
    }
    fclose(file);
    fclose(new_file);
    remove(filename);
    rename("temp", filename);
    if (line) free(line);
    pop_stack();
    return format ("%s%s OK EXPUNGE completed\n", ret, tag);
}

/******************************************************************************/
char *IMAP_fetch (char *tag, char *seq, char *macro)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_fetch()");
    macro = upper_case(macro);
    string_vector split_macro;
    if (macro[0] == '(') {
        split_macro = split(substr(macro, 1, strlen(macro) - 2), " ");
    } else {
        split_macro = split(macro, " ");
    }
    int ok = 1;
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char *line;
    size_t len = 0;
    ssize_t line_size;
    FILE *file = fopen(format("./%s/%s/dovecot-uidlist", maildir, user), "r");
    if (file == NULL) {
        pop_stack();
        return format("%s NO fetch failure\n", tag);
    }
    string_vector split_seq = split(seq, ":");
    int msg_num = atoi(split_seq.data[0]), msg_num_end;
    if (split_seq.size == 1) {
        msg_num_end = msg_num;
    } else {
        if (strcmp(split_seq.data[1], "*")) msg_num_end = 0;
        else                                msg_num_end = atoi(split_seq.data[1]);
    }
    int i;
    for (i = 0; i < msg_num; i++) getline(&line, &len, file);
    for (; i <= msg_num_end || !msg_num_end; i++) {
        if ((line_size = getline(&line, &len, file)) <= 0) break;
        string_vector split_line = split(line, " \n\r\t");
        char *msg_name = split_line.data[split_line.size-1];
        ret = format("%s* %d FETCH (UID %s", ret, i, split_line.data[0]);
        for (int j = 0; j < split_macro.size; j++) {
            char *cur_macro = split(split_macro.data[j], ".").data[0];
            if (!strcmp(cur_macro,        "RFC822")) {// RFC822.SIZE
                char *rfc822_size = substr(split_line.data[1], 1, strlen(split_line.data[1])-1);
                ret = format("%s RFC822.SIZE %s", ret, rfc822_size);
            } else if (!strcmp(cur_macro, "FLAGS")) {// FLAGS
                char *flags = _get_flags(msg_name);
                ret = format("%s FLAGS (%s)", ret, flags);
            } else if (!strcmp(cur_macro, "BODY")) {// BODY / BODY.PEEK
                char *msg_line;
                size_t msg_len = 0;
                ssize_t msg_line_size;
                char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
                char *msg_file_name = _get_file_name(msg_name);
                FILE *msg_file = fopen(format("%s/%s", path, msg_file_name), "r");
                if (msg_file == NULL) {
                    pop_stack();
                    return format("%s NO uid fetch failure\n", tag);
                }
                int macro_len = strlen(split_macro.data[j]);
                char *fields = "";
                int body_size = 0;
                char *body = "";
                if (macro_len <= 11) {// BODY.PEEK[]
                    while ((msg_line_size = getline(&msg_line, &msg_len, msg_file)) > 0) {
                        body = format("%s%s", body, msg_line);
                        body_size += msg_line_size;
                    }
                } else {// BODY.PEEK[HEADER.FIELDS (...)]
                    string_vector split_fields;
                    for (int k = 0; k < macro_len; k++) {
                        if (split_macro.data[j][k] == '(') {
                            fields = substr(split_macro.data[j], k+1, macro_len-k-3);
                            split_fields = split(fields, " \n\r\t");
                            break;
                        }
                    }
                    int flag = 0;
                    while ((msg_line_size = getline(&msg_line, &msg_len, msg_file)) > 0) {
                        if (msg_line[0] == '\n' || msg_line[0] == '\r') break;
                        if (msg_line[0] == ' '  || msg_line[0] == '\t') {
                            if (flag) {
                                body = format("%s%s", body, msg_line);
                                body_size += msg_line_size;
                                flag = 1;
                            }
                        } else {
                            flag = 0;
                            string_vector split_msg_line = split(msg_line, ":");
                            for (int k = 0; k < split_fields.size; k++) {
                                if (!strcmp(upper_case(split_msg_line.data[0]), split_fields.data[k])) {
                                    body = format("%s%s", body, msg_line);
                                    body_size += msg_line_size;
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                    }
                    fields = format("HEADER.FIELDS (%s)", fields);
                    free_vector(split_fields);
                }
                ret = format("%s BODY[%s] {%d}\n%s", ret, fields, body_size, body);
            }
        }
        ret = format("%s)\n", ret);
        free_vector(split_line);
    }
    fclose(file);
    if (line) free(line);
    pop_stack();
    if (ok) return format("%s%s OK UID FETCH completed\n", ret, tag);
    return format("%s %s", tag, ret);
}

/******************************************************************************/
char *IMAP_store (char *tag, char *seq, char *macro, char *flag_list)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_store()");
    macro = upper_case(macro);
    string_vector split_macro = split(macro, ".");
    int is_silent = 0;
    if (!strcmp(split_macro.data[split_macro.size-1], "SILENT")) is_silent = 1;
    macro = split_macro.data[0];
    int ok = 1;
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char *line;
    size_t len = 0;
    ssize_t line_size;
    FILE *file = fopen(format("./%s/%s/dovecot-uidlist", maildir, user), "r");
    if (file == NULL) {
        pop_stack();
        return format("%s NO store failure\n", tag);
    }
    string_vector split_seq = split(seq, ":");
    int msg_num = atoi(split_seq.data[0]), msg_num_end;
    if (split_seq.size == 1) {
        msg_num_end = msg_num;
    } else {
        if (strcmp(split_seq.data[1], "*")) msg_num_end = 0;
        else                                msg_num_end = atoi(split_seq.data[1]);
    }
    int i;
    for (i = 0; i < msg_num; i++) getline(&line, &len, file);
    for (; i <= msg_num_end || !msg_num_end; i++) {
        if ((line_size = getline(&line, &len, file)) <= 0) break;
        string_vector split_line = split(line, " \n\r\t");
        char *msg_name = split_line.data[3];
        if (!strcmp(macro, "FLAGS")) {// Replace flags
            if (!_edit_flags(REPLACE, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot replace the flags from message %d\n", i);
                break;
            }
        } else if (!strcmp(macro, "+FLAGS")) {// Add flags
            if (!_edit_flags(ADD, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot add the flags to message %d\n", i);
                break;
            }
        } else if (!strcmp(macro, "-FLAGS")) {// Remove flags
            if (!_edit_flags(REMOVE, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot remove the flags from message %d\n", i);
                break;
            }
        } else {
            free_vector(split_line);
            ok = 0;
            ret = "NO invalid command\n";
            break;
        }
        if (!is_silent) {
            char *flags = _get_flags(msg_name);
            if (flags != NULL) {
                ret = format("%s* %d FETCH (FLAGS (%s))\n", ret, i, flags);
            } else {
                free_vector(split_line);
                ok = 0;
                ret = "NO can't fetch that data\n";
                break;
            }
        }
        free_vector(split_line);
    }
    fclose(file);
    if (line) free(line);
    pop_stack();
    if (ok) return format("%s%s OK STORE completed\n", ret, tag);
    return format("%s %s", tag, ret);
}

/******************************************************************************/
char *IMAP_uid_fetch (char *tag, char *seq, char *macro)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_uid_fetch()");
    macro = upper_case(macro);
    string_vector split_macro;
    if (macro[0] == '(') {
        split_macro = split(substr(macro, 1, strlen(macro) - 2), " ");
    } else {
        split_macro = split(macro, " ");
    }
    int ok = 1;
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char *line;
    size_t len = 0;
    ssize_t line_size;
    FILE *file = fopen(format("./%s/%s/dovecot-uidlist", maildir, user), "r");
    if (file == NULL) {
        pop_stack();
        return format("%s NO uid fetch failure\n", tag);
    }
    string_vector split_seq = split(seq, ":");
    int uid = atoi(split_seq.data[0]), uid_end;
    if (split_seq.size == 1) {
        uid_end = uid;
    } else {
        if (strcmp(split_seq.data[1], "*")) uid_end = 0;
        else                                uid_end = atoi(split_seq.data[1]);
    }
    int i = 1;
    getline(&line, &len, file);
    while ((line_size = getline(&line, &len, file)) > 0) {
        if (atoi(split(line, " \n\r\t").data[0]) >= uid) break;
        i++;
    }
    do {
        string_vector split_line = split(line, " \n\r\t");
        uid = atoi(split_line.data[0]);
        if (uid_end > 0 && uid > uid_end) {
            free_vector(split_line);
            break;
        }
        char *msg_name = split_line.data[split_line.size-1];
        ret = format("%s* %d FETCH (UID %d", ret, i, uid);
        for (int j = 0; j < split_macro.size; j++) {
            char *cur_macro = split(split_macro.data[j], ".").data[0];
            if (!strcmp(cur_macro,        "RFC822")) {// RFC822.SIZE
                char *rfc822_size = substr(split_line.data[1], 1, strlen(split_line.data[1])-1);
                ret = format("%s RFC822.SIZE %s", ret, rfc822_size);
            } else if (!strcmp(cur_macro, "FLAGS")) {// FLAGS
                char *flags = _get_flags(msg_name);
                ret = format("%s FLAGS (%s)", ret, flags);
            } else if (!strcmp(cur_macro, "BODY")) {// BODY / BODY.PEEK
                char *msg_line;
                size_t msg_len = 0;
                ssize_t msg_line_size;
                char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
                char *msg_file_name = _get_file_name(msg_name);
                FILE *msg_file = fopen(format("%s/%s", path, msg_file_name), "r");
                if (msg_file == NULL) {
                    pop_stack();
                    return format("%s NO uid fetch failure\n", tag);
                }
                int macro_len = strlen(split_macro.data[j]);
                char *fields = "";
                int body_size = 0;
                char *body = "";
                if (macro_len <= 11) {// BODY.PEEK[]
                    while ((msg_line_size = getline(&msg_line, &msg_len, msg_file)) > 0) {
                        body = format("%s%s", body, msg_line);
                        body_size += msg_line_size;
                    }
                } else {// BODY.PEEK[HEADER.FIELDS (...)]
                    string_vector split_fields;
                    for (int k = 0; k < macro_len; k++) {
                        if (split_macro.data[j][k] == '(') {
                            fields = substr(split_macro.data[j], k+1, macro_len-k-3);
                            split_fields = split(fields, " \n\r\t");
                            break;
                        }
                    }
                    int flag = 0;
                    while ((msg_line_size = getline(&msg_line, &msg_len, msg_file)) > 0) {
                        if (msg_line[0] == '\n' || msg_line[0] == '\r') break;
                        if (msg_line[0] == ' '  || msg_line[0] == '\t') {
                            if (flag) {
                                body = format("%s%s", body, msg_line);
                                body_size += msg_line_size;
                                flag = 1;
                            }
                        } else {
                            flag = 0;
                            string_vector split_msg_line = split(msg_line, ":");
                            for (int k = 0; k < split_fields.size; k++) {
                                if (!strcmp(upper_case(split_msg_line.data[0]), split_fields.data[k])) {
                                    body = format("%s%s", body, msg_line);
                                    body_size += msg_line_size;
                                    flag = 1;
                                    break;
                                }
                            }
                        }
                    }
                    fields = format("HEADER.FIELDS (%s)", fields);
                    free_vector(split_fields);
                }
                ret = format("%s BODY[%s] {%d}\n%s", ret, fields, body_size, body);
            }
        }
        ret = format("%s)\n", ret);
        free_vector(split_line);
        i++;
    } while ((line_size = getline(&line, &len, file)) > 0);
    fclose(file);
    if (line) free(line);
    pop_stack();
    if (ok) return format("%s%s OK UID FETCH completed\n", ret, tag);
    return format("%s %s", tag, ret);
}

/******************************************************************************/
char *IMAP_uid_store (char *tag, char *seq, char *macro, char *flag_list)
{
    if (!is_loggedin) return format("%s BAD you must be logged in\n", tag);
    add_to_stack("imap_commands -> IMAP_uid_store()");
    macro = upper_case(macro);
    string_vector split_macro = split(macro, ".");
    int is_silent = 0;
    if (!strcmp(split_macro.data[split_macro.size-1], "SILENT")) is_silent = 1;
    macro = split_macro.data[0];
    int ok = 1;
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char *line;
    size_t len = 0;
    ssize_t line_size;
    FILE *file = fopen(format("./%s/%s/dovecot-uidlist", maildir, user), "r");
    if (file == NULL) {
        pop_stack();
        return format("%s NO uid store failure\n", tag);
    }
    string_vector split_seq = split(seq, ":");
    int uid = atoi(split_seq.data[0]), uid_end;
    if (split_seq.size == 1) {
        uid_end = uid;
    } else {
        if (strcmp(split_seq.data[1], "*")) uid_end = 0;
        else                                uid_end = atoi(split_seq.data[1]);
    }
    int i = 1;
    getline(&line, &len, file);
    while ((line_size = getline(&line, &len, file)) > 0) {
        if (atoi(split(line, " \n\r\t").data[0]) >= uid) break;
        i++;
    }
    do {
        string_vector split_line = split(line, " \n\r\t");
        uid = atoi(split_line.data[0]);
        if (uid_end > 0 && uid > uid_end) {
            free_vector(split_line);
            break;
        }
        char *msg_name = split_line.data[split_line.size-1];
        if (!strcmp(macro, "FLAGS")) {// Replace flags
            if (!_edit_flags(REPLACE, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot replace the flags from message %d\n", uid);
                break;
            }
        } else if (!strcmp(macro, "+FLAGS")) {// Add flags
            if (!_edit_flags(ADD, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot add the flags to message %d\n", uid);
                break;
            }
        } else if (!strcmp(macro, "-FLAGS")) {// Remove flags
            if (!_edit_flags(REMOVE, msg_name, flag_list)) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot remove the flags from message %d\n", uid);
                break;
            }
        } else {
            free_vector(split_line);
            ok = 0;
            ret = "NO invalid command\n";
            break;
        }
        if (!is_silent) {
            char *flags = _get_flags(msg_name);
            if (flags != NULL) {
                ret = format("%s* %d FETCH (UID %d FLAGS (%s))\n", ret, i, uid, flags);
            } else {
                free_vector(split_line);
                ok = 0;
                ret = "NO can't fetch that data\n";
                break;
            }
        }
        free_vector(split_line);
        i++;
    } while ((line_size = getline(&line, &len, file)) > 0);
    fclose(file);
    if (line) free(line);
    pop_stack();
    if (ok) return format("%s%s OK UID STORE completed\n", ret, tag);
    return format("%s %s", tag, ret);
}

/******************************************************************************/
char *IMAP_uid_move (char *seq, char *to)
{
    if (!is_loggedin) return "BAD you must be logged in\n";
    add_to_stack("imap_commands -> IMAP_uid_move()");
    to = upper_case(to);
    int ok = 1;
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = "";
    char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
    char *line;
    size_t len = 0;
    ssize_t line_size;
    char *filename = format("./%s/%s/dovecot-uidlist", maildir, user);
    FILE *file = fopen(filename, "r");
    FILE *new_file = fopen("temp", "w");
    if (file == NULL || new_file == NULL) {
        pop_stack();
        return "NO cannot remove the messages";
    }
    string_vector split_seq = split(seq, ":");
    int uid = atoi(split_seq.data[0]), uid_end;
    if (split_seq.size == 1) {
        uid_end = uid;
    } else {
        if (strcmp(split_seq.data[1], "*")) uid_end = 0;
        else                                uid_end = atoi(split_seq.data[1]);
    }
    int i = 0;
    getline(&line, &len, file);
    fprintf(new_file, "%s", line);
    while ((line_size = getline(&line, &len, file)) > 0) {
        if (atoi(split(line, " \n\r\t").data[0]) >= uid) break;
        i++;
        fprintf(new_file, "%s", line);
    }
    do {
        string_vector split_line = split(line, " \n\r\t");
        uid = atoi(split_line.data[0]);
        if (uid_end > 0 && uid > uid_end) {
            free_vector(split_line);
            break;
        }
        char *msg_name = split_line.data[split_line.size-1];
        if (!strcmp(to, "\"TRASH\"") || !strcmp(to, "TRASH")) {
            if (remove(format("%s/%s", path, _get_file_name(msg_name))) != 0) {
                free_vector(split_line);
                ok = 0;
                ret = format("NO cannot remove the message %d\n", uid);
                break;
            }
        } else {
            free_vector(split_line);
            ok = 0;
            ret = "NO invalid command\n";
            break;
        }
        free_vector(split_line);
        i++;
    } while ((line_size = getline(&line, &len, file)) > 0);
    if (line_size > 0) {
        do {
            fprintf(new_file, "%s", line);
        } while ((line_size = getline(&line, &len, file)) > 0);
    }
    fclose(file);
    fclose(new_file);
    remove(filename);
    rename("temp", filename);
    if (line) free(line);
    pop_stack();
    if (ok) return "OK UID MOVE completed\n";
    return ret;
}

/******************************************************************************/
char *_get_file_name (char *msg_name)
{
    add_to_stack("imap_commands -> _get_file_name()");
    char *file_name;
    msg_name = split(msg_name, ":.").data[0];
    char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
    DIR *dr = opendir(path);
    if (dr == NULL) return 0;
    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {
        if (de->d_type != 4) {
            string_vector split_cur_name = split(de->d_name, ".,:");
            if (!strcmp(split_cur_name.data[0], msg_name)) {// Message found
                file_name = de->d_name;
                break;
            }
        }
    }
    pop_stack();
    return file_name;
}

/******************************************************************************/
char *_get_flags (char *msg_name)
{
    add_to_stack("imap_commands -> _get_flags()");
    char *ret = emalloc(MAX_LENGTH * sizeof(char)); ret = NULL;
    string_vector split_cur_name = split(_get_file_name(msg_name), ".,:");
    char *ch_flags = split_cur_name.data[split_cur_name.size-1];
    if (ch_flags != NULL) {
        for (int i = 0; i < strlen(ch_flags); i++) {
            char *flag = 0;
            if      (ch_flags[i] == 'S') flag = "\\Seen";
            else if (ch_flags[i] == 'R') flag = "\\Answered";
            else if (ch_flags[i] == 'F') flag = "\\Flagged";
            else if (ch_flags[i] == 'T') flag = "\\Deleted";
            else if (ch_flags[i] == 'D') flag = "\\Draft";
            else if (ch_flags[i] == 'b') flag = "NonJunk";
            else if (ch_flags[i] == 'a') flag = "Junk";
            if (i == 0) ret = flag;
            else        ret = format("%s %s", ret, flag);
        }
    }
    pop_stack();
    return ret;
}

/******************************************************************************/
int _edit_flags (int action, char *msg_name, char *flag_list)
{
    add_to_stack("imap_commands -> _edit_flags()");
    string_vector split_flag_list;
    if (flag_list[0] == '(') {
        split_flag_list = split(substr(flag_list, 1, strlen(flag_list) - 2), " ");
    } else {
        split_flag_list = split(flag_list, " ");
    }
    char *ch_flags = "";
    for (int i = 0; i < split_flag_list.size; i++) {
        char flag = 0;
        if      (!strcmp(split_flag_list.data[i], "\\Seen"))     flag = 'S';
        else if (!strcmp(split_flag_list.data[i], "\\Answered")) flag = 'R';
        else if (!strcmp(split_flag_list.data[i], "\\Flagged"))  flag = 'F';
        else if (!strcmp(split_flag_list.data[i], "\\Deleted"))  flag = 'T';
        else if (!strcmp(split_flag_list.data[i], "\\Draft"))    flag = 'D';
        else if (!strcmp(split_flag_list.data[i], "NonJunk"))    flag = 'b';
        else if (!strcmp(split_flag_list.data[i], "Junk"))       flag = 'a';
        ch_flags = format("%s%c", ch_flags, flag);
    }
    char *file_name = _get_file_name(msg_name);
    string_vector split_name = split(file_name, ".,:");
    char *old_flags = split_name.data[split_name.size-1];
    char *new_flags = "";
    if (action == REPLACE) {
        for (int i = 0; i < strlen(ch_flags); i++) {
            new_flags = format("%s%c", new_flags, ch_flags[i]);
        }
    } else if (action == ADD) {
        int is_junk = 0;
        for (int i = 0; i < strlen(old_flags); i++) {
            if (old_flags[i] == 'a')
                is_junk = 1;
            else if (old_flags[i] == 'b')
                is_junk = 0;
            else
                new_flags = format("%s%c", new_flags, old_flags[i]);
        }
        for (int i = 0; i < strlen(ch_flags); i++) {
            if (ch_flags[i] == 'a') {
                is_junk = 1;
            } else if (ch_flags[i] == 'b') {
                is_junk = 0;
            } else {
                int flag = 1;
                for (int j = 0; j < strlen(old_flags); j++) {
                    if (ch_flags[i] == old_flags[j]) {
                        flag = 0;
                        break;
                    }
                }
                if (flag) new_flags = format("%s%c", new_flags, ch_flags[i]);
            }
        }
        if (is_junk) new_flags = format("%sa", new_flags);
        else         new_flags = format("%sb", new_flags);
    } else if (action == REMOVE) {
        int is_junk = 0;
        for (int i = 0; i < strlen(old_flags); i++) {
            if (old_flags[i] == 'a')      is_junk = 1;
            else if (old_flags[i] == 'b') is_junk = 0;
            int flag = 1;
            for (int j = 0; j < strlen(ch_flags); j++) {
                if (old_flags[i] == ch_flags[j]) {
                    if (ch_flags[j] == 'a')      is_junk = 0;
                    else if (ch_flags[j] == 'b') is_junk = 1;
                    flag = 0;
                    break;
                }
            }
            if (flag) new_flags = format("%s%c", new_flags, old_flags[i]);
        }
        if (is_junk) new_flags = format("%sa", new_flags);
        else         new_flags = format("%sb", new_flags);
    } else return 0;
    char *new_name = substr(file_name, 0, strlen(file_name) - strlen(old_flags));
    char *path = format("./%s/%s/%s/cur", maildir, user, selected_mailbox);
    new_name = format("%s/%s%s", path, new_name, new_flags);
    rename(format("%s/%s", path, file_name), new_name);
    pop_stack();
    return 1;
}

/******************************************************************************/
