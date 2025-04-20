#ifndef CASSERT_H_
#define CASSERT_H_

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TERM_OK "  \033[32mOK\033[0m  "
#define TERM_FAIL " \033[31mFAIL\033[0m "
#define TERM_NAME " \033[34mNAME\033[0m "
#define TERM_AMOUNT "\033[34mAMOUNT\033[0m"
#define TERM_ASSERT "\033[34mASSERT\033[0m"

#define OK "  OK  "
#define FAIL " FAIL "
#define NAME " NAME "
#define AMOUNT "AMOUNT"
#define ASSERT "ASSERT"

#define CASSERT_NEQ "!="
#define CASSERT_EQ "=="

#define SIZE_MIN 0
#define UINT8_MIN 0
#define UINT16_MIN 0
#define UINT32_MIN 0
#define UINT64_MIN 0

#define UCHAR_MIN 0
#define USHRT_MIN 0
#define UINT_MIN 0
#define ULONG_MIN 0
#define ULLONG_MIN 0

typedef enum {
  UNKNOWN_EQ,

  // EQ
  ANY_EQ,
  BOOL_EQ,
  STRING_EQ,
  STRING_INT64_EQ,
  STRING_FLOAT_EQ,
  STRING_DOUBLE_EQ,
  CHAR_NUMBER_EQ,
  PTR_EQ,

  CHAR_EQ,
  UCHAR_EQ,

  SHORT_EQ,
  INT_EQ,
  LONG_EQ,
  LONG_LONG_EQ,
  FLOAT_EQ,
  DOUBLE_EQ,

  USHORT_EQ,
  UINT_EQ,
  ULONG_EQ,
  ULONG_LONG_EQ,

  SSIZE_T_EQ,
  SIZE_T_EQ,

  INT8_T_EQ,
  INT16_T_EQ,
  INT32_T_EQ,
  INT64_T_EQ,

  UINT8_T_EQ,
  UINT16_T_EQ,
  UINT32_T_EQ,
  UINT64_T_EQ,

  SIZE_T_MAX_EQ,
  SIZE_T_MIN_EQ,

  INT8_T_MAX_EQ,
  INT16_T_MAX_EQ,
  INT32_T_MAX_EQ,
  INT64_T_MAX_EQ,

  INT8_T_MIN_EQ,
  INT16_T_MIN_EQ,
  INT32_T_MIN_EQ,
  INT64_T_MIN_EQ,

  UINT8_T_MAX_EQ,
  UINT16_T_MAX_EQ,
  UINT32_T_MAX_EQ,
  UINT64_T_MAX_EQ,

  UINT8_T_MIN_EQ,
  UINT16_T_MIN_EQ,
  UINT32_T_MIN_EQ,
  UINT64_T_MIN_EQ,

  CHAR_MAX_EQ,
  UCHAR_MAX_EQ,
  SHORT_MAX_EQ,
  USHORT_MAX_EQ,
  INT_MAX_EQ,
  UINT_MAX_EQ,
  LONG_MAX_EQ,
  ULONG_MAX_EQ,
  LONG_LONG_MAX_EQ,
  ULONG_LONG_MAX_EQ,

  CHAR_MIN_EQ,
  UCHAR_MIN_EQ,
  SHORT_MIN_EQ,
  USHORT_MIN_EQ,
  INT_MIN_EQ,
  UINT_MIN_EQ,
  LONG_MIN_EQ,
  ULONG_MIN_EQ,
  LONG_LONG_MIN_EQ,
  ULONG_LONG_MIN_EQ,

  // NEQ
  ANY_NEQ,
  BOOL_NEQ,
  STRING_NEQ,
  STRING_INT64_NEQ,
  STRING_FLOAT_NEQ,
  STRING_DOUBLE_NEQ,
  CHAR_NUMBER_NEQ,
  PTR_NEQ,

  CHAR_NEQ,
  UCHAR_NEQ,

  SHORT_NEQ,
  INT_NEQ,
  LONG_NEQ,
  LONG_LONG_NEQ,
  FLOAT_NEQ,
  DOUBLE_NEQ,

  USHORT_NEQ,
  UINT_NEQ,
  ULONG_NEQ,
  ULONG_LONG_NEQ,

  SSIZE_T_NEQ,
  SIZE_T_NEQ,

  INT8_T_NEQ,
  INT16_T_NEQ,
  INT32_T_NEQ,
  INT64_T_NEQ,

  UINT8_T_NEQ,
  UINT16_T_NEQ,
  UINT32_T_NEQ,
  UINT64_T_NEQ,

  SIZE_T_MAX_NEQ,
  SIZE_T_MIN_NEQ,

  INT8_T_MAX_NEQ,
  INT16_T_MAX_NEQ,
  INT32_T_MAX_NEQ,
  INT64_T_MAX_NEQ,

  INT8_T_MIN_NEQ,
  INT16_T_MIN_NEQ,
  INT32_T_MIN_NEQ,
  INT64_T_MIN_NEQ,

  UINT8_T_MAX_NEQ,
  UINT16_T_MAX_NEQ,
  UINT32_T_MAX_NEQ,
  UINT64_T_MAX_NEQ,

  UINT8_T_MIN_NEQ,
  UINT16_T_MIN_NEQ,
  UINT32_T_MIN_NEQ,
  UINT64_T_MIN_NEQ,

  CHAR_MAX_NEQ,
  UCHAR_MAX_NEQ,
  SHORT_MAX_NEQ,
  USHORT_MAX_NEQ,
  INT_MAX_NEQ,
  UINT_MAX_NEQ,
  LONG_MAX_NEQ,
  ULONG_MAX_NEQ,
  LONG_LONG_MAX_NEQ,
  ULONG_LONG_MAX_NEQ,

  CHAR_MIN_NEQ,
  UCHAR_MIN_NEQ,
  SHORT_MIN_NEQ,
  USHORT_MIN_NEQ,
  INT_MIN_NEQ,
  UINT_MIN_NEQ,
  LONG_MIN_NEQ,
  ULONG_MIN_NEQ,
  LONG_LONG_MIN_NEQ,
  ULONG_LONG_MIN_NEQ,

  ASSERT_TYPE_COUNT,
} Assert_Type;

typedef struct {
  Assert_Type assert_type;
  bool failed;
  int result;
  void *value1;
  void *value2;
  const char *comparison;
  size_t line;
  const char *file;
  const char *description;
  const char *operation_str;
} Cassert;

typedef struct {
  Cassert *elements;
  size_t count;
  size_t capacity;
  const char *name;
} Test;

typedef struct {
  Test *elements;
  size_t count;
  size_t capacity;
} Tests;

typedef struct {
  char *elements;
  size_t count;
  size_t capacity;
} DA_CHARPTR;

/** @brief The initial capacity of the dynamic arrays. */
#define CASSERT_DAP_CAP 64

/**
 * @brief The macro pushes the new array element at the end of the dynamic
 * array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param element The given new element by value of the same type the array
 * holds elements.
 */
#define cassert_dap(dynamic_array, element)                                    \
  do {                                                                         \
    if ((dynamic_array)->capacity <= (dynamic_array)->count) {                 \
      if ((dynamic_array)->capacity == 0)                                      \
        (dynamic_array)->capacity = CASSERT_DAP_CAP;                           \
      else                                                                     \
        (dynamic_array)->capacity = (dynamic_array)->capacity * 2;             \
                                                                               \
      (dynamic_array)->elements = realloc((dynamic_array)->elements,           \
                                          sizeof(*(dynamic_array)->elements) * \
                                              (dynamic_array)->capacity);      \
                                                                               \
      if ((dynamic_array)->elements == NULL) {                                 \
        fprintf(                                                               \
            stderr,                                                            \
            "The allocation for the dynamic array has failed in: %s: %d\n",    \
            __FILE__, __LINE__);                                               \
        abort();                                                               \
      }                                                                        \
    }                                                                          \
                                                                               \
    (dynamic_array)->elements[(dynamic_array)->count] = (element);             \
    (dynamic_array)->count = (dynamic_array)->count + 1;                       \
  } while (0)

/**
 * @brief The macro pushes the given amount of new elements to the end of the
 * dynamic array.
 *
 * @param dynamic_array The given array by a pointer.
 * @param new_element The given new elements by pointer of the same type the
 * array holds elements.
 * @param new_elements_count The amount of elements to add to the array.
 */
