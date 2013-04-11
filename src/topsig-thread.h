#ifndef TOPSIG_THREAD_H
#define TOPSIG_THREAD_H

#include "topsig-search.h"
#include "topsig-document.h"

void ThreadYield();

// Threaded indexing
void ProcessFile_Threaded(Document *);
void Flush_Threaded();

// Threaded searching
Results *FindHighestScoring_Threaded(Search *, const int, const int, const int, unsigned char *, unsigned char *, int);

void DivideWork(void **job_inputs, void *(*start_routine)(void*), int jobs);

#endif
