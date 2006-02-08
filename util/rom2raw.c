/*****************************************************************************
 * rom2raw -- extracts user code from HP-41 ROM images
 * Copyright (C) 2006  Thomas Okken
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *****************************************************************************/

/* TODO: Detect synthetic instructions, e.g. STO M etc.; if ignored, they
 * will either become valid HP-42S instructions that don't do what the
 * original code expects (e.g. STO 117), or they will get dropped (Free42)
 * or may cause the calculator to choke (Emu42).
 * TODO: When displaying strings to the user, do 42S->ASCII translation.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/* The following is a list of all XROM numbers used by the HP-42S. Some of
 * these match functions that may occur in HP-41 code. When such XROM numbers
 * are found, rom2raw will *not* warn about them; all others are flagged as
 * being potentially problematic XROM calls.
 * NOTE: XROM 01,33 - 01,38 are hyperbolics and their inverses. They match the
 * corresponding XROM numbers in the Math module, but in that module, they are
 * implemented in user code. When rom2raw is used to convert the Math module,
 * these XROMs will be converted to XEQ commands, which is probably just as
 * well; if they are encountered in any other ROM, they will be left alone,
 * which means the HP-42S/Free42 machine code implementations will execute.
 * NOTE: XROM 29,27 - 29,32 are functions specific to the unidirectional
 * infrared printer interface. The IR Printer module also has these functions,
 * but the XROM numbers are different (see table below). I could handle this by
 * remapping those XROMs, but then again, why bother: I doubt that any ROM
 * exists that actually calls these functions. This *is* something to consider
 * when writing a generic HP-41-to-42S user code translator, though.
 *         +------------+------------+
 *         | IR Printer |   HP-42S   |
 * +-------+------------+------------+
 * | MAN   | XROM 29,28 | XROM 29,27 |
 * | NORM  | XROM 29,31 | XROM 29,28 |
 * | TRACE | XROM 29,38 | XROM 29,29 |
 * | PRON  | XROM 29,33 | XROM 29,30 |
 * | PROFF | XROM 29,32 | XROM 29,31 |
 * | DELAY | XROM 29,27 | XROM 29,32 |
 * +-------+------------+------------+
 */

typedef struct {
    int number;
    int allow;
    char *name;
} xrom_spec;

