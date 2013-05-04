#ifndef UTIL_HEAD_H_
#define UTIL_HEAD_H_
#include "util/array.h"

// assumption on max word/term length
#define WORD_BUF  32 

// assumption on max file line length
#define LINE_BUF  1024000

// assumption on max doc info length
#define DOC_BUF   1024000

// assumption on max path length
#define PATH_BUF   1024

// assumption on max length of posting (doc, or pos)
#define PST_BUF   1048576

// max size for permuterm (including $)
#define PERMU_BUF 30

#endif  // UTIL_HEAD_H_
