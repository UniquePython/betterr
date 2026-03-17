#include "betterr.h"
#include "result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DEFINE_DOMAIN(TEST_DOMAIN, "test");

DEFINE_RESULT(int, INT)

/* ── Test framework ── */
#define TEST_ASSERT(cond, msg)                             \
    do                                                     \
    {                                                      \
        if (!(cond))                                       \
        {                                                  \
            fprintf(stderr, "FAIL [%s]: %s\n  at %s:%d\n", \
                    __func__, msg, __FILE__, __LINE__);    \
            exit(1);                                       \
        }                                                  \
    } while (0)

#define TEST_PASS() printf("PASS [%s]\n", __func__)

/* ── Test 1: betterr_free nulls the pointer ── */
void test_free_nulls_pointer(void)
{
    Betterr *e = BETTERR(&TEST_DOMAIN, 1, "test");
    betterr_free(&e);
    TEST_ASSERT(e == NULL, "betterr_free should null the pointer");
    TEST_PASS();
}

/* ── Test 2: betterr_free on NULL doesn't crash ── */
void test_free_null_safety(void)
{
    Betterr *e = NULL;
    betterr_free(&e);
    betterr_free(NULL);
    TEST_ASSERT(e == NULL, "pointer should remain NULL");
    TEST_PASS();
}

/* ── Test 3: cause chain is fully freed ── */
void test_chain_fully_freed(void)
{
    Betterr *a = BETTERR(&TEST_DOMAIN, 1, "root");
    Betterr *b = BETTERR_FROM(a, &TEST_DOMAIN, 2, "mid");
    Betterr *c = BETTERR_FROM(b, &TEST_DOMAIN, 3, "outer");

    // if chain walking is broken, this leaks or crashes
    betterr_free(&c);
    TEST_ASSERT(c == NULL, "betterr_free should null the pointer after freeing chain");
    TEST_PASS();
}

/* ── Test 4: propagate node has NULL message and NULL domain ── */
void test_propagate_node(void)
{
    Betterr *root = BETTERR(&TEST_DOMAIN, 1, "root cause");
    Betterr *prop = PROPAGATE(root);

    TEST_ASSERT(prop->message == NULL, "propagate node should have no message");
    TEST_ASSERT(prop->domain == NULL, "propagate node should have no domain");
    TEST_ASSERT(prop->cause == root, "propagate node should own root as cause");

    betterr_free(&prop);
    TEST_PASS();
}

/* ── Test 5: ERR_FROM takes ownership of cause ── */
void test_err_from_ownership(void)
{
    Betterr *cause = BETTERR(&TEST_DOMAIN, 1, "cause");
    Betterr *outer = BETTERR_FROM(cause, &TEST_DOMAIN, 2, "outer");

    TEST_ASSERT(outer->cause == cause, "outer should own cause");
    TEST_ASSERT(outer->cause->code == 1, "cause code should be intact");
    TEST_ASSERT(strcmp(outer->cause->message, "cause") == 0, "cause message should be intact");

    // freeing outer should free cause too — if this crashes, ownership is broken
    betterr_free(&outer);
    TEST_ASSERT(outer == NULL, "outer should be NULL after free");
    TEST_PASS();
}

/* ── Test 6: TRY propagates with location-only node on top ── */
INT failing_fn(void)
{
    return INT_ERR(BETTERR(&TEST_DOMAIN, 1, "original error"));
}

INT propagating_fn(void)
{
    int val = TRY(failing_fn(), INT);
    return INT_OK(val);
}

void test_try_propagation(void)
{
    INT r = propagating_fn();

    TEST_ASSERT(r.error != NULL, "result should be an error");
    TEST_ASSERT(r.error->message == NULL, "top node should have no message");
    TEST_ASSERT(r.error->domain == NULL, "top node should have no domain");
    TEST_ASSERT(r.error->cause != NULL, "top node should have a cause");
    TEST_ASSERT(r.error->cause->code == 1, "cause code should be intact");
    TEST_ASSERT(strcmp(r.error->cause->message, "original error") == 0,
                "cause message should be intact");

    betterr_free(&r.error);
    TEST_PASS();
}

int main(void)
{
    test_free_nulls_pointer();
    test_free_null_safety();
    test_chain_fully_freed();
    test_propagate_node();
    test_err_from_ownership();
    test_try_propagation();

    printf("\nAll tests passed.\n");
    return 0;
}