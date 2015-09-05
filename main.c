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

#define R(_reg, _sto) \
    __asm__ __volatile__("mov %%" #_reg ", %0" : "=r"(_sto):);

#define REGS(_regs) { \
    R(rcx, c);        \
    R(rdx, d);        \
    R(rsi, si);       \
    R(rdi, di);       \
    R(r8,  reg8);     \
    R(r9,  reg9);     \
    R(r10, reg10);    \
    R(r11, reg11);    \
    _regs.rcx = c;  _regs.rdx = d; _regs.rsi = si; \
    _regs.rdi = di; _regs.r8 = reg8; _regs.r9 = reg10; _regs.r11 = reg11; \
}

#define TEST(_fn, ...) { \
    addr_t c, d, si, di, reg8, reg9, reg10, reg11; \
    result_t *res = new_result(); \
    res->fn = #_fn;               \
    res->args = #__VA_ARGS__;     \
    REGS(res->before);            \
    _fn(__VA_ARGS__);             \
    REGS(res->after);             \
}

/* Display the symbol names for the given addresses */
static void print_symnames(addr_t addr1, addr_t addr2)
{
    Dl_info ainfo1, ainfo2;
    dladdr((void *)addr1, &ainfo1);
    dladdr((void *)addr2, &ainfo2);
    printf("((%s %s) :: (%s %s))\n",
           ainfo1.dli_sname, ainfo1.dli_fname,
           ainfo2.dli_sname, ainfo2.dli_fname);
}

static void print_regs(const result_t *res, const char *prefix)
{
#define PR_REG(_res, _pre, _r) {        \
    printf("%s%3s %18p :: %18p (%9s) ", \
            _pre, #_r,                  \
           (void *)_res->before._r,     \
            (void *)_res->after._r,     \
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

static void show_results(void)
{
    int i;
    const result_t *rr;

    for (rr=results; rr; rr=rr->next) {
        printf("==> Test %d <==\n", ++i);
        printf("  Func: %s\n", rr->fn);
        printf("  Args: %s\n", rr->args);
        print_regs(rr, "  ");
        putc('\n', stdout);
    }
}

int main(void)
{
    TEST(malloc, (1234));
    TEST(free, NULL);
    show_results();

    return 0;
}
