#ifndef STATIC_ASSERT_H
#define STATIC_ASSERT_H

/* Note we need the 2 concats below because arguments to ##
 * are not expanded, so we need to expand __LINE__ with one indirection
 * before doing the actual concatenation. */
#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
#define static_assert(e) {enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }; }

#endif
