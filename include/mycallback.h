#ifndef MYCALLBACK_H
#define MYCALLBACK_H
#include "vrp.h"

int CPXPUBLIC my_callback(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle);

int my_callback_relaxation(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                           void *userhandle);

int my_callback_candidate(CPXCALLBACKCONTEXTptr context, CPXLONG contextid,
                          void *userhandle);

int doit_fn_concorde(double cutval, int cutcount, int *cut, void *inparam);
#endif /* ifndef MYCALLBACK_H                                                  \
#define MYCALLBACK_H */
