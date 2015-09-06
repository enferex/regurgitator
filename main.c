/******************************************************************************
 * main.c
 *
 * regurgitator - Register Gurgitator: Reports symbols stored in registers 
 *                by a callee.  These registers can provide hints into 
 *                locations into stdlib even given a randomized 
 *                address space.
 *
 * Copyright (C) 2015, Matt Davis (enferex)
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or (at
 * your option) any later version.
 *             
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more
 * details.
 *                             
 * You should have received a copy of the GNU
 * General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#define __USE_GNU
#include <dlfcn.h>

typedef unsigned long long addr_t;

/* Capture register state before and after a test */
typedef struct _result_t
{
    struct user_regs_struct before;
    struct user_regs_struct after;
    const char *fn;
    const char *args;
    struct _result_t *next;
} result_t;

/* GLobal list of results */
static result_t *results;

/* Allocate storage for a new result */
static result_t *new_result(void)
{
    result_t *res = calloc(1, sizeof(result_t));
    res->next = results;
    results = res;
    return res;
}

/* For pretty printing */
#define PREFIX "  "

#define R(_reg, _sto) \
    __asm__ __volatile__("mov %%" #_reg ", %0\n" : "=r"(_sto))

#define CL(_reg) \
    __asm__ __volatile__("mov $0, %" #_reg "\n")

#define _REGS(_regs) { \
    R(rcx, c);        \
    R(rdx, d);        \
    R(r8,  reg8);     \
    R(r9,  reg9);     \
    R(r10, reg10);    \
    R(r11, reg11);    \
    _regs.rcx = c;  _regs.rdx = d; _regs.r8 = reg8;        \
    _regs.r9 = reg9; _regs.r10 = reg10; _regs.r11 = reg11; \
    CL(rcx); CL(rdx);CL(r8); CL(r9); CL(r10); CL(r11);     \
}

#define TEST(_fn, ...) { \
    addr_t c, d, reg8, reg9, reg10, reg11; \
    result_t *res = new_result(); \
    res->fn = #_fn;               \
    res->args = #__VA_ARGS__;     \
    _REGS(res->before);           \
    _fn(__VA_ARGS__);             \
    _REGS(res->after);            \
}

/* Display the symbol names for the given addresses */
static void print_symnames(addr_t addr1, addr_t addr2)
{
    Dl_info ainfo1, ainfo2;
    const int ok1 = dladdr((void *)addr1, &ainfo1);
    const int ok2 = dladdr((void *)addr2, &ainfo2);
    printf("(%s :: %s)\n",
           ok1 ? ainfo1.dli_sname : "?", 
           ok2 ? ainfo2.dli_sname : "?");
}

static void print_regs(const result_t *res, const char *prefix)
{
#define PR_REG(_res, _pre, _r) {          \
    printf("%s%-5s %-18p :: %-18p %-9s ", \
            _pre, #_r,                    \
           (void *)_res->before._r,       \
            (void *)_res->after._r,       \
           _res->before._r != _res->after._r ? "Different" : "Same"); \
    print_symnames(_res->before._r, _res->after._r);                  \
}
    PR_REG(res, prefix, rcx);
    PR_REG(res, prefix, rdx);
    PR_REG(res, prefix, rsi);
    PR_REG(res, prefix, rdi);
    PR_REG(res, prefix, r8);
    PR_REG(res, prefix, r9);
    PR_REG(res, prefix, r10);
    PR_REG(res, prefix, r11);
}

/* Print column names based on print_regs() spacing */
static inline void print_columns(void)
{
    printf("%s%-5s %-18s    %-18s %-9s SYMBOLNAME\n",
           PREFIX, "REG", "BEFORE", "AFTER", "CHANGED");
}

static inline void print_header(void)
{
    printf(
 "--------------------------------------------------------------------------\n"
 );
}

static inline void print_footer(void)
{
    print_header();
}

static void show_results(void)
{
    int i;
    const result_t *rr;

    print_columns();
    print_header();
    for (rr=results; rr; rr=rr->next) {
        printf("Test %d ==> %s(%s)\n", ++i, rr->fn, rr->args);
        print_regs(rr, PREFIX);
        print_footer();
    }
}

/* Add more tests here */
static void tests(void)
{
    char buf[32];

    /* stdlib.h tests */
    TEST(atof, "4.2");
    TEST(atoi, "42");
    TEST(atol, "42");
    TEST(strtod, "42.0", NULL);
    TEST(srand, 42);
    TEST(rand, );
    TEST(printf, "\n");
    TEST(fprintf, stdout, "\n");
    TEST(sprintf, buf, "%s", "bar");
    TEST(malloc, (1234));
    TEST(free, NULL);
}

int main(void)
{
    tests();
    show_results();
    return 0;
}
