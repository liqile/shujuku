/*
 * TableManager.h
 *
 *  Created on: 2014年12月4日
 *      Author: lql
 */
#ifndef TABLEMANAGER_H_
#define TABLEMANAGER_H_
#include <string>
#include <vector>
#include "../utils/MyLinkList.h"
#include "../utils/pagedef.h"
#include "../bplustree/BPlusTree.h"
#include "../utils/compare.h"
#include <map>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
using namespace std;
struct CB {
	int dt;
	int cl;
	int nl;
	int ut;
	int nt;
};
struct Col {
	CB cb;
	char* name;
	int root[2];
	int cs;
	int tid;
	BPlusTree* bpt[2];
};
struct Cmp {
	bool operator()(const char* a, const char* b) {
		return (strcmp(a, b) < 0);
	}
};
struct Create {
	int n;
	CB* c;
    char** name;
    int nl;
    char* tn;
};
struct Range {
	int rt;
	uchar* a;
	uchar* b;
};
struct Table {
	char* tn;
	int nl;
	Col col[MAX_COL_NUM];
	Range range[MAX_TB_NUM];
	Range rangeCopy[MAX_TB_NUM];
	int cn;
	map<char*, int, Cmp> cmap;
	NodeManager* nm;
	int fid;
	int len;
	uchar* ib;
	ListLayout* lo;
	int dh;
	int dt;
	Table(
		int dbid,
		NodeManager* nmg,
		bool create, void* p
	) {
		fid = dbid;
		nm = nmg;
		if (create) {
			Create* q = (Create*) p;
			tn = q->tn;
			//strcpy(tn, q->tn);
			nl = q->nl;
			createt(q->n, q->c, q->name);
		} else {
			int* q = (int*)p;
			load(*q);
			openIndex();
		}
	}
	uchar* getItem(int p, int s) {
		ItemList* d = nm->getList(dt, p, lo);
		uchar* it = d->itemAt(s);
		free(d);
		return it;
	}
	void load(int fd) {
		read(fd, &nl, sizeof(int));
		tn = new char[nl];
		read(fd, tn, nl);
		read(fd, &cn, sizeof(int));
		read(fd, &dh, sizeof(int));
		dt = nm->nt();
		len = 4;
		for (int i = 0; i < cn; ++ i) {
			Col& cl = col[i];
			read(fd, &(cl.cb), sizeof(CB));
			cl.name = new char[cl.cb.nl];
			read(fd, cl.name, cl.cb.nl);
			read(fd, cl.root, 8);
			cl.cs = len;
			len += cl.cb.cl;
			cmap[cl.name] = i;
			range[i].a = new uchar[cl.cb.cl + 8];
			range[i].b = new uchar[cl.cb.cl + 8];
			rangeCopy[i].a = new uchar[cl.cb.cl + 8];
			rangeCopy[i].b = new uchar[cl.cb.cl + 8];
		}
		ib = new uchar[len];
		lo = new ListLayout(len);
	}
	void save(int fd) {
		write(fd, &nl, sizeof(int));
		cout << nl << endl;
		write(fd, tn, nl);
		write(fd, &cn, sizeof(int));
		write(fd, &dh, sizeof(int));
		for (int i = 0; i < cn; ++ i) {
			Col& cl = col[i];
			write(fd, &(cl.cb), sizeof(CB));
			write(fd, cl.name, cl.cb.nl);
			if (cl.bpt[0] != NULL) {
				cl.root[0] = cl.bpt[0]->rootID;
				cl.root[1] = cl.bpt[1]->rootID;
			}
			write(fd, cl.root, 8);
		}
	}
	int keyLen(CB& a) {
		if (a.ut == UNI) {
			return a.cl;
		}
		return a.cl + 8;
	}
	cf* cmp(CB& a) {
		return kcmp[a.dt][a.ut];
	}
	void getTrees(Col& a) {
		a.tid = nm->nt();
		a.bpt[0] = new BPlusTree (
			a.tid, nm, keyLen(a.cb), a.cb.cl + 8,
			cmp(a.cb), a.root[0], a.cb.cl
		);
		a.bpt[1] = new BPlusTree (
			a.tid, nm, 8, 8, &keyu<ll>, a.root[1], 0
		);
	}
	void openIndex() {
		for (int i = 0; i < cn; ++ i) {
			Col& a = col[i];
			a.bpt[0] = a.bpt[1] = NULL;
			if (a.root[0] == -1) continue;
			getTrees(a);
		}
	}
	void createt(int n, CB* c, char** name) {
		len = 4;
		cn = n;
		for (int i = 0; i < n; ++ i) {
			Col& cc = col[i];
			cc.cb = c[i];
			cc.name = name[i];
			cmap[cc.name] = i;
			cc.root[0] = cc.root[1] = -1;
			cc.bpt[0] = cc.bpt[1] = NULL;
			cc.cs = len;
			len += c[i].cl;
			range[i].a = new uchar[c[i].cl + 8];
			range[i].b = new uchar[c[i].cl + 8];
			rangeCopy[i].a = new uchar[c[i].cl + 8];
			rangeCopy[i].b = new uchar[c[i].cl + 8];
		}
		ib = new uchar[len];
		lo = new ListLayout(len);
		dt = nm->nt();
		ItemList* d = nm->newList(dt, true, lo);
		dh = d->id;
		free(d);
	}
	bool insertbpt(uchar* it, int k, int p, int s) {
		int bm;
		memcpy(&bm, it, 4);
		if (bm < 0) return false;
		if ((bm & (1 << k)) > 0) {
			memcpy(ib, &p, 4);
			memcpy(ib + 4, &s, 4);
			return col[k].bpt[1]->insertValue(ib);
		} else {
			memcpy(ib, it + col[k].cs, col[k].cb.cl);
			memcpy(ib + col[k].cb.cl, &p, 4);
			memcpy(ib + col[k].cb.cl + 4, &s, 4);
			return col[k].bpt[0]->insertValue(ib);
		}
	}
	void erasebpt(uchar* it, int k, int p, int s) {
		int bm;
		memcpy(&bm, it, 4);
		if (bm < 0) return;
		if ((bm & (1 << k)) > 0) {
			memcpy(ib, &p, 4);
			memcpy(ib + 4, &s, 4);
			col[k].bpt[1]->eraseValue(ib);
		} else {
			memcpy(ib, it + col[k].cs, col[k].cb.cl);
			memcpy(ib + col[k].cb.cl, &p, 4);
			memcpy(ib + col[k].cb.cl + 4, &s, 4);
			col[k].bpt[0]->eraseValue(ib);
		}
	}
	bool createIndex(char* name) {
		map<char*, int>::iterator it = cmap.find(name);
		if (it == cmap.end()) {
		//	cout <<" no col"<<endl;
			return false;
		}
		int k = it->second;
		if (col[k].bpt[0] != NULL) {
		//	cout <<" tree"<<endl;
			return false;
		}
		col[k].root[0] = col[k].root[1] = -1;
		getTrees(col[k]);
		ItemList* d = nm->getList(dt, dh, lo);
		while (true) {
			int n = d->keyNum();
			for (int i = 0; i < n; ++ i) {
				uchar* it = d->itemAt(i);
				insertbpt(it, k, d->id, i);
			}
			n = d->header->next;
			free(d);
			if (n == -1) break;
			d = nm->getList(dt, n, lo);
		}
	//	cout <<" ok"<<endl;
		return true;
	}
	bool insert(uchar* item, int& p, int& s) {
		ItemList* d = nm->getList(dt, dh, lo);
		if (d->keyNum() >= d->capacity()) {
			ItemList* dn = nm->newList(dt, true, lo);
			dn->header->next = dh;
			dh = dn->id;
			free(d);
			d = dn;
		}
		p = d->id;
		s = d->keyNum();
		d->markDirty();
		d->insertBefore(s, item);
		free(d);
		bool ret = true;
		for (int i = 0; i < cn; ++ i) {
			if (col[i].bpt[0] == NULL) {
				continue;
			}
			ret = insertbpt(item, i, p, s);
			//if (!ret) printf("same");
			if (!ret) {
				for (int j = 0; j < i; ++ j) erasebpt(item, j, p, s);
				item[3] ^= (1 << 7);
				break;
			}
		}
		return ret;
	}
	void erase(int p, int s) {
		ItemList* d = nm->getList(dt, p, lo);
		uchar* kk = d->itemAt(s);
		d->markDirty();
		for (int i = 0; i < cn; ++ i) {
			if (col[i].bpt[0] == NULL) continue;
			erasebpt(kk,i,p,s);
		}
		//memset(kk, 0xff, 4);
		kk[3] ^= (1 << 7);
		free(d);
	}
	void reinsert(int p, int s) {
		ItemList* d = nm->getList(dt, p, lo);
		uchar* kk = d->itemAt(s);
		d->markDirty();
		kk[3] ^= (1 << 7);
		for (int i = 0; i < cn; ++ i) {
			if (col[i].bpt[0] == NULL) continue;
			insertbpt(kk, i, p, s);
		}
		free(d);
	}
	bool inRange(Range* r, uchar* item) {
		int bm;
		memcpy(&bm, item, 4);
		for (int i = 0; i < cn; ++ i)
			if (r[i].rt == NOTHING) return false;
		for (int i = 0; i < cn; ++ i) {
			cf* cmp = kcmp[col[i].cb.dt][UNUNI];
			if (r[i].rt == ALL) continue;
			if ((bm & (1 << i)) > 0) {
				if (r[i].rt == RANGE) return false;
			} else {
				if (r[i].rt == ISNULL) return false;
				memcpy(ib, item + col[i].cs, col[i].cb.cl);
				tmp = col[i].cb.cl;
				if (cmp(r[i].a, ib) > 0) return false;
				if (cmp(ib, r[i].b) > 0) return false;
			}
		}
		return true;
	}
	void selectAll(int k, int isNull, Range* r, vector<pair<int, int> >& res) {
		memset(ib, 0, len);
		BPlusTree* bpt = col[k].bpt[isNull];
		ItemList* l = bpt->findLeft();
		int st = (isNull == 1) ? 0 : col[k].cb.cl;
		while (true) {
			int n = l->keyNum();
		//	cout << "n"<<n<<endl;
			for (int i = 0; i < n; ++ i) {
				uchar* it = l->itemAt(i);
				int p, s;
				memcpy(&p, it + st, 4);
				memcpy(&s, it + st + 4, 4);
				ItemList* d = nm->getList(dt, p, lo);
				it = d->itemAt(s);
				if (inRange(r, it)) {
					pair<int, int> pr(p, s);
					res.push_back(pr);
				}
				free(d);
			}
			n = l->header->next;
			free(l);
		//	cout << n << endl;
			if (n == -1) break;
			l = nm->getList(col[k].tid, n, bpt->layout.leafLayout);
		}
	}
	void selectRange(int k, Range* r, vector<pair<int, int> >& res) {
		memset(ib, 0, len);
		BPlusTree* bpt = col[k].bpt[0];
		int p1, s1;
		bpt->findValue(r[k].a, p1, s1);
		ItemList* l = nm->getList(col[k].tid, p1, bpt->layout.leafLayout);
		cf* cmp = kcmp[col[k].cb.dt][UNUNI];
		while (true) {
			uchar* it = l->itemAt(s1);
			tmp = col[k].cb.cl;
			if (cmp(it, r[k].b) > 0) {
				free(l);
				break;
			}
			int p, s;
			memcpy(&p, it + col[k].cb.cl, 4);
			memcpy(&s, it + col[k].cb.cl + 4, 4);
			ItemList* d = nm->getList(dt, p, lo);
			it = d->itemAt(s);
			if (inRange(r, it)) {
				pair<int, int> pr(p, s);
				res.push_back(pr);
			}
			free(d);
			s1 ++;
			if (s1 >= l->keyNum()) {
				s1 = 0;
				p1 = l->header->next;
				free(l);
				if (p1 == -1) {
					break;
				}
				l = nm->getList(col[k].tid, p1, bpt->layout.leafLayout);
			}
		}
	}
	int rangeCount(int k) {
		Col& c = col[k];
		Range& r = range[k];
		if (r.rt == NOTHING) {
			return 0;
		}
		if (r.rt == ALL) {
			return c.bpt[0]->size() + c.bpt[1]->size();
		}
		if (r.rt == ISNULL) {
			return c.bpt[1]->size();
		}
		return c.bpt[0]->count(r.a, r.b);
	}
	void select(int k, Range* r, vector<pair<int, int> >& res) {
		if (r[k].rt == ALL) {
			tt = 2;
			cout <<"all"<<endl;
			selectAll(k, 0, r, res);
			tt = 0;
			cout <<"all"<<endl;
			selectAll(k, 1, r, res);
		} else if (r[k].rt == RANGE) {
			selectRange(k, r, res);
		} else if (r[k].rt == ISNULL) {
			selectAll(k, 1, r, res);
		}
	}
	bool dropIndex(char* name) {
		map<char*, int>::iterator it = cmap.find(name);
		if (it == cmap.end()) {
		//	cout <<" no col"<<endl;
			return false;
		}
		int k = it->second;
		if (col[k].bpt[0] == NULL) {
		//	cout <<" tree"<<endl;
			return false;
		}
		nm->ct(col[k].tid, false);
		delete col[k].bpt[0];
		delete col[k].bpt[1];
		col[k].bpt[0] = col[k].bpt[1] = NULL;
		col[k].tid = -1;
		col[k].root[0] = col[k].root[1] = -1;
	//	cout <<" ok"<<endl;
		return true;
	}
	void closeTB(bool reserve) {
		for (int i = 0; i < cn; ++ i) {
			if (col[i].bpt[0] != NULL) {
				nm->ct(col[i].tid, reserve);
			}
		}
		nm->ct(dt, reserve);
	}
	void rangeAll() {
		for (int i = 0; i < cn; ++ i) {
			range[i].rt = ALL;
		}
	}
	void copyRange(bool* c) {
		for (int i = 0; i < cn; ++ i) {
			if (!c[i]) continue;
			rangeCopy[i].rt = range[i].rt;
			memcpy(rangeCopy[i].a, range[i].a, col[i].cb.cl + 8);
			memcpy(rangeCopy[i].b, range[i].b, col[i].cb.cl + 8);
		}
	}
	void backRange(bool* c) {
		for (int i = 0; i < cn; ++ i) {
			if (!c[i]) continue;
			range[i].rt = rangeCopy[i].rt;
			memcpy(range[i].a, rangeCopy[i].a, col[i].cb.cl + 8);
			memcpy(range[i].b, rangeCopy[i].b, col[i].cb.cl + 8);
		}
	}
	void merge(int k, const Range& r2) {
		Range& r1 = range[k];
		int len = col[k].cb.cl + 8;
		cf* cmp = kcmp[col[k].cb.dt][UNUNI];
		if (r1.rt == ALL) {
			r1.rt = r2.rt;
			memcpy(r1.a, r2.a, len);
			memcpy(r1.b, r2.b, len);
			return;
		}
		if (r1.rt != r2.rt) {
			r1.rt = NOTHING;
			return;
		}
		if (r1.rt == ISNULL) return;
		if (cmp(r1.a, r2.b) > 0 || cmp(r1.b, r2.a) < 0) {
			r1.rt = NOTHING;
			return;
		}
		if (cmp(r1.a, r2.a) < 0) memcpy(r1.a, r2.a, len);
		if (cmp(r1.b, r2.b) > 0) memcpy(r1.b, r2.b, len);
	}
	int getcolid(char* n) {
		map<char*,int,Cmp>::iterator it = cmap.find(n);
		if (it == cmap.end()) {
			return -1;
		}
		return it->second;
	}
	~Table() {
		cmap.clear();
		for (int i = 0; i < cn; ++ i) {
			delete[] col[i].name;
			if (col[i].bpt[0] != NULL) {
				delete col[i].bpt[0];
				delete col[i].bpt[1];
			}
			delete[] range[i].a;
			delete[] range[i].b;
		}
		delete[] ib;
		delete[] tn;
		delete lo;
	}
};

#endif /* TABLEMANAGER_H_ */
