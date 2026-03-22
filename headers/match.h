#ifndef JCUNIT_MATCH_H
#define JCUNIT_MATCH_H 1

/*
 * Pattern matching with {{any}} placeholder support.
 * {{any}} matches zero or more characters in the actual string.
 * Returns 1 if pattern matches actual, 0 otherwise.
 */
int pattern_match(const char * pattern, unsigned int pattern_len,
                  const char * actual, unsigned int actual_len);

#endif /* JCUNIT_MATCH_H */
