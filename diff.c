#include <stdio.h>
#include <string.h>


#include "headers/diff.h"
#include "headers/allocator.h"
#include "headers/match.h"


#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_RESET   "\033[0m"

int diff_use_colors = 0;

struct diff_line {
    const char * start;
    unsigned int len;
};

struct diff_lines {
    struct diff_line * lines;
    unsigned int count;
};


static unsigned int count_lines(const char * str, unsigned int len);
static struct diff_lines split_lines(const char * str, unsigned int len);
static int lines_equal(struct diff_line * a, struct diff_line * b);
static void print_line_with_prefix(FILE * output, char prefix, struct diff_line * line);


void print_diff(FILE * output, struct string * expected, struct string * actual)
{
    struct diff_lines exp_lines, act_lines;
    unsigned int * lcs_table;
    unsigned int i, j, rows, cols;

    const char * exp_str = (expected != NULL) ? expected->value : "";
    unsigned int exp_len = (expected != NULL) ? expected->len : 0;
    const char * act_str = (actual != NULL) ? actual->value : "";
    unsigned int act_len = (actual != NULL) ? actual->len : 0;

    exp_lines = split_lines(exp_str, exp_len);
    act_lines = split_lines(act_str, act_len);

    rows = exp_lines.count + 1;
    cols = act_lines.count + 1;

    lcs_table = (unsigned int *) memory_blob_pool_alloc(
        &memory_pool,
        rows * cols * sizeof(unsigned int)
    );
    memset(lcs_table, 0, rows * cols * sizeof(unsigned int));

    for (i = 1; i < rows; ++i) {
        for (j = 1; j < cols; ++j) {
            if (lines_equal(&exp_lines.lines[i - 1], &act_lines.lines[j - 1])) {
                lcs_table[i * cols + j] = lcs_table[(i - 1) * cols + (j - 1)] + 1;
            } else {
                unsigned int up = lcs_table[(i - 1) * cols + j];
                unsigned int left = lcs_table[i * cols + (j - 1)];
                lcs_table[i * cols + j] = (up > left) ? up : left;
            }
        }
    }

    fprintf(output, "--- Expected\n+++ Actual\n");

    {
        unsigned int stack_size = exp_lines.count + act_lines.count;
        char * actions;
        unsigned int * indices;
        unsigned int top = 0;
        unsigned int k;

        actions = (char *) memory_blob_pool_alloc(&memory_pool, stack_size * sizeof(char));
        indices = (unsigned int *) memory_blob_pool_alloc(&memory_pool, stack_size * sizeof(unsigned int));

        i = exp_lines.count;
        j = act_lines.count;

        while (i > 0 || j > 0) {
            if (i > 0 && j > 0 && lines_equal(&exp_lines.lines[i - 1], &act_lines.lines[j - 1])) {
                actions[top] = ' ';
                indices[top] = i - 1;
                ++top;
                --i;
                --j;
            } else if (j > 0 && (i == 0 || lcs_table[i * cols + (j - 1)] >= lcs_table[(i - 1) * cols + j])) {
                actions[top] = '+';
                indices[top] = j - 1;
                ++top;
                --j;
            } else {
                actions[top] = '-';
                indices[top] = i - 1;
                ++top;
                --i;
            }
        }

        for (k = top; k > 0; --k) {
            char action = actions[k - 1];
            unsigned int idx = indices[k - 1];

            if (action == ' ') {
                print_line_with_prefix(output, ' ', &exp_lines.lines[idx]);
            } else if (action == '-') {
                print_line_with_prefix(output, '-', &exp_lines.lines[idx]);
            } else {
                print_line_with_prefix(output, '+', &act_lines.lines[idx]);
            }
        }
    }
}


unsigned int count_lines(const char * str, unsigned int len)
{
    unsigned int count = 0;
    unsigned int i;

    if (len == 0) {
        return 0;
    }

    for (i = 0; i < len; ++i) {
        if (str[i] == '\n') {
            ++count;
        }
    }

    if (len > 0 && str[len - 1] != '\n') {
        ++count;
    }

    return count;
}


struct diff_lines split_lines(const char * str, unsigned int len)
{
    struct diff_lines result;
    unsigned int line_count;
    unsigned int i, line_idx;
    const char * line_start;

    line_count = count_lines(str, len);

    result.count = line_count;

    if (line_count == 0) {
        result.lines = NULL;
        return result;
    }

    result.lines = (struct diff_line *) memory_blob_pool_alloc(
        &memory_pool,
        line_count * sizeof(struct diff_line)
    );

    line_start = str;
    line_idx = 0;

    for (i = 0; i < len; ++i) {
        if (str[i] == '\n') {
            result.lines[line_idx].start = line_start;
            result.lines[line_idx].len = (unsigned int)(str + i - line_start);
            ++line_idx;
            line_start = str + i + 1;
        }
    }

    if (line_start < str + len) {
        result.lines[line_idx].start = line_start;
        result.lines[line_idx].len = (unsigned int)(str + len - line_start);
    }

    return result;
}


int lines_equal(struct diff_line * a, struct diff_line * b)
{
    return pattern_match(a->start, a->len, b->start, b->len);
}


void print_line_with_prefix(FILE * output, char prefix, struct diff_line * line)
{
    const char * color = NULL;

    if (diff_use_colors) {
        if (prefix == '-') {
            color = ANSI_COLOR_RED;
        } else if (prefix == '+') {
            color = ANSI_COLOR_GREEN;
        }
    }

    if (color != NULL) {
        fprintf(output, "%s", color);
    }

    fprintf(output, "%c", prefix);
    fwrite(line->start, sizeof(char), line->len, output);

    if (color != NULL) {
        fprintf(output, "%s", ANSI_COLOR_RESET);
    }

    fprintf(output, "\n");
}
