/* radare - LGPL - Copyright 2015-2024 - jvoisin */

#include <r_lib.h>
#include <r_asm.h>

static char *replace(int argc, const char *argv[]) {
	int i, j;
	struct {
		const char *op;
		const char *str;
	} ops[] = {
		{ "adc",  "1 = 1 + 2" },
		{ "add",  "1 = 1 + 2" },
		{ "and",  "1 = 1 & 2" },
		{ "cpl",  "1 = ~1" },
		{ "ex",   "swap(1, 2)" },
		{ "in",   "1 = [2]" },
		{ "jp",   "goto [1]" },
		{ "jp",   "goto 1" },
		{ "jr",   "goto +1" },
		{ "ld",   "1 = 2" },
		{ "ldd",  "1 = 2--" },
		{ "neg",  "1 = -1" },
		{ "nop",  "" },
		{ "or",   "1 = 1 | 2" },
		{ "pop",  "pop 1" },
		{ "push", "push 1" },
		{ "rr",   "1 = 1 << 2" },
		{ "sbc",  "1 = 1 - 2" },
		{ "sla",  "1 = 1 << 2" },
		{ "sra",  "1 = 1 >> 2" },
		{ "srl",  "1 = 1 >> 2" },
		{ "sub",  "1 = 1 - 2" },
		{ "xor",  "1 = 1 ^ 2" },
		{ NULL }
	};

	for (i = 0; ops[i].op; i++) {
		if (!strcmp (ops[i].op, argv[0])) {
			RStrBuf *sb = r_strbuf_new ("");
			for (j = 0; ops[i].str[j] != '\0'; j++) {
				if (ops[i].str[j]>='1' && ops[i].str[j] <= '9') {
					const char *w = argv[ ops[i].str[j] - '0' ];
					if (w) {
						r_strbuf_append (sb, w);
					}
				} else {
					r_strbuf_appendf (sb, "%c", ops[i].str[j]);
				}
			}
			return r_strbuf_drain (sb);
		}
	}

	RStrBuf *sb = r_strbuf_new ("");
	for (i = 0; i < argc; i++) {
		r_strbuf_append (sb, argv[i]);
		r_strbuf_append (sb, (i == argc - 1)? "": " ");
	}
	return r_strbuf_drain (sb);
}

static char *parse(RAsmPluginSession *aps, const char *data) {
	int argc = 0;
	char *args = strdup (data);
	args = r_str_replace (args, ",", " ", true);
	args = r_str_replace (args, "  ", " ", true);
	const char **argv = (const char **)r_str_argv (args, &argc);
	char *res = replace (argc, argv);
	free (args);
	if (!res) {
		return NULL;
	}
	return res;
}

#if Z80_IS_GB
RAsmPlugin r_asm_plugin_gb = {
	.meta = {
		.name = "gb",
		.desc = "GameBoy pseudo syntax",
	},
	.parse = parse,
};
#else
RAsmPlugin r_asm_plugin_z80 = {
	.meta = {
		.name = "z80",
		.desc = "Z80 pseudo syntax",
	},
	.parse = parse,
};
#endif

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin = {
	.type = R_LIB_TYPE_ASM,
	.data = &r_asm_plugin_z80,
	.version = R2_VERSION
};
#endif
