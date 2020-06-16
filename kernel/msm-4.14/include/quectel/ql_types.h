
#ifndef _QL_TYPES_H_
#define _QL_TYPES_H_

typedef unsigned int uint32;
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef int boolean;

#ifdef FALSE
#undef FALSE
#endif
#define FALSE                           (1 == 0)

#ifdef TRUE
#undef TRUE
#endif
#define TRUE                            (1 == 1)


#endif //_QL_TYPES_H_

