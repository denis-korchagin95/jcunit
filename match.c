#include <string.h>

#include "headers/match.h"

#define PLACEHOLDER_NONE  0
#define PLACEHOLDER_ANY   1

struct placeholder {
    unsigned int kind;
    unsigned int len;
};

#define PLACEHOLDER_ANY_LEN 7

static const char PLACEHOLDER_ANY_STR[] = "{{any}}";

static struct placeholder detect_placeholder(const char * input, unsigned int remaining)
{
    struct placeholder result;
    result.kind = PLACEHOLDER_NONE;
    result.len = 0;

    if (remaining >= PLACEHOLDER_ANY_LEN && memcmp(input, PLACEHOLDER_ANY_STR, PLACEHOLDER_ANY_LEN) == 0) {
        result.kind = PLACEHOLDER_ANY;
        result.len = PLACEHOLDER_ANY_LEN;
    }

    return result;
}

/*
 * Find the next literal segment starting at pattern[start],
 * stopping before the next placeholder or end of pattern.
 */
static unsigned int literal_segment_len(const char * pattern, unsigned int pattern_len, unsigned int start)
{
    unsigned int len = 0;

    while (start + len < pattern_len) {
        if (detect_placeholder(pattern + start + len, pattern_len - start - len).kind != PLACEHOLDER_NONE) {
            break;
        }
        len++;
    }

    return len;
}

/*
 * Skip actual forward until the literal segment is found.
 * Returns 1 on success (actual_pos updated past the segment), 0 on failure.
 */
static int skip_to_segment(const char * pattern, unsigned int seg_start, unsigned int seg_len, const char * actual, unsigned int actual_len, unsigned int * actual_pos)
{
    const char * found;

    while (*actual_pos + seg_len <= actual_len) {
        found = (const char *) memchr(actual + *actual_pos, pattern[seg_start], actual_len - *actual_pos);
        if (found == NULL) {
            return 0;
        }
        *actual_pos = (unsigned int)(found - actual);
        if (*actual_pos + seg_len > actual_len) {
            return 0;
        }
        if (memcmp(found, pattern + seg_start, seg_len) == 0) {
            *actual_pos += seg_len;
            return 1;
        }
        (*actual_pos)++;
    }

    return 0;
}

static int match_placeholder_any(const char * pattern, unsigned int pattern_len, unsigned int * pattern_pos, const char * actual, unsigned int actual_len, unsigned int * actual_pos)
{
    unsigned int seg_start = *pattern_pos;
    unsigned int seg_len = literal_segment_len(pattern, pattern_len, seg_start);

    if (seg_len == 0) {
        if (*pattern_pos >= pattern_len) {
            /* {{any}} at end of pattern - match rest of actual */
            *actual_pos = actual_len;
        }
        /* followed by another placeholder, consume minimum (zero) chars */
        return 1;
    }

    if (!skip_to_segment(pattern, seg_start, seg_len, actual, actual_len, actual_pos)) {
        return 0;
    }

    *pattern_pos += seg_len;
    return 1;
}

int pattern_match(const char * pattern, unsigned int pattern_len, const char * actual, unsigned int actual_len)
{
    unsigned int pattern_pos = 0;
    unsigned int actual_pos = 0;

    while (pattern_pos < pattern_len) {
        struct placeholder holder = detect_placeholder(pattern + pattern_pos, pattern_len - pattern_pos);

        if (holder.kind != PLACEHOLDER_NONE) {
            pattern_pos += holder.len;

            switch (holder.kind) {
                case PLACEHOLDER_ANY:
                    if (!match_placeholder_any(pattern, pattern_len, &pattern_pos, actual, actual_len, &actual_pos)) {
                        return 0;
                    }
                    break;
                default:
                    return 0;
            }
        } else {
            if (actual_pos >= actual_len || pattern[pattern_pos] != actual[actual_pos]) {
                return 0;
            }
            pattern_pos++;
            actual_pos++;
        }
    }

    return actual_pos == actual_len;
}
