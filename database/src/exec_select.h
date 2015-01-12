/*
 * select.h
 *
 *  Created on: 2014年12月26日
 *      Author: lql
 */

#ifndef EXEC_SELECT_H_
#define EXEC_SELECT_H_

#include "database.h"
#include "parse_where.h"
#include "print.h"
void exec_select(char* sql) {
	if (cdbs == NULL) {
		printf("error: no db\n");
		return;
	}
	ctb = NULL;
	tname.clear();
	cname.clear();
	tabs.clear();
	char* prev = sql;
	char* next;
	all = false;
	while (true) {
		next = prev + 1;
		while(*next != '.' && *next != ',' && *next != ' ') ++ next;
		if (*next == '.') {
			*next = '\0';
			tname.push_back(prev);
			prev = next + 1;
			while (*next != ' ' && *next != ',') ++ next;
		}
		if (*next == ','){
			*next = '\0';
			cname.push_back(prev);
			prev = next + 1;
		} else {
			*next = '\0';
			cname.push_back(prev);
			prev = next + 1;
			break;
		}
	}
	if (cname.size() == 1 && *cname[0] == '*') all = true;
	while (*prev != ' ') ++ prev;
	++prev;
	while (true) {
		next = prev + 1;
		while (*next != ',' && *next != ' ') ++ next;
		char ch = *next;
		*next = '\0';
		Table* ttb = cdbs->getTable(prev);
		if (ttb == NULL) {
			printf("error: no table\n");
			return;
		}
		tabs.push_back(ttb);
		prev = next + 1;
		if (ch == ' ') break;
	}
	int n = tabs.size();
	if (n == 1) ctb = tabs[0];
	int csizek = cname.size();
	for (int i = 0; i < csizek; ++ i) {
		if (strcmp(cname[i], "*") == 0) break;
		if (n == 1) {
			if (ctb->getcolid(cname[i]) == -1) {
				printf("error: no column\n");
				return;
			}
		} else {
			Table* ctt = cdbs->getTable(tname[i]);
			if (ctt == NULL) {
				printf("error: no table\n");
				return;
			}
			if (ctt->getcolid(cname[i]) == -1) {
				printf("error: no column\n");
				return;
			}
		}
	}
	next = getWord(prev, ' ');
	exec_where(next + 1);
	if (errwhere == 1) return;
	getCost();
	ans1.clear();
	cout << cidx<<endl;
	ctb->select(cidx, ctb->range, ans1);
	int m = ans1.size();
	cout <<"ok"<<endl;
	int countt = 0;
	if (n == 1) {
		for (int i = 0; i < m; ++ i) {
			int p = ans1[i].first;
			int s = ans1[i].second;
			//cout << p <<"\t"<<s<<"\t"<<i - m<<endl;
			uchar* it = ctb->getItem(p, s);
			if (all) printItem(ctb, it);
			else printCols1(it);
		    printf("\n");
		    countt ++;
		}
		printf("counted:%d\n", countt);
		return;
	}
	int cns;
	if (!all) {
		cns = cname.size();
		for (int i = 0; i < cns; ++ i) {
			if (strcmp(tname[i], ctb->tn) == 0) {
				isFirst[i] = true;
				cid[i] = ctb->cmap[cname[i]];
			} else {
				isFirst[i] = false;
				cid[i] = stb->cmap[cname[i]];
			}
		}
	}
	int cn = tab2.size();
	for (int i = 0; i < stb->cn; ++ i) connect[i] = false;
	for (int i = 0; i < cn; ++ i) {
		if (strcmp(tab2[i]->tn, stb->tn) == 0) {
			Table* t = tab2[i];
			tab2[i] = tab1[i];
			tab1[i] = t;
			int c = col1[i];
			col1[i] = col2[i];
			col2[i] = c;
			op[i] = (op[i]<=3)?3-op[i]:op[i];
		}
		connect[col1[i]] = true;
	}
	stb->copyRange(connect);
	for (int i = 0; i < m; ++ i) {
		if (i != 0) stb->backRange(connect);
		int p = ans1[i].first;
		int s = ans1[i].second;
		uchar* it = ctb->getItem(p, s);
		int bm1;
		memcpy(&bm1, it, 4);
		uchar* cd;
		ans2.clear();
		for (int j = 0; j < cn; ++ j) {
			const Col& col = ctb->col[col2[j]];
			cd = it + col.cs;
			getRange(cd, col.cb.cl, col.cb.cl, col.cb.dt, op[j]);
			stb->merge(col1[j], cr);
		}
		int idx = -1;
		int w = -1;
		for (int j = 0; j < stb->cn; ++ j) {
			if (stb->col[j].bpt[0] != NULL) {
				if (w == -1 || w > stb->rangeCount(j)) {
					idx = j;
					w = stb->rangeCount(j);
				}
			}
		}
		stb->select(idx, stb->range, ans2);
		int m2 = ans2.size();
		for (int j = 0; j < m2; ++ j) {
			countt ++;
			int p2 = ans2[j].first;
			int s2 = ans2[j].second;
			uchar* it2 = stb->getItem(p2, s2);
			int bm2;
			memcpy(&bm2, it2, 4);
			if (all) {
				printItem(ctb, it);
				printItem(stb, it2);
			} else {
				for (int k = 0; k < cns; ++ k) {
					if (isFirst[k]) {
						bm = bm1;
						printCol(ctb, cid[k], it);
					} else {
						bm = bm2;
						printCol(stb, cid[k], it2);
					}
				}
			}
			printf("\n");
		}
	}
	printf("counted:%d\n", countt);
}

#endif /* SELECT_H_ */
