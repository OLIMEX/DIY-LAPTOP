/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc boot support
 */
#include <common.h>
#include <command.h>
#include <malloc.h>

#include "../memtest/types.h"
#include "../memtest/sizes.h"
#include "../memtest/tests.h"

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04



struct test tests[] = {
    { "Random Value", test_random_value },
    { "Compare XOR", test_xor_comparison },
    { "Compare SUB", test_sub_comparison },
    { "Compare MUL", test_mul_comparison },
    { "Compare DIV",test_div_comparison },
    { "Compare OR", test_or_comparison },
    { "Compare AND", test_and_comparison },
    { "Sequential Increment", test_seqinc_comparison },
    { "Solid Bits", test_solidbits_comparison },
    { "Block Sequential", test_blockseq_comparison },
    { "Checkerboard", test_checkerboard_comparison },
    { "Bit Spread", test_bitspread_comparison },
    { "Bit Flip", test_bitflip_comparison },
    { "Walking Ones", test_walkbits1_comparison },
    { "Walking Zeroes", test_walkbits0_comparison },
    { NULL, NULL }
};

/* Sanity checks and portability helper macros. */
#ifdef _SC_VERSION
void check_posix_system(void) {
    if (sysconf(_SC_VERSION) < 198808L) {
        fprintf(stderr, "A POSIX system is required.  Don't be surprised if "
            "this craps out.\n");
        fprintf(stderr, "_SC_VERSION is %lu\n", sysconf(_SC_VERSION));
    }
}
#else
#define check_posix_system()
#endif

#ifdef _SC_PAGE_SIZE
int memtester_pagesize(void) {
    int pagesize = sysconf(_SC_PAGE_SIZE);
    if (pagesize == -1) {
        perror("get page size failed");
        exit(EXIT_FAIL_NONSTARTER);
    }
    printf("pagesize is %ld\n", (long) pagesize);
    return pagesize;
}
#else
int memtester_pagesize(void) {
    printf("sysconf(_SC_PAGE_SIZE) not supported; using pagesize of 8192\n");
    return 8192;
}
#endif

/* Some systems don't define MAP_LOCKED.  Define it to 0 here
   so it's just a no-op when ORed with other constants. */
#ifndef MAP_LOCKED
  #define MAP_LOCKED 0
#endif

/* Function declarations */
void usage(char *me);



static int do_memtester(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    ul loops, loop, i;
    size_t pagesize, wantraw, wantmb, wantbytes, wantbytes_orig, bufsize,
         halflen, count;
    char memsuffix;
    char szSize[12];
    //ptrdiff_t pagesizemask;
    int pagesizemask;
    void volatile *buf, *aligned;
    ulv *bufa, *bufb;
    int do_mlock = 1, done_mem = 0;
    int exit_code = 0;
    int  memshift;
    size_t maxbytes = CONFIG_SYS_MALLOC_LEN; /* addressable memory, in bytes */
    size_t maxmb = (maxbytes >> 20) + 1; /* addressable memory, in MB */

    printf("memtester version 4.2.1 (%d-bit)\n", UL_LEN);
    printf("Copyright (C) 2010 Charles Cazabon.\n");
    printf("Licensed under the GNU General Public License version 2 (only).\n");
    printf("\n");
    check_posix_system();
    pagesize = memtester_pagesize();
    //pagesizemask = (ptrdiff_t) ~(pagesize - 1);
    pagesizemask = (int) ~(pagesize - 1);
    printf("pagesizemask is 0x%x\n", pagesizemask);


    if ( argc < 3 ) {
        fprintf(stderr, "need memory argument, in MB\n");
        usage(argv[0]); /* doesn't return */
        return -1;
    }


	
   
    
	memsuffix = argv[1][strlen(argv[1])-1];
	//printf("%s %s %s,%d\n", argv[0], argv[1], argv[2],strlen(argv[1]));
	
	memset(szSize,sizeof(szSize),0);
	if(memsuffix ==  '\0')
		memcpy(szSize,argv[1],strlen(argv[1]));
	else
		memcpy(szSize,argv[1],strlen(argv[1])-1);
		
	 //wantraw = (size_t) strtoul(argv[optind], &memsuffix, 0);
    wantraw = simple_strtoul(szSize, NULL, 10);
	
    switch (memsuffix) {
        //case 'G':
        //case 'g':
        //    memshift = 30; /* gigabytes */
        //    break;
        case 'M':
        case 'm':
            memshift = 20; /* megabytes */
            break;
        case 'K':
        case 'k':
            memshift = 10; /* kilobytes */
            break;
        case 'B':
        case 'b':
            memshift = 0; /* bytes*/
            break;
        case '\0':  /* no suffix */
            memshift = 20; /* megabytes */
            break;
        default:
            /* bad suffix */
            usage(argv[0]); /* doesn't return */
            return -1;
    }
    wantbytes_orig = wantbytes = ((size_t) wantraw << memshift);
    wantmb = (wantbytes_orig >> 20);

    if (wantmb > maxmb) {
        printf("This system can only address %d MB.\n",  maxmb);
        return -1;
    }
    if (wantbytes < pagesize) {
        printf("bytes %d < pagesize %d -- memory argument too large?\n",
                wantbytes, pagesize);
        return -1;
    }



    loops = simple_strtoul(argv[2], NULL, 10);
    
    
    printf("want %uMB (%u bytes)\n", wantmb, wantbytes);
    buf = NULL;


    while (!done_mem) {
        while (!buf && wantbytes) {
            buf = (void volatile *) malloc(wantbytes);
            if (!buf) wantbytes -= pagesize;
        }
        bufsize = wantbytes;
        printf("got  %lluMB (%llu bytes), start addr: 0x%lx", (ull) wantbytes >> 20,
            (ull) wantbytes, (ulong)buf);
        //fflush(stdout);
        
        aligned = buf;
        done_mem = 1;
        printf("\n");

    }

    if (!do_mlock) printf( "Continuing with unlocked memory; testing "
                           "will be slower and less reliable.\n");

    halflen = bufsize / 2;
    count = halflen / sizeof(ul);
    bufa = (ulv *) aligned;
    bufb = (ulv *) ((size_t) aligned + halflen);

    for(loop=1; ((!loops) || loop <= loops); loop++) {
        printf("Loop %lu", loop);
        if (loops) {
            printf("/%lu", loops);
        }
        printf(":\n");
        printf("  %-20s: ", "Stuck Address");
        //fflush(stdout);
        if (!test_stuck_address(aligned, bufsize / sizeof(ul))) {
             printf("ok\n");
        } else {
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        for (i=0;;i++) {
            if (!tests[i].name) break;
            printf("  %-20s: ", tests[i].name);
            if (!tests[i].fp(bufa, bufb, count)) {
                printf("ok\n");
            } else {
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
            //fflush(stdout);
        }
        printf("\n");
        //fflush(stdout);
    }
   // if (do_mlock) munlock((void *) aligned, bufsize);
    printf("Done.\n");
    //fflush(stdout);
    //exit(exit_code);
    return exit_code;
}

/* Function definitions */
void usage(char *me) {
    printf("\nUsage: %s [-p physaddrbase] <mem>[B|K|M|G] [loops]\n", me);
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	memtester, CONFIG_SYS_MAXARGS, 1,	do_memtester,
	"start application at address 'addr'",
	"memtester size[M] loop\n"
);