xrom_spec hp42s_xroms[] = {
    /* XROM 01,33 */ 0x061, 1, "SINH",      /* Math (see notes, above) */
    /* XROM 01,34 */ 0x062, 1, "COSH",      /* Math (see notes, above) */
    /* XROM 01,35 */ 0x063, 1, "TANH",      /* Math (see notes, above) */
    /* XROM 01,36 */ 0x064, 1, "ASINH",     /* Math (see notes, above) */
    /* XROM 01,37 */ 0x065, 1, "ATANH",     /* Math (see notes, above) */
    /* XROM 01,38 */ 0x066, 1, "ACOSH",     /* Math (see notes, above) */
    /* XROM 01,47 */ 0x06F, 0, "COMB",
    /* XROM 01,48 */ 0x070, 0, "PERM",
    /* XROM 01,49 */ 0x071, 0, "RAN",
    /* XROM 01,50 */ 0x072, 0, "COMPLEX",
    /* XROM 01,51 */ 0x073, 0, "SEED",
    /* XROM 01,52 */ 0x074, 0, "GAMMA",
    /* XROM 02,31 */ 0x09F, 0, "BEST",
    /* XROM 02,32 */ 0x0A0, 0, "EXPF",
    /* XROM 02,33 */ 0x0A1, 0, "LINF",
    /* XROM 02,34 */ 0x0A2, 0, "LOGF",
    /* XROM 02,35 */ 0x0A3, 0, "PWRF",
    /* XROM 02,36 */ 0x0A4, 0, "SLOPE",
    /* XROM 02,37 */ 0x0A5, 0, "SUM",
    /* XROM 02,38 */ 0x0A6, 0, "YINT",
    /* XROM 02,39 */ 0x0A7, 0, "CORR",
    /* XROM 02,40 */ 0x0A8, 0, "FCSTX",
    /* XROM 02,41 */ 0x0A9, 0, "FCSTY",
    /* XROM 02,42 */ 0x0AA, 0, "INSR",
    /* XROM 02,43 */ 0x0AB, 0, "DELR",
    /* XROM 02,44 */ 0x0AC, 0, "WMEAN",
    /* XROM 02,45 */ 0x0AD, 0, "LINSigma",
    /* XROM 02,46 */ 0x0AE, 0, "ALLSigma",
    /* XROM 03,34 */ 0x0E2, 0, "HEXM",
    /* XROM 03,35 */ 0x0E3, 0, "DECM",
    /* XROM 03,36 */ 0x0E4, 0, "OCTM",
    /* XROM 03,37 */ 0x0E5, 0, "BINM",
    /* XROM 03,38 */ 0x0E6, 0, "BASE+",
    /* XROM 03,39 */ 0x0E7, 0, "BASE-",
    /* XROM 03,40 */ 0x0E8, 0, "BASE*",
    /* XROM 03,41 */ 0x0E9, 0, "BASE/",
    /* XROM 03,42 */ 0x0EA, 0, "BASE+/-",
    /* XROM 09,25 */ 0x259, 0, "POLAR",
    /* XROM 09,26 */ 0x25A, 0, "RECT",
    /* XROM 09,27 */ 0x25B, 0, "RDX.",
    /* XROM 09,28 */ 0x25C, 0, "RDX,",
    /* XROM 09,29 */ 0x25D, 0, "ALL",
    /* XROM 09,30 */ 0x25E, 0, "MENU",
    /* XROM 09,31 */ 0x25F, 0, "X>=0?",
    /* XROM 09,32 */ 0x260, 0, "X>=Y?",
    /* XROM 09,34 */ 0x262, 0, "CLKEYS",
    /* XROM 09,35 */ 0x263, 0, "KEYASN",
    /* XROM 09,36 */ 0x264, 0, "LCLBL",
    /* XROM 09,37 */ 0x265, 0, "REAL?",
    /* XROM 09,38 */ 0x266, 0, "MAT?",
    /* XROM 09,39 */ 0x267, 0, "CPX?",
    /* XROM 09,40 */ 0x268, 0, "STR?",
    /* XROM 09,42 */ 0x26A, 0, "CPXRES",
    /* XROM 09,43 */ 0x26B, 0, "REALRES",
    /* XROM 09,44 */ 0x26C, 0, "EXITALL",
    /* XROM 09,45 */ 0x26D, 0, "CLMENU",
    /* XROM 09,46 */ 0x26E, 0, "GETKEY",
    /* XROM 09,47 */ 0x26F, 0, "CUSTOM",
    /* XROM 09,48 */ 0x270, 0, "ON",
    /* XROM 22,07 */ 0x587, 1, "NOT",       /* Advantage */
    /* XROM 22,08 */ 0x588, 1, "AND",       /* Advantage */
    /* XROM 22,09 */ 0x589, 1, "OR",        /* Advantage */
    /* XROM 22,10 */ 0x58A, 1, "XOR",       /* Advantage */
    /* XROM 22,11 */ 0x58B, 1, "ROTXY",     /* Advantage */
    /* XROM 22,12 */ 0x58C, 1, "BIT?",      /* Advantage */
    /* XROM 24,49 */ 0x631, 1, "AIP",       /* Advantage */
    /* XROM 25,01 */ 0x641, 1, "ALENG",     /* Extended Functions */
    /* XROM 25,06 */ 0x646, 1, "AROT",      /* Extended Functions */
    /* XROM 25,07 */ 0x647, 1, "ATOX",      /* Extended Functions */
    /* XROM 25,28 */ 0x65C, 1, "POSA",      /* Extended Functions */
    /* XROM 25,47 */ 0x66F, 1, "XTOA",      /* Extended Functions */
    /* XROM 25,56 */ 0x678, 1, "SigmaREG?", /* CX Extended Functions */
    /* XROM 27,09 */ 0x6C9, 0, "TRANS",
    /* XROM 27,10 */ 0x6CA, 0, "CROSS",
    /* XROM 27,11 */ 0x6CB, 0, "DOT",
    /* XROM 27,12 */ 0x6CC, 0, "DET",
    /* XROM 27,13 */ 0x6CD, 0, "UVEC",
    /* XROM 27,14 */ 0x6CE, 0, "INVRT",
    /* XROM 27,15 */ 0x6CF, 0, "FNRM",
    /* XROM 27,16 */ 0x6D0, 0, "RSUM",
    /* XROM 27,17 */ 0x6D1, 0, "R<>R",
    /* XROM 27,18 */ 0x6D2, 0, "I+",
    /* XROM 27,19 */ 0x6D3, 0, "I-",
    /* XROM 27,20 */ 0x6D4, 0, "J+",
    /* XROM 27,21 */ 0x6D5, 0, "J-",
    /* XROM 27,22 */ 0x6D6, 0, "STOEL",
    /* XROM 27,23 */ 0x6D7, 0, "RCLEL",
    /* XROM 27,24 */ 0x6D8, 0, "STOIJ",
    /* XROM 27,25 */ 0x6D9, 0, "RCLIJ",
    /* XROM 27,26 */ 0x6DA, 0, "NEWMAT",
    /* XROM 27,27 */ 0x6DB, 0, "OLD",
    /* XROM 27,28 */ 0x6DC, 0, "left",
    /* XROM 27,29 */ 0x6DD, 0, "right",
    /* XROM 27,30 */ 0x6DE, 0, "up",
    /* XROM 27,31 */ 0x6DF, 0, "down",
    /* XROM 27,33 */ 0x6E1, 0, "EDIT",
    /* XROM 27,34 */ 0x6E2, 0, "WRAP",
    /* XROM 27,35 */ 0x6E3, 0, "GROW",
    /* XROM 27,39 */ 0x6E7, 0, "DIM?",
    /* XROM 27,40 */ 0x6E8, 0, "GETM",
    /* XROM 27,41 */ 0x6E9, 0, "PUTM",
    /* XROM 27,42 */ 0x6EA, 0, "[MIN]",
    /* XROM 27,43 */ 0x6EB, 0, "[MAX]",
    /* XROM 27,44 */ 0x6EC, 0, "[FIND]",
    /* XROM 27,45 */ 0x6ED, 0, "RNRM",
    /* XROM 29,08 */ 0x748, 1, "PRA",       /* Printer */
    /* XROM 29,18 */ 0x752, 1, "PRSigma",   /* Printer */
    /* XROM 29,19 */ 0x753, 1, "PRSTK",     /* Printer */
    /* XROM 29,20 */ 0x754, 1, "PRX",       /* Printer */
    /* XROM 29,27 */ 0x75B, 0, "MAN",       /* see notes, above */
    /* XROM 29,28 */ 0x75C, 0, "NORM",      /* see notes, above */
    /* XROM 29,29 */ 0x75D, 0, "TRACE",     /* see notes, above */
    /* XROM 29,30 */ 0x75E, 0, "PON",       /* see notes, above */
    /* XROM 29,31 */ 0x75F, 0, "POFF",      /* see notes, above */
    /* XROM 29,32 */ 0x760, 0, "DELAY",     /* see notes, above */
    /* XROM 29,33 */ 0x761, 0, "PRUSR",
    /* XROM 29,34 */ 0x762, 0, "PRLCD",
    /* XROM 29,35 */ 0x763, 0, "CLLCD",
    /* XROM 29,36 */ 0x764, 0, "AGRAPH",
    /* XROM 29,37 */ 0x765, 0, "PIXEL",
    /* sentinel */      -1, 0, NULL
};

