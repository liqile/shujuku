/*
 * print.h
 *
 *  Created on: 2014年12月27日
 *      Author: lql
 */

#ifndef PRINT_H_
#define PRINT_H_
#include <cstdio>
#include "utils/compare.h"
#include "database.h"
void printCol(Table* tb, int i, uchar* it) {
	if ((bm & (1 << i)) != 0) {
		printf("null\t");
	} else {
	    if (tb->col[i].cb.dt == LL_TYPE) {
    		ll v;
		    memcpy(&v, it + tb->col[i].cs, 8);
		    printf("%lld\t", v);
	    } else {
		    printf("%s\t", (char*)(it + tb->col[i].cs));
	    }
	}
}
void printCols1(uchar* it) {
	int n = cname.size();
	memcpy(&bm, it, 4);
	for (int i = 0; i < n; ++ i) printCol(ctb, ctb->cmap[cname[i]], it);
}
void printItem(Table* tb, uchar* it) {
    memcpy(&bm, it, 4);
    for (int i = 0; i < tb->cn; ++ i) printCol(ctb, i, it);
}


#endif /* PRINT_H_ */
