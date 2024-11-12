#ifndef CUTEST_H
#define CUTEST_H

#include <stdlib.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"
#include <stdbool.h>
#include <stdio.h>

// macro concat
#define _CONCAT(a, b) a ## _ ## b
#define CONCAT(a, b) _CONCAT(a,b)

#define UNIQUE_FUNC_NAME(name, counter) CONCAT(name, counter)
#define UNIQUE_REGISTER_NAME(name, counter) CONCAT(register, CONCAT(name, counter))

#define MAX_TESTS_IN_FILE 100
#define MAX_FILES 100
#define BUFFER_SIZE 256

typedef struct {
    char* func_name;
    void (*func)();
} registry_t;

typedef struct {
    registry_t registry[MAX_TESTS_IN_FILE];
    size_t num_tests;
    void (*before_func)();
    void (*after_func)();
    char* name;
} file_t;

typedef struct assert_result_t{
    size_t line;
    struct assert_result_t* next;
    char buffer[];
} assert_result_t;

void assert_(int test, const char* test_expression, size_t line);

#define cute_assert(test) assert_((test), #test, __LINE__)

assert_result_t* assert_result_list = NULL;
assert_result_t* current_assert_result = NULL;
bool test_failed = false;
bool global_failed = false;

file_t files[MAX_FILES] = {0};
size_t num_files = 0;

arena_t utest_arena = {0};

void register_test(file_t* file, void (*func)(), const char* name);
void register_before_test(file_t* file, void (*func)());
void register_after_test(file_t* file, void (*func)());
void run_all_tests();

file_t* get_file_by_name(const char* name);

#define TEST(name) _TEST(name, __COUNTER__, __FILE__)
#define _TEST(name, counter, file) \
    void UNIQUE_FUNC_NAME(name, counter)(); \
    __attribute__((constructor)) \
    void UNIQUE_REGISTER_NAME(name, counter)() { \
        register_test(get_file_by_name(__FILE__), UNIQUE_FUNC_NAME(name, counter), #name); \
    } \
    void UNIQUE_FUNC_NAME(name, counter)()

#define BEFORE_TEST(name) _BEFORE_TEST(name, __COUNTER__)
#define _BEFORE_TEST(name, counter) \
    void UNIQUE_FUNC_NAME(name, counter)(); \
    __attribute__((constructor)) \
    void UNIQUE_REGISTER_NAME(name, counter)() { \
        register_before_test(get_file_by_name(__FILE__), UNIQUE_FUNC_NAME(name, counter)); \
    } \
    void UNIQUE_FUNC_NAME(name, counter)()

#define AFTER_TEST(name) _AFTER_TEST(name, __COUNTER__)
#define _AFTER_TEST(name, counter) \
    void UNIQUE_FUNC_NAME(name, counter)(); \
    __attribute__((constructor)) \
    void UNIQUE_REGISTER_NAME(name, counter)() { \
        register_after_test(get_file_by_name(__FILE__), UNIQUE_FUNC_NAME(name, counter)); \
    } \
    void UNIQUE_FUNC_NAME(name, counter)()

#ifdef CUTEST_IMPLEMENTATION

void add_assert_result(const char* test_expression, size_t line) {
    if(assert_result_list == NULL) {
        current_assert_result = (assert_result_t*)arena_calloc(&utest_arena, 1, sizeof(assert_result_t) + BUFFER_SIZE);
        current_assert_result->next = NULL;
        current_assert_result->line = line;
        strncpy(current_assert_result->buffer, test_expression, BUFFER_SIZE);
        assert_result_list = current_assert_result;
        return;
    }
    current_assert_result->next = (assert_result_t*)arena_calloc(&utest_arena, 1, sizeof(assert_result_t) + BUFFER_SIZE);
    current_assert_result = current_assert_result->next;
    current_assert_result->next = NULL;
    strncpy(current_assert_result->buffer, test_expression, BUFFER_SIZE);
}

void register_test(file_t* file, void (*func)(), const char* name) {
    if (file->num_tests < MAX_TESTS_IN_FILE && func != NULL) {
        file->registry[file->num_tests].func = func;
        file->registry[file->num_tests].func_name = arena_strdup(&utest_arena, name);
        file->num_tests++;
    }
}

void register_before_test(file_t* file, void (*func)()) {
    file->before_func = func;
}

void register_after_test(file_t* file, void (*func)()) {
    file->after_func = func;
}

char* basename(const char* file) {
    char* start = strrchr(file, '/');
    if(start == NULL) return NULL;
    char* test_name = arena_strdup(&utest_arena, start+1);
    *strrchr(test_name, '.') = '\0';
    return test_name;
}

void run_all_tests() {
    for(size_t j = 0; j < num_files; j++) {
        printf("Test : %s\n", basename(files[j].name));
        for (int i = 0; i < files[j].num_tests; i++) {
            if (files[j].before_func != NULL) files[j].before_func();
            test_failed = false;
            files[j].registry[i].func();
            if(!test_failed) {
                printf("    \033[0;32m%s : OK\033[0m\n", files[j].registry[i].func_name);
            } else {
                global_failed = true;
                printf("    \033[0;31m%s : KO\033[0m\n", files[j].registry[i].func_name);
                for (assert_result_t* current = assert_result_list; current != NULL; current = current->next) {
                    printf("    \033[0;31m    %s at %s:%lu\033[0m\n", current->buffer, strrchr(files[j].name, '/')+1, current->line);
                }
            }
            current_assert_result = NULL;
            assert_result_list = NULL;
            if (files[j].after_func != NULL) files[j].after_func();
        }
    }
    
    if(global_failed) {
        printf("\033[0;31m tests failed\033[0m\n");
    } else {
        printf("\033[0;32mAll tests passed\033[0m\n");
    }
    arena_free(&utest_arena);
}

void assert_(int test, const char* test_expression, size_t line) {
    if(!test) {
        char buffer[BUFFER_SIZE] = {0};
        snprintf(buffer, BUFFER_SIZE, "Assertion (%s) failed", test_expression);
        add_assert_result(buffer, line);
        test_failed = true;
    }
}

file_t* get_file_by_name(const char* name) {
    for (size_t i = 0; i < num_files; i++) {
        if(strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    // create file
    if(num_files < MAX_FILES) {
        files[num_files].name = arena_strdup(&utest_arena, name);
        num_files++;
        return &files[num_files-1];
    }
    return NULL;
}

#endif // CUTEST_IMPLEMENTATION
#endif // CUTEST_H