char *fallback_argv[] = { "foo", "-", NULL };

int entry[1024], entry_index[1024], mach_entry[1024];
int rom[65536], rom_size;
int pages, rom_number[16], num_func[16];

int entry_index_compar(const void *ap, const void *bp) {
    int a = *((int *) ap);
    int b = *((int *) bp);
    return entry[a] - entry[b];
}

unsigned char chartrans[] = {
    31, 31, 31, 16, 31, 31, 31, 14, 31, 31, 31, 31, 17, 23, 31, 31,
    31, 31, 38, 20, 20, 22, 22, 28, 28, 29, 29, 25, 25, 12, 18, 30,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 7, 124, 15, 5, 127
};

void string_convert(int len, unsigned char *s) {
    int i;
    for (i = 0; i < len; i++)
	s[i] = chartrans[s[i] & 127];
}

void getname(char *dest, int src, int space_to_underscore) {
    int c;
    char k;
    char *d = dest;
    do {
	c = rom[--src];
	k = c & 127;
	if (k >= 0 && k <= 31)
	    k += 64;
	if (k == ' ' && space_to_underscore)
	    k = '_';
	*d++ = k;
    } while ((c & 128) == 0 && src > 0);
    *d = 0;
    string_convert(strlen(dest), dest);
}

int xrom2index(int modnum, int funcnum) {
    int res = 0;
    int p;
    for (p = 0; p < pages; p++) {
	if (rom_number[p] == modnum)
	    break;
	res += num_func[p];
    }
    return res + funcnum - 1;
}

