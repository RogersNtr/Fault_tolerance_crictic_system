#include "../header/Count.h"

static int Count = 0;

inline int IncCount(void){
	Count+=1;
	return(Count);
}

inline int GetCount(void){
	return(Count);
}

inline void ClearCount(void){
	Count = 0;
}
