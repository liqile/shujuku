/*
 * TableTest.h
 *
 *  Created on: 2014年12月11日
 *      Author: lql
 */

#ifndef TABLETEST_H_
#define TABLETEST_H_
#include "../utils/compare.h"
#include "../bplustree/BPlusTree.h"
#include "Table.h"
struct TableTest {
	FileManager* fm;
	BufPageManager* bpm;
	MyLinkList* bpl;
	TableTest(
	    FileManager* fMan,
	    BufPageManager* bpMan,
	    MyLinkList* bplist
	) {
		fm = fMan;
		bpm = bpMan;
		bpl = bplist;
	}
	void testCreateTable() {
		int n = 4;
		CB cb[4];
		for (int i = 0; i < n; ++ i) {
			cb[i].cl = 8;
			cb[i].dt = LL_TYPE;
			cb[i].nl = 2;
		}
		cb[0].nt = cb[1].nt = UN;
		cb[2].nt = cb[3].nt = N;
		cb[0].ut = cb[2].ut = UNI;
		cb[1].ut = cb[3].ut = UNUNI;
		Create create;
		create.n = n;
		create.c = cb;
		create.name = new char*[n];
		for (int i = 0; i < n; ++ i) create.name[i] = new char[2];
		strcpy(create.name[0], "a");
		strcpy(create.name[1], "b");
		strcpy(create.name[2], "c");
		strcpy(create.name[3], "d");
		create.tn = new char[6];
		strcpy(create.tn, "table");
		create.nl = 6;
		fm->createFile("table");
		int fid;
		fm->openFile("table", fid);
		NodeManager* nm = new NodeManager(
		    bpl, fm, bpm, fid, 0
		);
		Table table(
		    fid, nm, true, &create
		);
		cout << "cap " << table.lo->itemCapacity << endl;
		int num = table.lo->itemCapacity * 1000;
		int num1 = table.lo->itemCapacity * 321;
		int a = 0;
		int b = num / 4;
		int c = num / 2;
		int d = num * 3 / 4;
		ll** tb = new ll*[num];
		int* p = new int[num];
		int* s = new int[num];
		for (int i = 0; i < num; ++ i) tb[i] = new ll[4];
		int* bm = new int[num];
		bool* erase = new bool[num];
		for (int i = 0; i < num; ++ i) erase[i] = false;
		for (int i = 0; i < num; ++ i) {
			bm[i] = 0;
			for (int j = 0; j < 4; ++ j) {
				tb[i][j] = i;
				if (cb[j].nt == N) {
					if (i % 5 == 0) {
						tb[i][j] = -1;
						bm[i] ^= (1 << j);
					}
				}
				if (cb[j].ut == UNUNI && tb[i][j] != -1) {
					tb[i][j] %= num1;
				}
			}
		}
		for (int i = 0; i < num; ++ i) {
			int x = rand() % num;
			if (x == i) continue;
			swap(bm[i], bm[x]);
			for (int j = 0; j < 4; ++ j) swap(tb[i][j], tb[x][j]);
		}
		cout << "inserting" << endl;
		uchar* item = new uchar[36];
		table.createIndex(create.name[0]);
		for (int i = a; i < b; ++ i) {
		//	cout << "inserting " << i << endl;
			memcpy(item, &bm[i], 4);
			memcpy(item + 4, tb[i], 32);
			table.insert(item, p[i], s[i]);
		}
		for (int i = a; i < b; ++ i) {
			int k = rand() % 6;
			if (k == 0) {
				erase[i] = true;
				table.erase(p[i], s[i]);
			}
		}
		table.createIndex(create.name[1]);
		for (int i = b; i < c; ++ i) {
			memcpy(item, &bm[i], 4);
			memcpy(item + 4, tb[i], 32);
			table.insert(item, p[i], s[i]);
		}
		table.createIndex(create.name[2]);
		for (int i = 0; i < c; ++ i) {
			int k = rand() % 6;
			if (k == 0&&!erase[i]){
				table.erase(p[i], s[i]);
				erase[i] = true;
			}
		}
		for (int i = c; i < d; ++ i) {
			memcpy(item, &bm[i], 4);
			memcpy(item + 4, tb[i], 32);
			table.insert(item, p[i], s[i]);
		}
		table.createIndex(create.name[3]);
		for (int i = d; i < num; ++ i) {
			memcpy(item, &bm[i], 4);
			memcpy(item + 4, tb[i], 32);
			table.insert(item, p[i], s[i]);
		}
		for (int i =b; i < num; ++ i) {
			int k = rand() % 5;
			if (k == 0 && !erase[i]) {
				erase[i] = true;
				table.erase(p[i], s[i]);
			}
		}
		cout << "inserted all" << endl;
		cout << num << "-" << num1 << endl;
		Range r[4];
		ll x[4] = {num / 3, num1 / 2, num / 6, num1 / 3};
		ll y[4] = {num * 3 / 4, num1 * 2 / 3, num * 3 / 4, num1 * 2 / 3};
		bool ex[4] = {false, false, true, true};
		bool ey[4] = {true, false, false, false};
		for (int i = 0; i < 4; ++ i) {
			r[i].rt = RANGE;
			r[i].a = new uchar[16];
			r[i].b = new uchar[16];
			getl(&x[i], ex[i], r[i].a);
			getr(&y[i], ey[i], r[i].b);
		}
		vector<pair<int, int> > vec;
		vec.clear();
		cout <<"ok"<<endl;
		table.select(r, vec);
		int cnt = 0;
		for (int i = 0; i < num; ++ i) {
			if (erase[i]) continue;
			bool ok = true;
			for (int j = 0; j < 4; ++ j) {
				if (!judge(tb[i][j], x[j], y[j], ex[j], ey[j])) ok = false;
			}
			if (ok) ++ cnt;
		}
		cout << "find " << vec.size() << " " << cnt << endl;
		vec.clear();
		table.selectRange(0, r, vec);
		cout << "find " << vec.size() << endl;
		table.dropIndex(create.name[1]);
		table.closeTB(true);
		cout <<"save1"<<endl;
		int fd = open("a.txt", O_RDWR);
		write(fd, &(nm->num), 4);
		table.save(fd);
		close(fd);
		cout <<"save"<<endl;
	}
	void opentb() {
		int fd = open("a.txt", O_RDWR);
		int fid;
		fm->openFile("table", fid);
		int nodenum;
		read(fd, &(nodenum), 4);
		NodeManager* nm = new NodeManager(
			bpl, fm, bpm, fid, nodenum
		);
		Table table(
		    fid, nm, false, &fd
		);
		close(fd);
		table.openIndex();
		int num = 213000;
		int num1 = 68373;
		Range r[4];
		ll x[4] = {num / 3, num1 / 2, num / 6, num1 / 3};
		ll y[4] = {num * 3 / 4, num1 * 2 / 3, num * 3 / 4, num1 * 2 / 3};
		bool ex[4] = {false, false, true, true};
		bool ey[4] = {true, false, false, false};
		for (int i = 0; i < 4; ++ i) {
			r[i].rt = RANGE;
			r[i].a = new uchar[16];
			r[i].b = new uchar[16];
			getl(&x[i], ex[i], r[i].a);
			getr(&y[i], ey[i], r[i].b);
		}
		vector<pair<int, int> > vec;
		vec.clear();
		cout <<"ok"<<endl;
		table.select(r, vec);
		cout << vec.size() << endl;
		char in[2] = {'b', '\0'};
		table.createIndex(in);
		vec.clear();
		table.select(r, vec);
		table.closeTB(false);
		remove("table");
		cout << vec.size() << endl;
	}
	void getl(void* x, bool e, uchar* a) {
		memcpy(a, x, 8);
		if (e) {
			memset(a + 8, 0xff, 8);
		} else {
			memset(a + 8, 0x7f, 8);
		}
	}
	void getr(void* x, bool e, uchar* a) {
		memcpy(a, x, 8);
		if (e) {
			memset(a + 8, 0x7f, 8);
		} else {
			memset(a + 8, 0xff, 8);
		}
	}
	bool judge(ll v, ll x, ll y, bool ex, bool ey) {
		if (x < v && v < y) return true;
		if (ex && x == v) return true;
		if (ey && y == v) return true;
		return false;
	}
};

#endif /* TABLETEST_H_ */
