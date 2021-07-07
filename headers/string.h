#ifndef JCUNIT_STRING_H
#define JCUNIT_STRING_H

struct string
{
    char * value;
    unsigned int len;
};

struct string * make_string(const char * source, unsigned int len);

#endif /* JCUNIT_STRING_H */