#define cassert_dapc(dynamic_array, new_elements, new_elements_count)          \
  do {                                                                         \
    if (new_elements != NULL) {                                                \
      if ((dynamic_array)->capacity <                                          \
          (dynamic_array)->count + new_elements_count) {                       \
        if ((dynamic_array)->capacity == 0) {                                  \
          (dynamic_array)->capacity = CASSERT_DAP_CAP;                         \
        }                                                                      \
        while ((dynamic_array)->capacity <                                     \
               (dynamic_array)->count + new_elements_count) {                  \
          (dynamic_array)->capacity = (dynamic_array)->capacity * 2;           \
        }                                                                      \
        (dynamic_array)->elements = realloc(                                   \
            (dynamic_array)->elements,                                         \
            sizeof(*(dynamic_array)->elements) * (dynamic_array)->capacity);   \
        if ((dynamic_array)->elements == NULL) {                               \
          fprintf(                                                             \
              stderr,                                                          \
              "The allocation for the dynamic array has failed in: %s: %d\n",  \
              __FILE__, __LINE__);                                             \
          abort();                                                             \
        }                                                                      \
      }                                                                        \
      memcpy((dynamic_array)->elements + (dynamic_array)->count,               \
             (new_elements),                                                   \
             sizeof(*(dynamic_array)->elements) * (new_elements_count));       \
      (dynamic_array)->count = (dynamic_array)->count + (new_elements_count);  \
    }                                                                          \
  } while (0)

#define cassert_tests Tests tests = {0};
static inline Test cassert_init_test(const char *name) {
  return (Test){.name = name};
}
// The free functions are just needed if xou want to clean up every used memory
// and it is also just needed if some cassert_functions are called and you
// provide a custom comparison function to the cassert_type_compare_function
// macro. Here are the Assert_Types that involve heap allocation:
// DOUBLE_EQ: DOUBLE_EQ: FLOAT_EQ: FLOAT_EQ:
// STRING_INT64_EQ: STRING_FLOAT_EQ: STRING_DOUBLE_EQ:
void cassert_free_case_value_mem(Cassert *cassert);
void cassert_array_free_case_value_mem(Test *test);
void cassert_free_test(Test *test);
void cassert_free_tests(Tests *tests);

int cassert_min(int a, int b);
const char *cassert_str_assert_type(Assert_Type assert_type);
bool cassert_set_last_cassert_description(Test *test, const char *description);
int print_operation_and_description(Cassert cassert);

int cassert_print(Cassert cassert, int len_assert_type);
int cassert_print_failure_case(Cassert cassert, int len_assert_type);
int cassert_print_success_case(Cassert cassert, int len_assert_type);
int cassert_print_all_cases(Test *test);
int cassert_print_all_failure_cases(Test *test);
int cassert_print_all_success_cases(Test *test);
void cassert_short_print_test(Test *test);
void cassert_short_print_tests(Tests *tests);
void cassert_print_test(Test *test);
void cassert_print_tests(Tests *tests);

