/* radare - LGPL - Copyright 2014-2023 - pancake */

#include <r_debug.h>

#define DB dbg->sgnls

// TODO: this must be done by the debugger plugin
// which is stored already in SDB.. but this is faster :P
static const struct {
	const char *k;
	const char *v;
} signals[] = {
	// hardcoded from linux
	{ "SIGHUP", "1" },
	{ "SIGINT", "2" },
	{ "SIGQUIT", "3" },
	{ "SIGILL", "4" },
	{ "SIGTRAP", "5" },
	{ "SIGABRT", "6" },
	// { "SIGIOT", "6" },
	{ "SIGBUS", "7" },
	{ "SIGFPE", "8" },
	{ "SIGKILL", "9" },
	{ "SIGUSR1", "10" },
	{ "SIGSEGV", "11" },
	{ "SIGUSR2", "12" },
	{ "SIGPIPE", "13" },
	{ "SIGALRM", "14" },
	{ "SIGTERM", "15" },
	{ "SIGSTKFLT", "16" },
	{ "SIGCHLD", "17" },
	{ "SIGCONT", "18" },
	{ "SIGSTOP", "19" },
	{ "SIGTSTP", "20" },
	{ "SIGTTIN", "21" },
	{ "SIGTTOU", "22" },
	{ "SIGURG", "23" },
	{ "SIGXCPU", "24" },
	{ "SIGXFSZ", "25" },
	{ "SIGVTALRM", "26" },
	{ "SIGPROF", "27" },
	{ "SIGWINCH", "28" },
	{ "SIGIO", "29" },
	{ "SIGPOLL", "SIGIO" },
	{ "SIGLOST", "29" },
	{ "SIGPWR", "30" },
	{ "SIGSYS", "31" },
	{ "SIGRTMIN", "32" },
	{ "SIGRTMAX", "NSIG" },
	{ NULL }
};

R_API void r_debug_signal_fini(RDebug *dbg) {
	sdb_free (DB);
}

R_API void r_debug_signal_init(RDebug *dbg) {
	int i;
	// XXX
	DB = sdb_new (NULL, "signals", 0);
	for (i = 0; signals[i].k; i++) {
		sdb_set (DB, signals[i].k, signals[i].v, 0);
		sdb_set (DB, signals[i].v, signals[i].k, 0);
	}
}

typedef struct {
	RStrBuf *sb;
	RDebug *dbg;
} SignalContext;

static bool siglistcb(void *p, const char *k, const char *v) {
	SignalContext *ctx = (SignalContext *)p;
	RDebug *dbg = ctx->dbg;
	if (atoi (k) > 0) {
		int mode = dbg->_mode;
		char *key = r_str_newf ("cfg.%s", k);
		int opt = sdb_num_get (DB, key, 0);
		free (key);
		if (opt) {
			r_strbuf_appendf (ctx->sb, "%s %s", k, v);
			if (opt & R_DBG_SIGNAL_CONT) {
				r_strbuf_append (ctx->sb, " cont");
			}
			if (opt & R_DBG_SIGNAL_SKIP) {
				r_strbuf_append (ctx->sb, " skip");
			}
			r_strbuf_append (ctx->sb, "\n");
		} else {
			if (mode == 0) {
				r_strbuf_appendf (ctx->sb, "%s %s\n", k, v);
			}
		}
	}
	return true;
}

static bool siglistjsoncb(void *p, const char *k, const char *v) {
	SignalContext *ctx = (SignalContext *)p;
	static char key[32] = "cfg.";
	PJ *pj = ctx->dbg->pj;
	int opt;
	if (atoi (k) > 0) {
		strncpy (key + 4, k, 20);
		opt = (int)sdb_num_get (ctx->dbg->sgnls, key, 0);
		pj_o (pj);
		pj_ks (pj, "signum", k);
		pj_ks (pj, "name", v);
		pj_k (pj, "option");
		if (opt & R_DBG_SIGNAL_CONT) {
			pj_s (pj, "cont");
		} else if (opt & R_DBG_SIGNAL_SKIP) {
			pj_s (pj, "skip");
		} else {
			pj_null (pj);
		}
		pj_end (pj);
	}
	return true;
}

R_API char *r_debug_signal_list(RDebug *dbg, int mode) {
	RStrBuf *sb = r_strbuf_new ("");
	SignalContext ctx = { sb, dbg };
	dbg->_mode = mode;
	switch (mode) {
	case 0:
	case 1:
		sdb_foreach (DB, siglistcb, &ctx);
		break;
	case 2:
		if (dbg->pj) {
			pj_a (dbg->pj);
			sdb_foreach (DB, siglistjsoncb, &ctx);
			pj_end (dbg->pj);
			r_strbuf_append (sb, pj_string (dbg->pj));
		}
		break;
	}
	dbg->_mode = 0;
	return r_strbuf_drain (sb);
}

R_API int r_debug_signal_send(RDebug *dbg, int num) {
	return r_sandbox_kill (dbg->pid, num);
}

R_API void r_debug_signal_setup(RDebug *dbg, int num, int opt) {
	sdb_queryf (DB, "cfg.%d=%d", num, opt);
}

R_API int r_debug_signal_what(RDebug *dbg, int num) {
	char k[32];
	snprintf (k, sizeof (k), "cfg.%d", num);
	return sdb_num_get (DB, k, 0);
}

R_API int r_debug_signal_set(RDebug *dbg, int num, ut64 addr) {
	// TODO
	// r_debug_syscall (dbg, "signal", "addr");
	return 0;
}

/* TODO rename to _kill_ -> _signal_ */
R_API RList *r_debug_kill_list(RDebug *dbg) {
	if (dbg->current->plugin->kill_list) {
		return dbg->current->plugin->kill_list (dbg);
	}
	return NULL;
}

R_API int r_debug_kill_setup(RDebug *dbg, int sig, int action) {
	R_LOG_TODO ("set signal handlers of child");
	// TODO: must inject code to call signal()
#if 0
	if (dbg->current->plugin.kill_setup)
		return dbg->current->plugin.kill_setup (dbg, sig, action);
#endif
	// TODO: implement r_debug_kill_setup
	return false;
}
