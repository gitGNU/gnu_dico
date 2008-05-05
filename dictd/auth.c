/* This file is part of Dico.
   Copyright (C) 1998-2000, 2008 Sergey Poznyakoff

   Dico is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Dico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Dico.  If not, see <http://www.gnu.org/licenses/>. */

#include <dictd.h>
#include <md5.h>

static int
verify_apop(const char *password, const char *user_digest)
{
    int i;
    struct md5_ctx md5context;
    unsigned char md5digest[16];
    char buf[sizeof(md5digest) * 2 + 1];
    char *p;

    md5_init_ctx(&md5context);
    md5_process_bytes(msg_id, strlen(msg_id), &md5context);
    md5_process_bytes(password, strlen(password), &md5context);
    md5_finish_ctx(&md5context, md5digest);

    for (i = 0, p = buf; i < 16; i++, p += 2)
	sprintf(p, "%02x", md5digest[i]);
    return strcmp(user_digest, buf);
}

static int
auth(const char *username, const char *authstr)
{
    int rc = 1;
    char *password;
    
    if (udb_open(user_db)) {
	logmsg(L_ERR, 0, _("failed to open user database"));
	return 1;
    }
    if (udb_get_password(user_db, username, &password)) {
	logmsg(L_ERR, 0,
	       _("failed to get password for `%s' from the database"),
	       username);
    } else {
	if (!password) {
	    logmsg(L_ERR, 0, _("no such user `%s'"), username);
	    rc = 1;
	} else {
	    rc = verify_apop(password, authstr);
	    if (rc) 
		logmsg(L_ERR, 0,
		       _("authentication failed for `%s'"), username);
	    free(password);
	}
    }
    udb_close(user_db);
    return rc;
}

void
dictd_auth(stream_t str, int argc, char **argv)
{
    /* FIXME: Raise some global variable and obtain user's groups */
    if (auth(argv[1], argv[2]) == 0) 
	stream_writez(str, "230 Authentication successful");
    else
	stream_writez(str,
		      "531 Access denied, "
		      "use \"SHOW INFO\" for server information");
    stream_write(str, "\r\n", 2);
}

static int
auth_init(void *ptr)
{
    if (user_db == NULL) {
	logmsg(L_ERR, 0, "auth capability required but user database is "
	       "not configured");
	return 1;
    }
    return 0;
}

void
register_auth()
{
    static struct dictd_command cmd =
	{ "AUTH", 3, "user string", "provide authentication information",
	  dictd_auth };
    dictd_capa_register("auth", &cmd, auth_init, NULL);
}
