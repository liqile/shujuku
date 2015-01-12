/*
 * exec_insert.h
 *
 *  Created on: 2014年12月26日
 *      Author: lql
 */

#ifndef EXEC_INSERT_H_
#define EXEC_INSERT_H_
#include "database.h"
void exec_insert(char* sql) {
	char* next = sql;
	while (*next == ' ') ++ next;
	char* prev;
	int bm = 0;
	char* start = itemBuf + 4;
	if (ctb == NULL) return;
	int mc = ctb->cn - 1;
	for (int cc = 0; cc <= mc; ++ cc) {
		const CB& cb = ctb->col[cc].cb;
		int dt = cb.dt;
		int nt = cb.nt;
		char e = (cc == mc) ? ')' : ',';
		int len = cb.cl;
		prev = next + 1;
		switch (dt) {
		case LL_TYPE:
			next = getWord(prev, e);
			//printf("data\t%s\n", prev);
			if (*prev == '\'') {
				printf("error: insert string instead of int\n");
				return;
			}
			if (*prev != 'n') {
			    long long d = atoll(prev);
			    memcpy(start, &d, len);
			} else if (nt == N) {
				bm ^= (1 << cc);
			} else {
				printf("error: insert null value, but not null type\n");
				return;
			}
			break;
		case ST_TYPE:
			while(*prev == ' ') ++ prev;
			if (*prev == '\'') {
				next = prev + 1;
				int k = 0;
				while (*next != '\'') {
					k ++;
					next ++;
				}
				memcpy(start, prev + 1, k);
				*(start + k) = '\0';
				//printf("data\t%s\n", start);
				while (*next != e) ++ next;
			} else if (*prev == 'n' && nt == N) {
				bm ^= (1 << cc);
				next = prev + 1;
				while (*next != e) ++ next;
				//printf("data\t%s\n", next + 1);
			} else {
				printf("error:insert null string, but not null type\n");
				return;
			}
		}
		start += len;
	}
	memcpy(itemBuf, &bm, 4);
	int p, s;
	if (ctb != NULL) {
		bool ret = ctb->insert((uchar*)itemBuf, p, s);
		if (!ret) printf("error: repeated occurrence\n");
	} else printf("error\n");
	next ++;
	while (*next == ' ') ++ next;
	state = (*next == ';') ? START : INSERT;
}

#endif /* EXEC_INSERT_H_ */