int main(int argc, char *argv[]) {
    int argnum;
    FILE *in, *out;
    int pos;
    char rom_name[256], buf[256];
    int f, e, i, j, p, total_func;
    int used_xrom[2048];
    int machine_code_warning;

    if (argc == 1) {
	argc = 2;
	argv = fallback_argv;
    }

    for (argnum = 1; argnum < argc; argnum++) {
	printf("\n");
	if (strcmp(argv[argnum], "-") != 0) {
	    in = fopen(argv[argnum], "rb");
	    if (in == NULL) {
		int err = errno;
		printf("Can't open \"%s\" for reading: %s (%d)\n",
					    argv[argnum], strerror(err), err);
		continue;
	    }
	} else
	    in = stdin;

	printf("Input file: %s\n", argv[argnum]);

	rom_size = 0;
	while (rom_size < 65536) {
	    int c1, c2;
	    c1 = fgetc(in);
	    if (c1 == EOF)
		break;
	    c2 = fgetc(in);
	    if (c2 == EOF)
		break;
	    rom[rom_size++] = (c1 << 8) | c2;
	}
	if (strcmp(argv[argnum], "-") != 0)
	    fclose(in);

	num_func[0] = rom[1] - 1;
	if (num_func[0] == 0 || num_func[0] > 63)
	    num_func[0] = 63;

	pos = ((rom[2] & 255) << 8) | (rom[3] & 255);
	if (pos <= num_func[0] * 2 + 2 || pos >= rom_size) {
	    printf("Bad offset to ROM name (%d, rom size = %d), "
			    "using \"foo\" instead.\n", pos, rom_size);
	    strcpy(rom_name, "foo");
	} else
	    getname(rom_name, pos, 1);

	printf("Output file: %s.raw\n", rom_name);
	printf("ROM Name: %s\n", rom_name);
	pages = (rom_size + 4095) / 4096;
	printf("ROM Size: %d (0x%03X), %d pages\n", rom_size, rom_size, pages);

	machine_code_warning = 0;
	total_func = 0;

	for (p = 0; p < pages; p++) {
	    int page_base = 4096 * p;
	    printf("Page %d ---\n", p);
	    rom_number[p] = rom[page_base];
	    if (rom_number[p] > 31) {
		printf("Bad ROM number (%d), using %d instead.\n",
					    rom_number[p], rom_number[p] & 31);
		rom_number[p] &= 31;
	    } else
		printf("ROM Number: %d\n", rom_number[p]);
	    num_func[p] = rom[page_base + 1] - 1;
	    if (num_func[p] <= 0 || num_func[p] > 63) {
		printf("Bad function count (%d), skipping this page.\n",
					    num_func[p]);
		num_func[p] = 0;
		continue;
	    }

	    printf("%d functions (XROM %02d,01 - %02d,%02d)\n",
			num_func[p], rom_number[p], rom_number[p], num_func[p]);

	    for (f = 1; f <= num_func[p]; f++) {
		int mcode;
		e = (rom[page_base + f * 2 + 2] << 8)
			| (rom[page_base + f * 2 + 3] & 255);
		mcode = (e & 0x20000) == 0;
		e &= 0xffff;
		if (e >= 0x8000)
		    e -= 0x10000;
		e += page_base;
		if (mcode) {
		    if (e < 0 || e >= rom_size) {
			printf("Bad machine code entry point for "
				"XROM %02d,%02d: 0x%03X.\n",
				rom_number[p], f, e);
			mach_entry[total_func] = 0;
		    } else {
			getname(buf, e, 0);
			printf("XROM %02d,%02d: machine code %s\n",
							rom_number[p], f, buf);
			mach_entry[total_func] = e;
			machine_code_warning = 1;
		    }
		    entry[total_func] = 0;
		} else {
		    e &= 0xFFFF;
		    if (e >= rom_size - 5) {
			printf("Bad user code entry point for "
				"XROM %02d,%02d: 0x%03X, skipping.\n",
				rom_number[p],
				f, e);
			entry[total_func] = 0;
		    } else if (rom[e] & 0xF0 != 0xC0
				|| rom[e + 2] < 0xF2 || rom[e + 2] > 0xF8) {
			printf("User code entry point (0x%03X) from "
				"XROM %02d,%02d does not point to a "
				"global label; skipping.\n",
				e, rom_number[p], f);
			entry[total_func] = 0;
		    } else {
			entry[total_func] = e;
			for (i = 0; i < (rom[e + 2] & 15) - 1; i++)
			    buf[i] = rom[e + 4 + i];
			buf[i] = 0;
			string_convert(i, buf);
			printf("XROM %02d,%02d: user code \"%s\"\n",
						    rom_number[p], f, buf);
		    }
		}
		entry_index[total_func] = total_func;
		total_func++;
	    }
	}

	if (machine_code_warning)
	    printf("Warning: this ROM contains machine code; "
		    "this code cannot be translated.\n");

	qsort(entry_index, total_func, sizeof(int), entry_index_compar);
	f = 0;
	while (entry[entry_index[f]] == 0 && f < total_func)
	    f++;

	strcpy(buf, rom_name);
	strcat(buf, ".raw");
	out = fopen(buf, "wb");
	if (out == NULL) {
	    int err = errno;
	    printf("Can't open \"%s\" for writing: %s (%d)\n",
						    buf, strerror(err), err);
	    continue;
	}

	for (i = 0; i < 2048; i++)
	    used_xrom[i] = 0;

	pos = 0;
	while (f < total_func) {
	    unsigned char instr[16];
	    int c, k;
	    if (entry[entry_index[f]] < pos) {
		f++;
		continue;
	    }
	    pos = entry[entry_index[f]];
	    do {
		i = 0;
		do {
		    c = rom[pos++];
		    instr[i++] = c & 255;
		} while ((c & 512) == 0 && (rom[pos] & 256) == 0);
		k = instr[0];
		if (k >= 0x1D && k <= 0x1F) {
		    /* GTO/XEQ/W <alpha> */
		    string_convert(instr[1] & 15, instr + 2);
		} else if (k >= 0xB1 && k <= 0xBF) {
		    /* Short-form GTO; wipe out offset (second byte) */
		    instr[1] = 0;
		} else if (k >= 0xC0 && k <= 0xCD) {
		    /* Global; wipe out offset
		     * (low nybble of 1st byte + all of 2nd byte)
		     */
		    instr[0] &= 0xF0;
		    instr[1] = 0;
		    if (instr[2] < 0xF1)
			/* END */
			instr[2] = 0x0D;
		    else
			string_convert((instr[2] & 15) - 1, instr + 4);
		} else if (k >= 0xD0 && k <= 0xEF) {
		    /* Long-form GTO, and XEQ: wipe out offset
		     * (low nybble of 1st byte + all of 2nd byte)
		     */
		    instr[0] &= 0xF0;
		    instr[1] = 0;
		} else if (k >= 0xA0 && k <= 0xA7) {
		    /* XROM */
		    int num = ((k & 7) << 8) | instr[1];
		    int modnum = num >> 6;
		    int instnum = num & 63;
		    int islocal = 0;
		    for (p = 0; p < pages; p++)
			if (num_func[p] != 0 && rom_number[p] == modnum) {
			    islocal = 1;
			    break;
			}
		    if (islocal) {
			/* Local XROM */
			int idx = xrom2index(modnum, instnum);
			if (entry[idx] == 0) {
			    /* Mcode XROM, can't translate */
			    used_xrom[num] = 1;
			} else {
			    /* User code XROM, translate to XEQ */
			    int len = (rom[entry[idx] + 2] & 15) - 1;
			    instr[0] = 0x1E;
			    instr[1] = 0xF0 + len;
			    for (i = 0; i < len; i++)
				instr[i + 2] = rom[entry[idx] + 4 + i];
			    i = len + 2;
			    string_convert(len, instr + 2);
			}
		    } else {
			/* Nonlocal XROM;
			 * we'll separate the HP-42S XROMs out later
			 */
			used_xrom[num] = 2;
		    }
		} else if (k > 0xF0) {
		    string_convert(k & 15, instr + 1);
		}
		for (j = 0; j < i; j++)
		    fputc(instr[j], out);
	    } while ((c & 512) == 0);
	}

	fclose(out);

	/* Don't complain about XROMs that match HP-42S instructions
	 * if those are indeed the same instructions as in the corresponding
	 * HP-41 ROMs; complain about all the others.
	 */
	for (i = 0; hp42s_xroms[i].number != -1; i++) {
	    if (hp42s_xroms[i].allow)
		used_xrom[hp42s_xroms[i].number] = 0;
	    else if (used_xrom[hp42s_xroms[i].number] != 0)
		used_xrom[hp42s_xroms[i].number] = 3;
	}

	j = 0;
	for (i = 0; i < 2048; i++) {
	    if (used_xrom[i] == 1) {
		int p;
		if (j == 0) {
		    j = 1;
		    printf("\nThe following machine code XROMs were called "
			    "from user code:\n");
		}
		p = mach_entry[xrom2index(i >> 6, i & 63)];
		if (p == 0)
		    strcpy(buf, "(bad entry point)");
		else
		    getname(buf, p, 0);
		printf("XROM %02d,%02d: %s\n", i >> 6, i & 63, buf);
	    }
	}

	for (i = 0; i < 2048; i++) {
	    if (used_xrom[i] == 2) {
		if (j < 2) {
		    if (j == 0)
			printf("\n");
		    j = 2;
		    printf("The following non-local XROMs were called "
			    "from user code:\n");
		}
		printf("XROM %02d,%02d\n", i >> 6, i & 63);
	    }
	}

	for (i = 0; i < 2048; i++) {
	    if (used_xrom[i] == 3) {
		int p;
		if (j < 3) {
		    if (j == 0)
			printf("\n");
		    j = 3;
		    printf("The following XROMs were called which are "
			    "going to be\nmistaken for HP-42S commands:\n");
		}
		for (f = 0; hp42s_xroms[f].number != i; f++);
		p = mach_entry[xrom2index(i >> 6, i & 63)];
		if (p == 0)
		    strcpy(buf, "(bad entry point)");
		else
		    getname(buf, p, 0);
		printf("XROM %02d,%02d: %s => %s\n", i >> 6, i & 63, buf,
						    hp42s_xroms[f].name);
	    }
	}

	if (j != 0)
	    printf("Because of these XROM calls, "
		    "the converted user code may not work.\n");
    }

    return 0;
}
