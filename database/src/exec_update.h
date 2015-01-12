/*
 * exec_update.h
 *
 *  Created on: 2014年12月26日
 *      Author: lql
 */

#ifndef EXEC_UPDATE_H_
#define EXEC_UPDATE_H_
#include "database.h"
#include "parse_where.h"
void exec_update(char* sql) {
	if (cdbs == NULL) {
		printf("error : no db\n");
		return;
	}
	char* prev, *next;
	prev = sql;
	next = getWord(prev, ' ');
	ctb = cdbs->getTable(prev);
	if (ctb == NULL) {
		printf("error: no table\n");
		return;
	}
	prev = next + 1;
	next = getWord(prev, ' ');
	int cc;
	int cn = 0;
	char* start = updatebuf;
	int cbm = 0;
	while (true) {
		prev = next + 1;
		next = getWord(prev, '=');
		//cc = ctb->cmap[prev];
		cc = ctb->getcolid(prev);
		if (cc == -1) {
			printf("error: no column\n");
			return;
		}
		cid[cn++] =  cc;
		const Col& col = ctb->col[cc];
		prev = next + 1;
		while (*prev == ' ') ++ prev;
		char brk;
		switch (col.cb.dt) {
		case LL_TYPE:
			if (*prev == '\'') {
				printf("error: update a integer-type element as a string.");
				return;
			}
			next = prev + 1;
			while (*next != ' ' && *next != ',') ++ next;
			brk = *next;
			*next = '\0';
			if (*prev == 'n') {
				if (col.cb.nt == UN) {
					printf("error: update a unnull-type element as null.");
					return;
				}
				cbm ^= (1 << cc);
				start += 8;
			} else {
			    ll key = atoll(prev);
			    memcpy(start, &key, 8);
			    start += 8;
			}
			break;
		case ST_TYPE:
			if (*prev == '\'') {
				prev ++;
				next = prev;
				while (*next != '\'') ++ next;
				*next = '\0';
				strcpy(start, prev);
			} else if (*prev == 'n') {
				if (col.cb.dt == UN) {
					printf("error: update a unnull-type element as null.");
					return;
				}
				next = prev + 1;
				cbm ^= (1<<cc);
			} else {
				printf("error: update a string with non-string value.");
				return;
			}
			start += col.cb.cl;
			while (*next != ' ' && *next != ',') ++ next;
			brk = *next;
		}
		if (brk == ' ') {
			*next = brk;
			while (*next == ' ') ++ next;
			brk = *next;
		}
		if (brk != ',') break;
	}
	prev = next;
	next = getWord(prev, ' ');
	tabs.clear();
	tabs.push_back(ctb);
	exec_where(next + 1);
	if (errwhere == 1) {
		return;
	}
	cidx = -1;
	int w = -1;
	for (int j = 0; j < ctb->cn; ++ j) {
		if (ctb->col[j].bpt[0] != NULL) {
			if (w == -1 || w > ctb->rangeCount(j)) {
				cidx = j;
				w = ctb->rangeCount(j);
			}
		}
	}
	ans1.clear();
	ctb->select(cidx, ctb->range, ans1);
	int m = ans1.size();
	int ppp, sss;
	for (int i = 0; i < m; ++ i) {
		int p = ans1[i].first;
		int s = ans1[i].second;
		uchar* it = ctb->getItem(p, s);
		memcpy(itemBuf, it, ctb->len);
		int kbm;
		memcpy(&kbm, itemBuf, 4);
		kbm |= cbm;
		memcpy(itemBuf, &kbm, 4);
		ctb->erase(p, s);
		start = updatebuf;
		for (int j = 0; j < cn; ++ j) {
			int cc = cid[j];
			if ((cbm & (1<<cc)) == 0) {
				memcpy(itemBuf + ctb->col[cc].cs, start, ctb->col[cc].cb.cl);
			}
			start += ctb->col[cc].cb.cl;
		}
		int pp, ss;
		bool ret = ctb->insert((uchar*)itemBuf, pp, ss);
		if (!ret) {
			for (int j = 0; j <= i; ++ j) ctb->reinsert(ans1[j].first, ans1[j].second);
			if (i > 0) ctb->erase(ppp, sss);
			printf("error: repeated occurrence\n");
			break;
		}
		ppp = pp;
		sss = ss;
	}
}
#endif /* EXEC_UPDATE_H_ */
