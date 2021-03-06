/* radare - LGPL - Copyright 2009-2013 - pancake */

static int cmd_flag(void *data, const char *input) {
	RCore *core = (RCore *)data;
	ut64 off = core->offset;
	char *ptr, *str = NULL;
	st64 base;

	if (*input)
		str = strdup (input+1);
	switch (*input) {
	case 'b':
		switch (input[1]) {
		case ' ':
			str = strdup (input+2);
			ptr = strchr (str, ' ');
			if (ptr) {
				RListIter *iter;
				RFlagItem *flag;
				RFlag *f = core->flags;
				*ptr = 0;
				base = r_num_math (core->num, str);
				r_list_foreach (f->flags, iter, flag) {
					if (r_str_glob (flag->name, ptr+1))
						flag->offset += base;
				}
			} else core->flags->base = r_num_math (core->num, input+1);
			free (str);
			str = NULL;
			break;
		case '\0':
			r_cons_printf ("%"PFMT64d" 0x%"PFMT64x"\n",
				core->flags->base,
				core->flags->base);
			break;
		default:
			eprintf ("Usage: fb [addr] [[flags*]]\n");
			break;
		}
		break;
	case '+':
	case ' ': {
		char *s = NULL, *s2 = NULL, *eq = strchr (str, '=');
		ut32 bsze = 1; //core->blocksize;
		if (eq) {
			// TODO: add support for '=' char in flag comments
			*eq = 0;
			off = r_num_math (core->num, eq+1);
		}
		s = strchr (str, ' ');
		if (s) {
			*s = '\0';
			s2 = strchr (s+1, ' ');
			if (s2) {
				*s2 = '\0';
				if (s2[1]&&s2[2])
					off = r_num_math (core->num, s2+1);
			}
			bsze = r_num_math (core->num, s+1);
		}
		r_flag_set (core->flags, str, off, bsze, (*input=='+'));
		}
		break;
	case '-':
		if (input[1]) {
			const char *flagname = input+1;
			while (*flagname==' ') flagname++;
			if (strchr (input+1, '*'))
				r_flag_unset_glob (core->flags, flagname);
			else r_flag_unset (core->flags, flagname, NULL);
		} else r_flag_unset_i (core->flags, off, NULL);
		break;
	case 'l':
		if (input[1] == ' ') {
			RFlagItem *item = r_flag_get_i (core->flags,
				r_num_math (core->num, input+2));
			if (item)
				r_cons_printf ("0x%08"PFMT64x"\n", item->offset);
		} else eprintf ("Missing arguments\n");
		break;
#if 0
	case 'd':
		if (input[1] == ' ') {
			char cmd[128];
			RFlagItem *item = r_flag_get_i (core->flags,
				r_num_math (core->num, input+2));
			if (item) {
				r_cons_printf ("0x%08"PFMT64x"\n", item->offset);
				snprintf (cmd, sizeof (cmd), "pD@%"PFMT64d":%"PFMT64d,
					 item->offset, item->size);
				r_core_cmd0 (core, cmd);
			}
		} else eprintf ("Missing arguments\n");
		break;
#endif
	case 'x':
		if (input[1] == ' ') {
			char cmd[128];
			RFlagItem *item = r_flag_get_i (core->flags,
				r_num_math (core->num, input+2));
			if (item) {
				r_cons_printf ("0x%08"PFMT64x"\n", item->offset);
				snprintf (cmd, sizeof (cmd), "px@%"PFMT64d":%"PFMT64d,
					 item->offset, item->size);
				r_core_cmd0 (core, cmd);
			}
		} else eprintf ("Missing arguments\n");
		break;
	case 'S':
		r_flag_sort (core->flags, (input[1]=='n'));
		break;
	case 's':
		switch (input[1]) {
		case 'r':
			if (input[2]==' ')
				r_flag_space_rename (core->flags, NULL, input+2);
			else eprintf ("Usage: fsr [newname]\n");
			break;
		case 'j':
		case '\0':
		case '*':
			r_flag_space_list (core->flags, input[1]);
			break;
		case ' ':
			r_flag_space_set (core->flags, input+2);
			break;
		case 'm':
			{ RFlagItem *f;
			ut64 off = core->offset;
			if (input[2] == ' ')
				off = r_num_math (core->num, input+2);
			f = r_flag_get_i (core->flags, off);
			if (f) {
				f->space = core->flags->space_idx;
			} else eprintf ("Cannot find any flag at 0x%"PFMT64x".\n", off);
			}
			break;
		default: {
			int i, j = 0;
			for (i=0; i<R_FLAG_SPACES_MAX; i++) {
				if (core->flags->spaces[i])
					r_cons_printf ("%02d %c %s\n", j++,
					(i==core->flags->space_idx)?'*':' ',
					core->flags->spaces[i]);
			}
			} break;
		}
		break;
	case 'g':
		eprintf ("radare2: fg: current: no such job :)\n");
		break;
	case 'c':
		eprintf ("TODO: fc\n");
		break;
	case 'C':
		if (input[1]==' ') {
			RFlagItem *item;
			char *q, *p = strdup (input+2);
			q = strchr (p, ' ');
			if (q) {
				*q = 0;
				item = r_flag_get (core->flags, p);
				if (item) {
					r_flag_item_set_comment (item, q+1);
				} else eprintf ("Cannot find flag with name '%s'\n", p);
			} else {
				item = r_flag_get_i (core->flags, r_num_math (core->num, p));
				if (item && item->comment) {
					r_cons_printf ("%s\n", item->comment);
				} else eprintf ("Cannot find item\n");
			}
			free (p);
		} else eprintf ("Usage: fC [name] [comment]\n");
		break;
	case 'o':
		{ // TODO: use file.fortunes // can be dangerous in sandbox mode
			char *file = R2_PREFIX"/share/doc/radare2/fortunes";
			char *line = r_file_slurp_random_line (file);
			if (line) {
				r_cons_printf (" -- %s\n", line);
				free (line);
			}
		}
		break;
	case 'r':
		if (input[1]==' ' && input[2]) {
			char *old, *new;
			RFlagItem *item;
			old = str+1;
			new = strchr (old, ' ');
			if (new) {
				*new = 0;
				new++;
				item = r_flag_get (core->flags, old);
			} else {
				new = old;
				item = r_flag_get_i (core->flags, core->offset);
			}
			if (item) {
				if (!r_flag_rename (core->flags, item, new))
					eprintf ("Invalid name\n");
			} else eprintf ("Cannot find flag\n");
		}
		break;
	case '*':
	case '\0':
	case 'j':
		r_flag_list (core->flags, *input);
		break;
	case 'd':
		{
			ut64 addr = 0;
			RFlagItem *f = NULL;
			switch (input[1]) {
			case '?':
				eprintf ("Usage: fd [offset|flag|expression]\n");
				return R_FALSE;
			case '\0':
				addr = core->offset;
				break;
			default:
				addr = r_num_math (core->num, input+2);
				break;
			}
			f = r_flag_get_at (core->flags, addr);
			if (f) {
				if (f->offset != addr) {
					r_cons_printf ("%s + %d\n", f->name, (int)(addr-f->offset));
				} else r_cons_printf ("%s\n", f->name);
			}
		}
		break;
	case '?':
		r_cons_printf (
		"Usage: f[?] [flagname]\n"
		" f                ; list flags\n"
		" f*               ; list flags in r commands\n"
		" fj               ; list flags in JSON format\n"
		" fs               ; display flagspaces\n"
		" fs *             ; set all flagspace\n"
		" fs sections      ; set flagspace (f will only list flags from selected ones)\n"
		" fsr newname      ; set flagspace (f will only list flags from selected ones)\n"
		" fsm [addr]       ; move flags at given address to the current flagspace\n"
		" fb [addr]        ; set base address for new flags\n"
		" fb [addr] [flag*]; move flags matching 'flag' to relative addr\n"
		" f name 12 @ 33   ; set flag 'name' with length 12 at offset 33\n"
		" f name = 33      ; alias for 'f name @ 33' or 'f name 1 33'\n"
		" f name 12 33     ; same as above\n"
		" f name 12 33 cmt ; same as above + set flag comment\n"
		" f+name 12 @ 33   ; like above but creates new one if doesnt exist\n"
		" f-name           ; remove flag 'name'\n"
		" f-@addr          ; remove flag at address expression\n"
		" fd addr          ; return flag+delta\n"
		//" fc [name] [cmt]  ; set execution command for a specific flag\n"
		" fC [name] [cmt]  ; set comment for given flag\n"
		" fr [old] [[new]] ; rename flag (if no new flag current seek one is used)\n"
		" fl [flagname]    ; show flag length (size)\n"
		" fS[on]           ; sort flags by offset or name\n"
		" fx[d]            ; show hexdump (or disasm) of flag:flagsize\n"
		" fo               ; show fortunes\n");
		break;
	}
	if (str)
		free (str);
	return 0;
}

