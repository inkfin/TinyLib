#ifndef TEST_H
#define TEST_H

#ifndef TL_SINGLE_TEST_FILE

extern int common_test_cases(void);
extern int dyn_arr_test_cases(void);
// extern int memory_pool_test_cases(void);
extern int logging_test_cases(void);

#else // TL_SINGLE_TEST_FILE

#define common_test_cases main
#define dyn_arr_test_cases main
#define memory_pool_test_cases main
#define logging_test_cases main

#endif // TL_SINGLE_TEST_FILE

#endif // TEST_H
