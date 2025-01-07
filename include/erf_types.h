#ifndef CBNB_ERF_TYPES_H
#define CBNB_ERF_TYPES_H

#define Str char*
#define ConstStr const char*


#define IS_EQUAL bool (*compare)(const void* , const void*)

#define typecheck(type, x)        \
({  type __dummy;                 \
    typeof(x) __dummy2;           \
    (void)(&__dummy == &__dummy2);\
    1;                            \
})

#endif //CBNB_ERF_TYPES_H