#define cassert_type_compare_function_ex_param(type, a, b, compare_function,   \
                                               cf_ex, param)                   \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.line = __LINE__;                                                   \
    cassert.file = __FILE__;                                                   \
    cassert.assert_type = type;                                                \
    cassert.value1 = malloc(sizeof(typeof((a))));                              \
    assert(cassert.value1 != NULL);                                            \
    cassert.value2 = malloc(sizeof(typeof((b))));                              \
    assert(cassert.value2 != NULL);                                            \
    *(typeof((a)) *)cassert.value1 = (a);                                      \
    *(typeof((b)) *)cassert.value2 = (b);                                      \
    cassert.comparison = #cf_ex;                                               \
    cassert.operation_str = #a " " #cf_ex " " #b;                              \
    cassert.result = compare_function((a), cf_ex, (b), (param));               \
    if (!cassert.result) {                                                     \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_type_compare_function_ex(type, a, b, compare_function, cf_ex)  \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.line = __LINE__;                                                   \
    cassert.file = __FILE__;                                                   \
    cassert.assert_type = type;                                                \
    cassert.value1 = malloc(sizeof(typeof((a))));                              \
    assert(cassert.value1 != NULL);                                            \
    cassert.value2 = malloc(sizeof(typeof((b))));                              \
    assert(cassert.value2 != NULL);                                            \
    *(typeof((a)) *)cassert.value1 = (a);                                      \
    *(typeof((b)) *)cassert.value2 = (b);                                      \
    cassert.comparison = #cf_ex;                                               \
    cassert.operation_str = #a " " #cf_ex " " #b;                              \
    cassert.result = compare_function((a), cf_ex, (b));                        \
    if (!cassert.result) {                                                     \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_type_compare_function(type, a, b, compare_function)            \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.line = __LINE__;                                                   \
    cassert.file = __FILE__;                                                   \
    cassert.assert_type = type;                                                \
    cassert.value1 = malloc(sizeof(typeof((a))));                              \
    assert(cassert.value1 != NULL);                                            \
    cassert.value2 = malloc(sizeof(typeof((b))));                              \
    assert(cassert.value2 != NULL);                                            \
    *(typeof((a)) *)cassert.value1 = (a);                                      \
    *(typeof((b)) *)cassert.value2 = (b);                                      \
    cassert.comparison = #compare_function;                                    \
    cassert.operation_str = #a " " #compare_function " " #b;                   \
    cassert.result = compare_function((a), (b));                               \
    if (!cassert.result) {                                                     \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_type_compare(type, a, compare, b)                              \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.line = __LINE__;                                                   \
    cassert.file = __FILE__;                                                   \
    cassert.assert_type = type;                                                \
    cassert.value1 = (void *)(a);                                              \
    cassert.value2 = (void *)(b);                                              \
    cassert.comparison = #compare;                                             \
    cassert.operation_str = #a " " #compare " " #b;                            \
    cassert.result = (a)compare(b) ? 1 : 0;                                    \
    if (!cassert.result) {                                                     \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_eq(a, compare, b)                                              \
  cassert_type_compare(ANY_EQ, (a), compare, (b))
// NOTE: Wrapping the compare in parentheses breaks some inline expressions.
// Use cassert_eq() instead and negate manual if needed.
#define cassert_neq(a, compare, b)                                             \
  cassert_type_compare(ANY_NEQ, (a), !(compare), (b))

#define cassert_bool_eq(a, b) cassert_type_compare(BOOL_EQ, (a), ==, (b))
#define cassert_bool_neq(a, b) cassert_type_compare(BOOL_NEQ, (a), !=, (b))

#ifndef eps
#define eps 0.01
#endif // !epsilon

const char *term_color(const char *str);
static inline double cassert_fmax(double x, double y) { return x < y ? y : x; }
static inline float cassert_fmaxf(float x, float y) { return x < y ? y : x; }
static inline double cassert_fabs(double x) { return 0 <= x ? x : -x; }
static inline float cassert_fabsf(float x) { return 0 <= x ? x : -x; }

static inline bool float_equals_epsilon(float x, float y, float epsilon) {
  return (cassert_fabsf(x - y)) <=
         (epsilon * cassert_fmaxf(1.0f, cassert_fmaxf(cassert_fabsf(x),
                                                      cassert_fabsf(y))));
}

static inline bool float_equals(float x, float y) {
  return float_equals_epsilon(x, y, eps);
}

static inline bool double_equals_epsilon(double x, double y, float epsilon) {
  return (cassert_fabs(x - y)) <=
         (epsilon *
          cassert_fmax(1.0f, cassert_fmax(cassert_fabs(x), cassert_fabs(y))));
}

static inline bool double_equals(double x, double y) {
  return double_equals_epsilon(x, y, eps);
}

#define cassert_compare_epsilon(x, compare, y, epsilon)                        \
  ((cassert_fabsf(x - y)) compare(                                             \
      epsilon *                                                                \
      cassert_fmaxf(1.0f, cassert_fmaxf(cassert_fabsf(x), cassert_fabsf(y)))))

#define cassert_type_float_compare(type, a, compare, b)                        \
  cassert_type_compare_function_ex_param(type, ((float)(a)), ((float)(b)),     \
                                         cassert_compare_epsilon, compare, 0)

#define cassert_type_float_compare_epsilon(type, a, compare, b, epsilon)       \
  cassert_type_compare_function_ex_param(type, ((float)(a)), ((float)(b)),     \
                                         cassert_compare_epsilon, compare,     \
                                         epsilon)

#define cassert_type_double_compare(type, a, compare, b)                       \
  cassert_type_compare_function_ex_param(type, ((double)(a)), ((double)(b)),   \
                                         cassert_compare_epsilon, compare, 0)

#define cassert_type_double_compare_epsilon(type, a, compare, b, epsilon)      \
  cassert_type_compare_function_ex_param(type, ((double)(a)), ((double)(b)),   \
                                         cassert_compare_epsilon, compare,     \
                                         epsilon)

#define cassert_string_eq(a, b)                                                \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.assert_type = STRING_EQ;                                           \
    cassert.file = __FILE__;                                                   \
    cassert.line = __LINE__;                                                   \
    cassert.value1 = (void *)(a);                                              \
    cassert.value2 = (void *)(b);                                              \
    assert(cassert.value1 != NULL);                                            \
    assert(cassert.value2 != NULL);                                            \
    cassert.comparison = CASSERT_EQ;                                           \
    cassert.operation_str = #a " " CASSERT_EQ " " #b;                          \
    cassert.result = strncmp(a, b, cassert_min(strlen(a), strlen(b)));         \
    if (cassert.result != 0) {                                                 \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_string_neq(a, b)                                               \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.assert_type = STRING_NEQ;                                          \
    cassert.file = __FILE__;                                                   \
    cassert.line = __LINE__;                                                   \
    cassert.value1 = (void *)(a);                                              \
    cassert.value2 = (void *)(b);                                              \
    assert(cassert.value1 != NULL);                                            \
    assert(cassert.value2 != NULL);                                            \
    cassert.comparison = CASSERT_NEQ;                                          \
    cassert.operation_str = #a " " CASSERT_NEQ " " #b;                         \
    cassert.result = !strncmp(a, b, cassert_min(strlen(a), strlen(b)));        \
    if (cassert.result != 0) {                                                 \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_char_number_eq(c, number)                                      \
  cassert_type_compare(CHAR_NUMBER_EQ, c, ==, number);

#define cassert_char_number_neq(c, number)                                     \
  cassert_type_compare(CHAR_NUMBER_NEQ, c, !=, number);

enum { _float, _double, _int64 };
#define cassert_string_int64_eq(string, number)                                \
  cassert_type_string_number_eq(string, number, _int64)
#define cassert_string_float_eq(string, number)                                \
  cassert_type_string_number_eq(string, number, _float)
#define cassert_string_double_eq(string, number)                               \
  cassert_type_string_number_eq(string, number, _double)

#define cassert_string_int64_neq(string, number)                               \
  cassert_type_string_number_neq(string, number, _int64)
#define cassert_string_float_neq(string, number)                               \
  cassert_type_string_number_neq(string, number, _float)
#define cassert_string_double_neq(string, number)                              \
  cassert_type_string_number_neq(string, number, _double)

#define cassert_type_string_number_eq(string, number, type)                    \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.file = __FILE__;                                                   \
    cassert.line = __LINE__;                                                   \
    cassert.value1 = (void *)string;                                           \
    cassert.value2 = malloc(sizeof(typeof(number)));                           \
    assert(cassert.value2 != NULL);                                            \
    char number_string[64] = {0};                                              \
    switch (type) {                                                            \
    case _int64:                                                               \
      *(int64_t *)cassert.value2 = (int64_t)(number);                          \
      cassert.assert_type = STRING_INT64_EQ;                                   \
      if (snprintf(number_string, sizeof(number_string), "%ld\n",              \
                   (int64_t)(number)) < 0) {                                   \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    case _float:                                                               \
      *(float *)cassert.value2 = (float)(number);                              \
      cassert.assert_type = STRING_FLOAT_EQ;                                   \
      if (snprintf(number_string, sizeof(number_string), "%f\n",               \
                   (float)(number)) < 0) {                                     \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    case _double:                                                              \
      *(double *)cassert.value2 = (double)(number);                            \
      cassert.assert_type = STRING_DOUBLE_EQ;                                  \
      if (snprintf(number_string, sizeof(number_string), "%lf\n",              \
                   (double)(number)) < 0) {                                    \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    default:                                                                   \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    cassert.comparison = CASSERT_EQ;                                           \
    cassert.operation_str = #string " " CASSERT_EQ " " #number;                \
    cassert.result =                                                           \
        strncmp(string, number_string,                                         \
                cassert_min(strlen(string), strlen(number_string)));           \
    if (cassert.result != 0) {                                                 \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

#define cassert_type_string_number_neq(string, number, type)                   \
  do {                                                                         \
    Cassert cassert = {0};                                                     \
    cassert.file = __FILE__;                                                   \
    cassert.line = __LINE__;                                                   \
    cassert.value1 = (void *)string;                                           \
    cassert.value2 = malloc(sizeof(typeof(number)));                           \
    assert(cassert.value2 != NULL);                                            \
    char number_string[64] = {0};                                              \
    switch (type) {                                                            \
    case _int64:                                                               \
      *(int64_t *)cassert.value2 = (int64_t)(number);                          \
      cassert.assert_type = STRING_INT64_NEQ;                                  \
      if (snprintf(number_string, sizeof(number_string), "%ld\n",              \
                   (int64_t)(number)) < 0) {                                   \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    case _float:                                                               \
      *(float *)cassert.value2 = (float)(number);                              \
      cassert.assert_type = STRING_FLOAT_NEQ;                                  \
      if (snprintf(number_string, sizeof(number_string), "%f\n",               \
                   (float)(number)) < 0) {                                     \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    case _double:                                                              \
      *(double *)cassert.value2 = (double)(number);                            \
      cassert.assert_type = STRING_DOUBLE_NEQ;                                 \
      if (snprintf(number_string, sizeof(number_string), "%lf\n",              \
                   (double)(number)) < 0) {                                    \
        exit(EXIT_FAILURE);                                                    \
      };                                                                       \
      break;                                                                   \
    default:                                                                   \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    cassert.comparison = CASSERT_NEQ;                                          \
    cassert.operation_str = #string " " CASSERT_NEQ " " #number;               \
    cassert.result =                                                           \
        !strncmp(string, number_string,                                        \
                 cassert_min(strlen(string), strlen(number_string)));          \
    if (cassert.result != 0) {                                                 \
      cassert.failed = true;                                                   \
    }                                                                          \
    cassert_dap(&test, cassert);                                               \
  } while (0)

//--- eq ---

#define cassert_ptr_eq(a, b)                                                   \
  cassert_type_compare(PTR_EQ, (void *)(a), ==, (void *)(b));

#define cassert_size_t_max_eq(a)                                               \
  cassert_type_compare(SIZE_T_MAX_EQ, a, ==, SIZE_MAX);

#define cassert_size_t_min_eq(a)                                               \
  cassert_type_compare(SIZE_T_MIN_EQ, a, ==, SIZE_MIN);

#define cassert_int8_t_max_eq(a)                                               \
  cassert_type_compare(INT8_T_MAX_EQ, a, ==, INT8_MAX);

#define cassert_int8_t_min_eq(a)                                               \
  cassert_type_compare(INT8_T_MIN_EQ, a, ==, INT8_MIN);

#define cassert_uint8_t_max_eq(a)                                              \
  cassert_type_compare(UINT8_T_MAX_EQ, a, ==, UINT8_MAX);

#define cassert_uint8_t_min_eq(a)                                              \
  cassert_type_compare(UINT8_T_MIN_EQ, a, ==, UINT8_MIN);

#define cassert_int16_t_max_eq(a)                                              \
  cassert_type_compare(INT16_T_MAX_EQ, a, ==, INT16_MAX);

#define cassert_int16_t_min_eq(a)                                              \
  cassert_type_compare(INT16_T_MIN_EQ, a, ==, INT16_MIN);

#define cassert_uint16_t_max_eq(a)                                             \
  cassert_type_compare(UINT16_T_MAX_EQ, a, ==, UINT16_MAX);

#define cassert_uint16_t_min_eq(a)                                             \
  cassert_type_compare(UINT16_T_MIN_EQ, a, ==, UINT16_MIN);

#define cassert_int32_t_max_eq(a)                                              \
  cassert_type_compare(INT32_T_MAX_EQ, a, ==, INT32_MAX);

#define cassert_int32_t_min_eq(a)                                              \
  cassert_type_compare(INT32_T_MIN_EQ, a, ==, INT32_MIN);

#define cassert_uint32_t_max_eq(a)                                             \
  cassert_type_compare(UINT32_T_MAX_EQ, a, ==, UINT32_MAX);

#define cassert_uint32_t_min_eq(a)                                             \
  cassert_type_compare(UINT32_T_MIN_EQ, a, ==, UINT32_MIN);

#define cassert_int64_t_max_eq(a)                                              \
  cassert_type_compare(INT64_T_MAX_EQ, a, ==, INT64_MAX);

#define cassert_int64_t_min_eq(a)                                              \
  cassert_type_compare(INT64_T_MIN_EQ, a, ==, INT64_MIN);

#define cassert_uint64_t_max_eq(a)                                             \
  cassert_type_compare(UINT64_T_MAX_EQ, a, ==, UINT64_MAX);

#define cassert_uint64_t_min_eq(a)                                             \
  cassert_type_compare(UINT64_T_MIN_EQ, a, ==, UINT64_MIN);

#define cassert_char_max_eq(a)                                                 \
  cassert_type_compare(CHAR_MAX_EQ, a, ==, CHAR_MAX);

#define cassert_uchar_max_eq(a)                                                \
  cassert_type_compare(UCHAR_MAX_EQ, a, ==, UCHAR_MAX);

#define cassert_short_max_eq(a)                                                \
  cassert_type_compare(SHORT_MAX_EQ, a, ==, SHRT_MAX);

#define cassert_ushort_max_eq(a)                                               \
  cassert_type_compare(USHORT_MAX_EQ, a, ==, USHRT_MAX);

#define cassert_int_max_eq(a) cassert_type_compare(INT_MAX_EQ, a, ==, INT_MAX);

#define cassert_unit_max_eq(a)                                                 \
  cassert_type_compare(UINT_MAX_EQ, a, ==, UINT_MAX);

#define cassert_long_max_eq(a)                                                 \
  cassert_type_compare(LONG_MAX_EQ, a, ==, LONG_MAX);

#define cassert_ulong_max_eq(a)                                                \
  cassert_type_compare(ULONG_MAX_EQ, a, ==, ULONG_MAX);

#define cassert_long_long_max_eq(a)                                            \
  cassert_type_compare(LONG_LONG_MAX_EQ, a, ==, LLONG_MAX);

#define cassert_ulong_long_max_eq(a)                                           \
  cassert_type_compare(ULONG_LONG_MAX_EQ, a, ==, ULLONG_MAX);

#define cassert_char_min_eq(a)                                                 \
  cassert_type_compare(CHAR_MIN_EQ, a, ==, CHAR_MIN);

#define cassert_uchar_min_eq(a)                                                \
  cassert_type_compare(UCHAR_MIN_EQ, a, ==, UCHAR_MIN);

#define cassert_short_min_eq(a)                                                \
  cassert_type_compare(SHORT_MIN_EQ, a, ==, SHRT_MIN);

#define cassert_ushort_min_eq(a)                                               \
  cassert_type_compare(USHORT_MIN_EQ, a, ==, USHRT_MIN);

#define cassert_int_min_eq(a) cassert_type_compare(INT_MIN_EQ, a, ==, INT_MIN);

#define cassert_unit_min_eq(a)                                                 \
  cassert_type_compare(UINT_MIN_EQ, a, ==, UINT_MIN);

#define cassert_long_min_eq(a)                                                 \
  cassert_type_compare(LONG_MIN_EQ, a, ==, LONG_MIN);

#define cassert_ulong_min_eq(a)                                                \
  cassert_type_compare(ULONG_MIN_EQ, a, ==, ULONG_MIN);

#define cassert_long_long_min_eq(a)                                            \
  cassert_type_compare(LONG_LONG_MIN_EQ, a, ==, LLONG_MIN);

#define cassert_ulong_long_min_eq(a)                                           \
  cassert_type_compare(ULONG_LONG_MIN_EQ, a, ==, ULLONG_MIN);

#define cassert_char_eq(a, b) cassert_type_compare(CHAR_EQ, a, ==, b);

#define cassert_uchar_eq(a, b) cassert_type_compare(UCHAR_EQ, a, ==, b);

#define cassert_short_eq(a, b) cassert_type_compare(SHORT_EQ, a, ==, b);

#define cassert_ushort_eq(a, b) cassert_type_compare(USHORT_EQ, a, ==, b);

#define cassert_int_eq(a, b) cassert_type_compare(INT_EQ, a, ==, b);

#define cassert_uint_eq(a, b) cassert_type_compare(UINT_EQ, a, ==, b);

#define cassert_long_eq(a, b) cassert_type_compare(LONG_EQ, a, ==, b);

#define cassert_ulong_eq(a, b) cassert_type_compare(ULONG_EQ, a, ==, b);

#define cassert_long_long_eq(a, b) cassert_type_compare(LONG_LONG_EQ, a, ==, b);

#define cassert_ulong_long_eq(a, b)                                            \
  cassert_type_compare(ULONG_LONG_EQ, a, ==, b);

#define cassert_size_t_eq(a, b) cassert_type_compare(SIZE_T_EQ, a, ==, b);

#define cassert_int8_t_eq(a, b) cassert_type_compare(INT8_T_EQ, a, ==, b);

#define cassert_int16_t_eq(a, b) cassert_type_compare(INT16_T_EQ, a, ==, b);

#define cassert_int32_t_eq(a, b) cassert_type_compare(INT32_T_EQ, a, ==, b);

#define cassert_int64_t_eq(a, b) cassert_type_compare(INT64_T_EQ, a, ==, b);

#define cassert_Uint8_t_eq(a, b) cassert_type_compare(UINT8_T_EQ, a, ==, b);

#define cassert_uint16_t_eq(a, b) cassert_type_compare(UINT16_T_EQ, a, ==, b);

#define cassert_uint32_t_eq(a, b) cassert_type_compare(UINT32_T_EQ, a, ==, b);

#define cassert_uint64_t_eq(a, b) cassert_type_compare(UINT64_T_EQ, a, ==, b);

#define cassert_float_eq(a, b) cassert_type_float_compare(FLOAT_EQ, a, ==, b);

#define cassert_double_eq(a, b)                                                \
  cassert_type_double_compare(DOUBLE_EQ, a, ==, b);

//--- neq ---
#define cassert_ptr_neq(a, b)                                                  \
  cassert_type_compare(PTR_NEQ, (void *)(a), !=, (void *)(b));

#define cassert_size_t_max_neq(a)                                              \
  cassert_type_compare(SIZE_T_MAX_NEQ, a, !=, SIZE_MAX);

#define cassert_size_t_min_neq(a)                                              \
  cassert_type_compare(SIZE_T_MIN_NEQ, a, !=, SIZE_MIN);

#define cassert_int8_t_max_neq(a)                                              \
  cassert_type_compare(INT8_T_MAX_NEQ, a, !=, INT8_MAX);

#define cassert_int8_t_min_neq(a)                                              \
  cassert_type_compare(INT8_T_MIN_NEQ, a, !=, INT8_MIN);

#define cassert_uint8_t_max_neq(a)                                             \
  cassert_type_compare(UINT8_T_MAX_NEQ, a, !=, UINT8_MAX);

#define cassert_uint8_t_min_neq(a)                                             \
  cassert_type_compare(UINT8_T_MIN_NEQ, a, !=, UINT8_MIN);

#define cassert_int16_t_max_neq(a)                                             \
  cassert_type_compare(INT16_T_MAX_NEQ, a, !=, INT16_MAX);

#define cassert_int16_t_min_neq(a)                                             \
  cassert_type_compare(INT16_T_MIN_NEQ, a, !=, INT16_MIN);

#define cassert_uint16_t_max_neq(a)                                            \
  cassert_type_compare(UINT16_T_MAX_NEQ, a, !=, UINT16_MAX);

#define cassert_uint16_t_min_neq(a)                                            \
  cassert_type_compare(UINT16_T_MIN_NEQ, a, !=, UINT16_MIN);

#define cassert_int32_t_max_neq(a)                                             \
  cassert_type_compare(INT32_T_MAX_NEQ, a, !=, INT32_MAX);

#define cassert_int32_t_min_neq(a)                                             \
  cassert_type_compare(INT32_T_MIN_NEQ, a, !=, INT32_MIN);

#define cassert_uint32_t_max_neq(a)                                            \
  cassert_type_compare(UINT32_T_MAX_NEQ, a, !=, UINT32_MAX);

#define cassert_uint32_t_min_neq(a)                                            \
  cassert_type_compare(UINT32_T_MIN_NEQ, a, !=, UINT32_MIN);

#define cassert_int64_t_max_neq(a)                                             \
  cassert_type_compare(INT64_T_MAX_NEQ, a, !=, INT64_MAX);

#define cassert_int64_t_min_neq(a)                                             \
  cassert_type_compare(INT64_T_MIN_NEQ, a, !=, INT64_MIN);

#define cassert_uint64_t_max_neq(a)                                            \
  cassert_type_compare(UINT64_T_MAX_NEQ, a, !=, UINT64_MAX);

#define cassert_uint64_t_min_neq(a)                                            \
  cassert_type_compare(UINT64_T_MIN_NEQ, a, !=, UINT64_MIN);

#define cassert_char_max_neq(a)                                                \
  cassert_type_compare(CHAR_MAX_NEQ, a, !=, CHAR_MAX);

#define cassert_uchar_max_neq(a)                                               \
  cassert_type_compare(UCHAR_MAX_NEQ, a, !=, UCHAR_MAX);

#define cassert_short_max_neq(a)                                               \
  cassert_type_compare(SHORT_MAX_NEQ, a, !=, SHRT_MAX);

#define cassert_ushort_max_neq(a)                                              \
  cassert_type_compare(USHORT_MAX_NEQ, a, !=, USHRT_MAX);

#define cassert_int_max_neq(a)                                                 \
  cassert_type_compare(INT_MAX_NEQ, a, !=, INT_MAX);

#define cassert_unit_max_neq(a)                                                \
  cassert_type_compare(UINT_MAX_NEQ, a, !=, UINT_MAX);

#define cassert_long_max_neq(a)                                                \
  cassert_type_compare(LONG_MAX_NEQ, a, !=, LONG_MAX);

#define cassert_ulong_max_neq(a)                                               \
  cassert_type_compare(ULONG_MAX_NEQ, a, !=, ULONG_MAX);

#define cassert_long_long_max_neq(a)                                           \
  cassert_type_compare(LONG_LONG_MAX_NEQ, a, !=, LLONG_MAX);

#define cassert_ulong_long_max_neq(a)                                          \
  cassert_type_compare(ULONG_LONG_MAX_NEQ, a, !=, ULLONG_MAX);

#define cassert_char_min_neq(a)                                                \
  cassert_type_compare(CHAR_MIN_NEQ, a, !=, CHAR_MIN);

#define cassert_uchar_min_neq(a)                                               \
  cassert_type_compare(UCHAR_MIN_NEQ, a, !=, UCHAR_MIN);

#define cassert_short_min_neq(a)                                               \
  cassert_type_compare(SHORT_MIN_NEQ, a, !=, SHRT_MIN);

#define cassert_ushort_min_neq(a)                                              \
  cassert_type_compare(USHORT_MIN_NEQ, a, !=, USHRT_MIN);

#define cassert_int_min_neq(a)                                                 \
  cassert_type_compare(INT_MIN_NEQ, a, !=, INT_MIN);

#define cassert_unit_min_neq(a)                                                \
  cassert_type_compare(UINT_MIN_NEQ, a, !=, UINT_MIN);

#define cassert_long_min_neq(a)                                                \
  cassert_type_compare(LONG_MIN_NEQ, a, !=, LONG_MIN);

#define cassert_ulong_min_neq(a)                                               \
  cassert_type_compare(ULONG_MIN_NEQ, a, !=, ULONG_MIN);

#define cassert_long_long_min_neq(a)                                           \
  cassert_type_compare(LONG_LONG_MIN_NEQ, a, !=, LLONG_MIN);

#define cassert_ulong_long_min_neq(a)                                          \
  cassert_type_compare(ULONG_LONG_MIN_NEQ, a, !=, ULLONG_MIN);

#define cassert_char_neq(a, b) cassert_type_compare(CHAR_NEQ, a, !=, b);

#define cassert_uchar_neq(a, b) cassert_type_compare(UCHAR_NEQ, a, !=, b);

#define cassert_short_neq(a, b) cassert_type_compare(SHORT_NEQ, a, !=, b);

#define cassert_ushort_neq(a, b) cassert_type_compare(USHORT_NEQ, a, !=, b);

#define cassert_int_neq(a, b) cassert_type_compare(INT_NEQ, a, !=, b);

#define cassert_uint_neq(a, b) cassert_type_compare(UINT_NEQ, a, !=, b);

#define cassert_long_neq(a, b) cassert_type_compare(LONG_NEQ, a, !=, b);

#define cassert_ulong_neq(a, b) cassert_type_compare(ULONG_NEQ, a, !=, b);

#define cassert_long_long_neq(a, b)                                            \
  cassert_type_compare(LONG_LONG_NEQ, a, !=, b);

#define cassert_ulong_long_neq(a, b)                                           \
  cassert_type_compare(ULONG_LONG_NEQ, a, !=, b);

#define cassert_size_t_neq(a, b) cassert_type_compare(SIZE_T_NEQ, a, !=, b);

#define cassert_int8_t_neq(a, b) cassert_type_compare(INT8_T_NEQ, a, !=, b);

#define cassert_int16_t_neq(a, b) cassert_type_compare(INT16_T_NEQ, a, !=, b);

#define cassert_int32_t_neq(a, b) cassert_type_compare(INT32_T_NEQ, a, !=, b);

#define cassert_int64_t_neq(a, b) cassert_type_compare(INT64_T_NEQ, a, !=, b);

#define cassert_Uint8_t_neq(a, b) cassert_type_compare(UINT8_T_NEQ, a, !=, b);

#define cassert_uint16_t_neq(a, b) cassert_type_compare(UINT16_T_NEQ, a, !=, b);

#define cassert_uint32_t_neq(a, b) cassert_type_compare(UINT32_T_NEQ, a, !=, b);

#define cassert_uint64_t_neq(a, b) cassert_type_compare(UINT64_T_NEQ, a, !=, b);

#define cassert_float_neq(a, b) cassert_type_float_compare(FLOAT_NEQ, a, !=, b);

#define cassert_double_neq(a, b)                                               \
  cassert_type_double_compare(DOUBLE_NEQ, a, !=, b);

// ---------------------------------------------------------------------------
// --- epsilon comparison ---
#define cassert_float_eq_epsilon(a, b)                                         \
  cassert_type_float_compare_epsilon(FLOAT_EQ, a, <=, b, eps);

#define cassert_double_eq_epsilon(a, b)                                        \
  cassert_type_double_compare_epsilon(DOUBLE_EQ, a, <=, b, eps);

// --- neq epsilon comparison ---
#define cassert_float_neq_epsilon(a, b)                                        \
  cassert_type_float_compare_epsilon(FLOAT_NEQ, a, >, b, eps);

#define cassert_double_neq_epsilon(a, b)                                       \
  cassert_type_double_compare_epsilon(DOUBLE_NEQ, a, >, b, eps);

#endif // CASSERT_H_

// ===========================================================================

#ifdef CASSERT_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

int cassert_min(int a, int b) { return a < b ? a : b; }

const char *cassert_str_assert_type(Assert_Type assert_type) {
  static_assert(ASSERT_TYPE_COUNT == 137, "assert types count has changed");
  switch (assert_type) {
  case ANY_EQ:
    return "ANY_EQ";
  case BOOL_EQ:
    return "BOOL_EQ";
  case STRING_EQ:
    return "STRING_EQ";
  case STRING_INT64_EQ:
    return "STRING_INT64_EQ";
  case STRING_FLOAT_EQ:
    return "STRING_FLOAT_EQ";
  case STRING_DOUBLE_EQ:
    return "STRING_DOUBLE_EQ";
  case CHAR_NUMBER_EQ:
    return "CHAR_NUMBER_EQ";
  case PTR_EQ:
    return "PTR_EQ";
  case CHAR_EQ:
    return "CHAR_EQ";
  case UCHAR_EQ:
    return "UCHAR_EQ";
  case SHORT_EQ:
    return "SHORT_EQ";
  case INT_EQ:
    return "INT_EQ";
  case LONG_EQ:
    return "LONG_EQ";
  case LONG_LONG_EQ:
    return "LONG_LONG_EQ";
  case USHORT_EQ:
    return "USHORT_EQ";
  case UINT_EQ:
    return "UINT_EQ";
  case ULONG_EQ:
    return "ULONG_EQ";
  case ULONG_LONG_EQ:
    return "ULONG_LONG_EQ";
  case SSIZE_T_EQ:
    return "SSIZE_T_EQ";
  case SIZE_T_EQ:
    return "SIZE_T_EQ";
  case INT8_T_EQ:
    return "INT8_T_EQ";
  case INT16_T_EQ:
    return "INT16_T_EQ";
  case INT32_T_EQ:
    return "INT32_T_EQ";
  case INT64_T_EQ:
    return "INT64_T_EQ";
  case UINT8_T_EQ:
    return "UINT8_T_EQ";
  case UINT16_T_EQ:
    return "UINT16_T_EQ";
  case UINT32_T_EQ:
    return "UINT32_T_EQ";
  case UINT64_T_EQ:
    return "UINT64_T_EQ";
  case FLOAT_EQ:
    return "FLOAT_EQ";
  case DOUBLE_EQ:
    return "DOUBLE_EQ";
  case SIZE_T_MAX_EQ:
    return "SIZE_T_MAX_EQ";
  case SIZE_T_MIN_EQ:
    return "SIZE_T_MIN_EQ";
  case INT8_T_MAX_EQ:
    return "INT8_T_MAX_EQ";
  case INT16_T_MAX_EQ:
    return "INT16_T_MAX_EQ";
  case INT32_T_MAX_EQ:
    return "INT32_T_MAX_EQ";
  case INT64_T_MAX_EQ:
    return "INT64_T_MAX_EQ";
  case INT8_T_MIN_EQ:
    return "INT8_T_MIN_EQ";
  case INT16_T_MIN_EQ:
    return "INT16_T_MIN_EQ";
  case INT32_T_MIN_EQ:
    return "INT32_T_MIN_EQ";
  case INT64_T_MIN_EQ:
    return "INT64_T_MIN_EQ";
  case UINT8_T_MAX_EQ:
    return "UINT8_T_MAX_EQ";
  case UINT16_T_MAX_EQ:
    return "UINT16_T_MAX_EQ";
  case UINT32_T_MAX_EQ:
    return "UINT32_T_MAX_EQ";
  case UINT64_T_MAX_EQ:
    return "UINT64_T_MAX_EQ";
  case UINT8_T_MIN_EQ:
    return "UINT8_T_MIN_EQ";
  case UINT16_T_MIN_EQ:
    return "UINT16_T_MIN_EQ";
  case UINT32_T_MIN_EQ:
    return "UINT32_T_MIN_EQ";
  case UINT64_T_MIN_EQ:
    return "UINT64_T_MIN_EQ";
  case CHAR_MAX_EQ:
    return "CHAR_MAX_EQ";
  case UCHAR_MAX_EQ:
    return "UCHAR_MAX_EQ";
  case SHORT_MAX_EQ:
    return "SHORT_MAX_EQ";
  case USHORT_MAX_EQ:
    return "USHORT_MAX_EQ";
  case INT_MAX_EQ:
    return "INT_MAX_EQ";
  case UINT_MAX_EQ:
    return "UINT_MAX_EQ";
  case LONG_MAX_EQ:
    return "LONG_MAX_EQ";
  case ULONG_MAX_EQ:
    return "ULONG_MAX_EQ";
  case LONG_LONG_MAX_EQ:
    return "LONG_LONG_MAX_EQ";
  case ULONG_LONG_MAX_EQ:
    return "ULONG_LONG_MAX_EQ";
  case CHAR_MIN_EQ:
    return "CHAR_MIN_EQ";
  case UCHAR_MIN_EQ:
    return "UCHAR_MIN_EQ";
  case SHORT_MIN_EQ:
    return "SHORT_MIN_EQ";
  case USHORT_MIN_EQ:
    return "USHORT_MIN_EQ";
  case INT_MIN_EQ:
    return "INT_MIN_EQ";
  case UINT_MIN_EQ:
    return "UINT_MIN_EQ";
  case LONG_MIN_EQ:
    return "LONG_MIN_EQ";
  case ULONG_MIN_EQ:
    return "ULONG_MIN_EQ";
  case LONG_LONG_MIN_EQ:
    return "LONG_LONG_MIN_EQ";
  case ULONG_LONG_MIN_EQ:
    return "ULONG_LONG_MIN_EQ";

  case ANY_NEQ:
    return "ANY_NEQ";
  case BOOL_NEQ:
    return "BOOL_NEQ";
  case STRING_NEQ:
    return "STRING_NEQ";
  case STRING_INT64_NEQ:
    return "STRING_INT64_NEQ";
  case STRING_FLOAT_NEQ:
    return "STRING_FLOAT_NEQ";
  case STRING_DOUBLE_NEQ:
    return "STRING_DOUBLE_NEQ";
  case CHAR_NUMBER_NEQ:
    return "CHAR_NUMBER_NEQ";
  case PTR_NEQ:
    return "PTR_NEQ";
  case CHAR_NEQ:
    return "CHAR_NEQ";
  case UCHAR_NEQ:
    return "UCHAR_NEQ";
  case SHORT_NEQ:
    return "SHORT_NEQ";
  case INT_NEQ:
    return "INT_NEQ";
  case LONG_NEQ:
    return "LONG_NEQ";
  case LONG_LONG_NEQ:
    return "LONG_LONG_NEQ";
  case USHORT_NEQ:
    return "USHORT_NEQ";
  case UINT_NEQ:
    return "UINT_NEQ";
  case ULONG_NEQ:
    return "ULONG_NEQ";
  case ULONG_LONG_NEQ:
    return "ULONG_LONG_NEQ";
  case SSIZE_T_NEQ:
    return "SSIZE_T_NEQ";
  case SIZE_T_NEQ:
    return "SIZE_T_NEQ";
  case INT8_T_NEQ:
    return "INT8_T_NEQ";
  case INT16_T_NEQ:
    return "INT16_T_NEQ";
  case INT32_T_NEQ:
    return "INT32_T_NEQ";
  case INT64_T_NEQ:
    return "INT64_T_NEQ";
  case UINT8_T_NEQ:
    return "UINT8_T_NEQ";
  case UINT16_T_NEQ:
    return "UINT16_T_NEQ";
  case UINT32_T_NEQ:
    return "UINT32_T_NEQ";
  case UINT64_T_NEQ:
    return "UINT64_T_NEQ";
  case FLOAT_NEQ:
    return "FLOAT_NEQ";
  case DOUBLE_NEQ:
    return "DOUBLE_NEQ";
  case SIZE_T_MAX_NEQ:
    return "SIZE_T_MAX_NEQ";
  case SIZE_T_MIN_NEQ:
    return "SIZE_T_MIN_NEQ";
  case INT8_T_MAX_NEQ:
    return "INT8_T_MAX_NEQ";
  case INT16_T_MAX_NEQ:
    return "INT16_T_MAX_NEQ";
  case INT32_T_MAX_NEQ:
    return "INT32_T_MAX_NEQ";
  case INT64_T_MAX_NEQ:
    return "INT64_T_MAX_NEQ";
  case INT8_T_MIN_NEQ:
    return "INT8_T_MIN_NEQ";
  case INT16_T_MIN_NEQ:
    return "INT16_T_MIN_NEQ";
  case INT32_T_MIN_NEQ:
    return "INT32_T_MIN_NEQ";
  case INT64_T_MIN_NEQ:
    return "INT64_T_MIN_NEQ";
  case UINT8_T_MAX_NEQ:
    return "UINT8_T_MAX_NEQ";
  case UINT16_T_MAX_NEQ:
    return "UINT16_T_MAX_NEQ";
  case UINT32_T_MAX_NEQ:
    return "UINT32_T_MAX_NEQ";
  case UINT64_T_MAX_NEQ:
    return "UINT64_T_MAX_NEQ";
  case UINT8_T_MIN_NEQ:
    return "UINT8_T_MIN_NEQ";
  case UINT16_T_MIN_NEQ:
    return "UINT16_T_MIN_NEQ";
  case UINT32_T_MIN_NEQ:
    return "UINT32_T_MIN_NEQ";
  case UINT64_T_MIN_NEQ:
    return "UINT64_T_MIN_NEQ";
  case CHAR_MAX_NEQ:
    return "CHAR_MAX_NEQ";
  case UCHAR_MAX_NEQ:
    return "UCHAR_MAX_NEQ";
  case SHORT_MAX_NEQ:
    return "SHORT_MAX_NEQ";
  case USHORT_MAX_NEQ:
    return "USHORT_MAX_NEQ";
  case INT_MAX_NEQ:
    return "INT_MAX_NEQ";
  case UINT_MAX_NEQ:
    return "UINT_MAX_NEQ";
  case LONG_MAX_NEQ:
    return "LONG_MAX_NEQ";
  case ULONG_MAX_NEQ:
    return "ULONG_MAX_NEQ";
  case LONG_LONG_MAX_NEQ:
    return "LONG_LONG_MAX_NEQ";
  case ULONG_LONG_MAX_NEQ:
    return "ULONG_LONG_MAX_NEQ";
  case CHAR_MIN_NEQ:
    return "CHAR_MIN_NEQ";
  case UCHAR_MIN_NEQ:
    return "UCHAR_MIN_NEQ";
  case SHORT_MIN_NEQ:
    return "SHORT_MIN_NEQ";
  case USHORT_MIN_NEQ:
    return "USHORT_MIN_NEQ";
  case INT_MIN_NEQ:
    return "INT_MIN_NEQ";
  case UINT_MIN_NEQ:
    return "UINT_MIN_NEQ";
  case LONG_MIN_NEQ:
    return "LONG_MIN_NEQ";
  case ULONG_MIN_NEQ:
    return "ULONG_MIN_NEQ";
  case LONG_LONG_MIN_NEQ:
    return "LONG_LONG_MIN_NEQ";
  case ULONG_LONG_MIN_NEQ:
    return "ULONG_LONG_MIN_NEQ";

  case ASSERT_TYPE_COUNT:
  case UNKNOWN_EQ:
  default:
    return "";
  }
}

int cassert_fprintf(FILE *stream, const char *status, const char *fmt, ...) {
  DA_CHARPTR c = {0};
  if (status != NULL) {
    cassert_dap(&c, '[');
    cassert_dapc(&c, status, strlen(status));
    cassert_dap(&c, ']');
    cassert_dap(&c, ' ');
  }
  cassert_dapc(&c, fmt, strlen(fmt));
  cassert_dap(&c, '\n');
  cassert_dap(&c, 0);

  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(stream, c.elements, args);
  va_end(args);
  free(c.elements);
  return ret;
}

const char *term_color(const char *str) {
  if (isatty(STDERR_FILENO)) {
    if (strncmp(ASSERT, str, strlen(ASSERT)) == 0) {
      return TERM_ASSERT;
    } else if (strncmp(AMOUNT, str, strlen(AMOUNT)) == 0) {
      return TERM_AMOUNT;
    } else if (strncmp(NAME, str, strlen(NAME)) == 0) {
      return TERM_NAME;
    } else if (strncmp(FAIL, str, strlen(FAIL)) == 0) {
      return TERM_FAIL;
    } else if (strncmp(OK, str, strlen(OK)) == 0) {
      return TERM_OK;
    }
  } else {
    if (strncmp(ASSERT, str, strlen(ASSERT)) == 0) {
      return ASSERT;
    } else if (strncmp(AMOUNT, str, strlen(AMOUNT)) == 0) {
      return AMOUNT;
    } else if (strncmp(NAME, str, strlen(NAME)) == 0) {
      return NAME;
    } else if (strncmp(FAIL, str, strlen(FAIL)) == 0) {
      return FAIL;
    } else if (strncmp(OK, str, strlen(OK)) == 0) {
      return OK;
    }
  }
  assert(0 && "UNREACHABLE term_color()");
}

const char *booltostr(bool value) {
  return value ? term_color(OK) : term_color(FAIL);
}

int print_operation_and_description(Cassert cassert) {
  size_t space = 1;
  size_t nullterminator = 1;
  size_t dash = 1;
  size_t len = 0;
  if (cassert.description) {
    len = strlen(cassert.description);
  }
  size_t buffer_size = len + 2 * space + 2 * dash + nullterminator;
  size_t byte_size = sizeof(char) * buffer_size;
  char *desc_buf = malloc(byte_size);
  if (desc_buf == NULL) {
    assert(0 && "OUT OF MEMORY");
  }
  memset(desc_buf, 0, byte_size);
  if (len > 0) {
    snprintf(desc_buf, buffer_size, " -- %s\n", cassert.description);
  }

  len = 0;
  if (cassert.operation_str) {
    len = strlen(cassert.operation_str);
  }
  buffer_size = len + nullterminator;
  byte_size = sizeof(char) * buffer_size;
  char *operation_buf = malloc(byte_size);
  if (operation_buf == NULL) {
    assert(0 && "OUT OF MEMORY");
  }
  memset(operation_buf, 0, byte_size);
  if (len > 0) {
    snprintf(operation_buf, buffer_size, "%s\n", cassert.operation_str);
  }

  int ret = -1;
  if (cassert.description || cassert.operation_str) {
    ret = cassert_fprintf(stderr, term_color(ASSERT), "%s%s", operation_buf,
                          desc_buf);
  }
  free(operation_buf);
  free(desc_buf);
  return ret;
}

#define LOC_FMT " %s:%d"
#define LOC_ARG(arg) (arg).file, (arg).line
#define TERM_ASSERT_TYPE_FMT "|\033[34m%-*s\033[0m| "
#define ASSERT_TYPE_FMT "|%-*s| "

#define cassert_print_fmtf(fmt_value1, fmt_value2, value1, value2)             \
  do {                                                                         \
    if (isatty(STDERR_FILENO)) {                                               \
      return cassert_fprintf(                                                  \
          stream, booltostr(!cassert.failed),                                  \
          TERM_ASSERT_TYPE_FMT fmt_value1 " %s " fmt_value2 " ->" LOC_FMT, n,  \
          cassert_str_assert_type(cassert.assert_type), (value1),              \
          cassert.comparison, (value2), LOC_ARG(cassert));                     \
    } else {                                                                   \
      return cassert_fprintf(                                                  \
          stream, booltostr(!cassert.failed),                                  \
          ASSERT_TYPE_FMT fmt_value1 " %s " fmt_value2 " ->" LOC_FMT, n,       \
          cassert_str_assert_type(cassert.assert_type), (value1),              \
          cassert.comparison, (value2), LOC_ARG(cassert));                     \
    }                                                                          \
  } while (0)

int cassert_print_internal(FILE *stream, Cassert cassert, int len_assert_type) {
  // NOTE: n is used in the macro cassert_print_fmtf() that indirectly calls to
  // n without accepting it as an argument.
  int n = len_assert_type;

#ifdef PRINT_OPERATION_AND_DESCRIPTION
  print_operation_and_description(cassert);
#endif // PRINT_DESCRIPTION
  switch (cassert.assert_type) {
  case ANY_EQ:
  case ANY_NEQ:
    cassert_print_fmtf("%p", "%p", cassert.value1, cassert.value2);
  case BOOL_EQ:
  case BOOL_NEQ:
    cassert_print_fmtf("%s", "%s", cassert.value1 ? "TRUE" : "FALSE",
                       cassert.value2 ? "TRUE" : "FALSE");
  case STRING_EQ:
  case STRING_NEQ:
    cassert_print_fmtf("%s", "%s", (char *)cassert.value1,
                       (char *)cassert.value2);
  case FLOAT_EQ:
  case FLOAT_NEQ:
    cassert_print_fmtf("%f", "%f", *(float *)cassert.value1,
                       *(float *)cassert.value2);
  case DOUBLE_EQ:
  case DOUBLE_NEQ:
    cassert_print_fmtf("%lf", "%lf", *(double *)cassert.value1,
                       *(double *)cassert.value2);
  case STRING_INT64_EQ:
  case STRING_INT64_NEQ:
    cassert_print_fmtf("%s", "%ld", (char *)cassert.value1,
                       *(int64_t *)cassert.value2);
  case STRING_FLOAT_EQ:
  case STRING_FLOAT_NEQ:
    cassert_print_fmtf("%s", "%f", (char *)cassert.value1,
                       *(float *)cassert.value2);
  case STRING_DOUBLE_EQ:
  case STRING_DOUBLE_NEQ:
    cassert_print_fmtf("%s", "%lf", (char *)cassert.value1,
                       *(double *)cassert.value2);
  case PTR_EQ:
  case PTR_NEQ:
    cassert_print_fmtf("%p", "%p", (uintptr_t *)cassert.value1,
                       (uintptr_t *)cassert.value2);
  case CHAR_NUMBER_EQ:
  case CHAR_NUMBER_NEQ:
    cassert_print_fmtf("%c", "%ld", (char *)cassert.value1,
                       (int64_t *)cassert.value2);

  case CHAR_MAX_EQ:
  case CHAR_MIN_EQ:
  case CHAR_MAX_NEQ:
  case CHAR_MIN_NEQ:
  case CHAR_EQ:
  case CHAR_NEQ:
    cassert_print_fmtf("%c", "%c", (char *)cassert.value1,
                       (char *)cassert.value2);
  case SHORT_MAX_EQ:
  case SHORT_MIN_EQ:
  case SHORT_MAX_NEQ:
  case SHORT_MIN_NEQ:
  case SHORT_EQ:
  case SHORT_NEQ:
    cassert_print_fmtf("%hd", "%hd", (short *)cassert.value1,
                       (short *)cassert.value2);
  case INT_MAX_EQ:
  case INT_MIN_EQ:
  case INT_MAX_NEQ:
  case INT_MIN_NEQ:
  case INT_EQ:
  case INT_NEQ:
    cassert_print_fmtf("%d", "%d", (int *)cassert.value1,
                       (int *)cassert.value2);
  case LONG_MAX_EQ:
  case LONG_MIN_EQ:
  case LONG_MAX_NEQ:
  case LONG_MIN_NEQ:
  case LONG_EQ:
  case LONG_NEQ:
    cassert_print_fmtf("%ld", "%ld", (long int *)cassert.value1,
                       (long int *)cassert.value2);
  case LONG_LONG_MAX_EQ:
  case LONG_LONG_MIN_EQ:
  case LONG_LONG_MAX_NEQ:
  case LONG_LONG_MIN_NEQ:
  case LONG_LONG_EQ:
  case LONG_LONG_NEQ:
    cassert_print_fmtf("%lld", "%lld", (long long int *)cassert.value1,
                       (long long int *)cassert.value2);

  case UCHAR_MAX_EQ:
  case UCHAR_MIN_EQ:
  case UCHAR_MAX_NEQ:
  case UCHAR_MIN_NEQ:
  case UCHAR_EQ:
  case UCHAR_NEQ:
    cassert_print_fmtf("%c", "%c", (char *)cassert.value1,
                       (char *)cassert.value2);
  case USHORT_MAX_EQ:
  case USHORT_MIN_EQ:
  case USHORT_MAX_NEQ:
  case USHORT_MIN_NEQ:
  case USHORT_EQ:
  case USHORT_NEQ:
    cassert_print_fmtf("%hu", "%hu", (short *)cassert.value1,
                       (short *)cassert.value2);
  case UINT_MAX_EQ:
  case UINT_MIN_EQ:
  case UINT_MAX_NEQ:
  case UINT_MIN_NEQ:
  case UINT_EQ:
  case UINT_NEQ:
    cassert_print_fmtf("%u", "%u", (int *)cassert.value1,
                       (int *)cassert.value2);
  case ULONG_MAX_EQ:
  case ULONG_MIN_EQ:
  case ULONG_MAX_NEQ:
  case ULONG_MIN_NEQ:
  case ULONG_EQ:
  case ULONG_NEQ:
    cassert_print_fmtf("%lu", "%lu", (long int *)cassert.value1,
                       (long int *)cassert.value2);
  case ULONG_LONG_MAX_EQ:
  case ULONG_LONG_MIN_EQ:
  case ULONG_LONG_MAX_NEQ:
  case ULONG_LONG_MIN_NEQ:
  case ULONG_LONG_EQ:
  case ULONG_LONG_NEQ:
    cassert_print_fmtf("%llu", "%llu", (long long int *)cassert.value1,
                       (long long int *)cassert.value2);

  case SSIZE_T_EQ:
  case SSIZE_T_NEQ:
    cassert_print_fmtf("%zd", "%zd", (ssize_t *)cassert.value1,
                       (ssize_t *)cassert.value2);

  case SIZE_T_MAX_EQ:
  case SIZE_T_MIN_EQ:
  case SIZE_T_MAX_NEQ:
  case SIZE_T_MIN_NEQ:
  case SIZE_T_EQ:
  case SIZE_T_NEQ:
    cassert_print_fmtf("%zu", "%zu", (size_t *)cassert.value1,
                       (size_t *)cassert.value2);

  case INT8_T_MAX_EQ:
  case INT8_T_MIN_EQ:
  case INT8_T_MAX_NEQ:
  case INT8_T_MIN_NEQ:
  case INT8_T_EQ:
  case INT8_T_NEQ:
    cassert_print_fmtf("%hhd", "%hhd", (int8_t *)cassert.value1,
                       (int8_t *)cassert.value2);
  case INT16_T_MAX_EQ:
  case INT16_T_MIN_EQ:
  case INT16_T_MAX_NEQ:
  case INT16_T_MIN_NEQ:
  case INT16_T_EQ:
  case INT16_T_NEQ:
    cassert_print_fmtf("%hd", "%hd", (int16_t *)cassert.value1,
                       (int16_t *)cassert.value2);
  case INT32_T_MAX_EQ:
  case INT32_T_MIN_EQ:
  case INT32_T_MAX_NEQ:
  case INT32_T_MIN_NEQ:
  case INT32_T_EQ:
  case INT32_T_NEQ:
    cassert_print_fmtf("%d", "%d", (int32_t *)cassert.value1,
                       (int32_t *)cassert.value2);
  case INT64_T_MAX_EQ:
  case INT64_T_MIN_EQ:
  case INT64_T_MAX_NEQ:
  case INT64_T_MIN_NEQ:
  case INT64_T_EQ:
  case INT64_T_NEQ:
    cassert_print_fmtf("%ld", "%ld", (int64_t *)cassert.value1,
                       (int64_t *)cassert.value2);

  case UINT8_T_MAX_EQ:
  case UINT8_T_MIN_EQ:
  case UINT8_T_MAX_NEQ:
  case UINT8_T_MIN_NEQ:
  case UINT8_T_EQ:
  case UINT8_T_NEQ:
    cassert_print_fmtf("%hhu", "%hhu", (uint8_t *)cassert.value1,
                       (uint8_t *)cassert.value2);
  case UINT16_T_MAX_EQ:
  case UINT16_T_MIN_EQ:
  case UINT16_T_MAX_NEQ:
  case UINT16_T_MIN_NEQ:
  case UINT16_T_EQ:
  case UINT16_T_NEQ:
    cassert_print_fmtf("%hu", "%hu", (uint16_t *)cassert.value1,
                       (uint16_t *)cassert.value2);
  case UINT32_T_MAX_EQ:
  case UINT32_T_MIN_EQ:
  case UINT32_T_MAX_NEQ:
  case UINT32_T_MIN_NEQ:
  case UINT32_T_EQ:
  case UINT32_T_NEQ:
    cassert_print_fmtf("%u", "%u", (uint32_t *)cassert.value1,
                       (uint32_t *)cassert.value2);
  case UINT64_T_MAX_EQ:
  case UINT64_T_MIN_EQ:
  case UINT64_T_MAX_NEQ:
  case UINT64_T_MIN_NEQ:
  case UINT64_T_EQ:
  case UINT64_T_NEQ:
    cassert_print_fmtf("%lu", "%lu", (uint64_t *)cassert.value1,
                       (uint64_t *)cassert.value2);

  case UNKNOWN_EQ:
  default:
    return cassert_fprintf(stream, booltostr(!cassert.failed), "%s ->" LOC_FMT,
                           cassert_str_assert_type(cassert.assert_type),
                           LOC_ARG(cassert));
  }
}

int cassert_print(Cassert cassert, int len_assert_type) {
  return cassert_print_internal(stderr, cassert, len_assert_type);
}

int cassert_print_failure_case(Cassert cassert, int len_assert_type) {
  if (cassert.failed) {
    return cassert_print(cassert, len_assert_type);
  }
  return 0;
}

int cassert_print_success_case(Cassert cassert, int len_assert_type) {
  if (!cassert.failed) {
    return cassert_print(cassert, len_assert_type);
  }
  return 0;
}

int cassert_print_all_cases(Test *test) {
  int max_len = 0;
  for (size_t i = 0; i < test->count; ++i) {
    max_len = cassert_fmax(max_len, strlen(cassert_str_assert_type(
                                        test->elements[i].assert_type)));
  }

  int result = 0;
  for (size_t c = 0; c < test->count; ++c) {
    if (cassert_print(test->elements[c], max_len) < 0) {
      result = -1;
    }
  }
  return result;
}

int cassert_print_all_failure_cases(Test *test) {
  int max_len = 0;
  for (size_t i = 0; i < test->count; ++i) {
    max_len = cassert_fmax(max_len, strlen(cassert_str_assert_type(
                                        test->elements[i].assert_type)));
  }
  int result = 0;
  for (size_t c = 0; c < test->count; ++c) {
    if (cassert_print_failure_case(test->elements[c], max_len) < 0) {
      result = -1;
    }
  }
  return result;
}

int cassert_print_all_success_cases(Test *test) {
  int max_len = 0;
  for (size_t i = 0; i < test->count; ++i) {
    max_len = cassert_fmax(max_len, strlen(cassert_str_assert_type(
                                        test->elements[i].assert_type)));
  }
  int result = 0;
  for (size_t c = 0; c < test->count; ++c) {
    if (cassert_print_success_case(test->elements[c], max_len) < 0) {
      result = -1;
    }
  }
  return result;
}

void cassert_print_test(Test *test) {
  fprintf(stderr, "--------------------------------------------------------\n");
  if (isatty(STDERR_FILENO)) {
    cassert_fprintf(stderr, TERM_NAME, "%s", test->name);
    cassert_fprintf(stderr, TERM_AMOUNT, "%zu", test->count);
  } else {
    cassert_fprintf(stderr, NAME, "%s", test->name);
    cassert_fprintf(stderr, AMOUNT, "%zu", test->count);
  }
  cassert_print_all_cases(test);
}

void cassert_short_print_test(Test *test) {
  int failed = 0;
  for (size_t i = 0; i < test->count; ++i) {
    if (test->elements[i].failed) {
      failed++;
    }
  }
  if (failed) {
    if (isatty(STDERR_FILENO)) {
      cassert_fprintf(stderr, TERM_FAIL, "[" TERM_AMOUNT " %d:%zu] %s", failed,
                      test->count, test->name);
    } else {
      cassert_fprintf(stderr, FAIL, "[" AMOUNT " %d:%zu] %s", failed,
                      test->count, test->name);
    }
    return;
  }
  if (isatty(STDERR_FILENO)) {
    cassert_fprintf(stderr, TERM_OK, "[" TERM_AMOUNT " %zu:%zu] %s",
                    test->count, test->count, test->name);
    return;
  } else {
    cassert_fprintf(stderr, OK, "[" AMOUNT " %zu:%zu] %s", test->count,
                    test->count, test->name);
  }
}

void cassert_short_print_tests(Tests *tests) {
  for (size_t i = 0; i < tests->count; ++i) {
    cassert_short_print_test(&tests->elements[i]);
  }
}

void cassert_print_tests(Tests *tests) {
  for (size_t i = 0; i < tests->count; ++i) {
    cassert_print_test(&tests->elements[i]);
  }
}

void cassert_free_case_value_mem(Cassert *cassert) {
  switch (cassert->assert_type) {
  case DOUBLE_EQ:
  case FLOAT_EQ: {
    if (cassert->value1) {
      free(cassert->value1);
    }
    if (cassert->value2) {
      free(cassert->value2);
    }
  } break;
  case STRING_INT64_EQ:
  case STRING_FLOAT_EQ:
  case STRING_DOUBLE_EQ: {
    if (cassert->value2) {
      free(cassert->value2);
    }
  } break;
  default:
    break;
  }
}

void cassert_array_free_case_value_mem(Test *test) {
  for (size_t i = 0; i < test->count; ++i) {
    cassert_free_case_value_mem(&test->elements[i]);
  }
}

void cassert_free_test(Test *test) {
  cassert_array_free_case_value_mem(test);
  free(test->elements);
}

void cassert_free_tests(Tests *tests) {
  for (size_t i = 0; i < tests->count; ++i) {
    cassert_free_test(&tests->elements[i]);
  }
  free(tests->elements);
}

bool cassert_set_last_cassert_description(Test *test, const char *description) {
  if (test->count == 0) {
    return false;
  }
  test->elements[test->count - 1].description = description;
  return true;
}

#endif // CASSERT_IMPLEMENTATION
