/*
 * database.h
 *
 *  Created on: 2014年12月25日
 *      Author: lql
 */

#ifndef DATABASE_H_
#define DATABASE_H_
#define COL_NAME 0
#include <iostream>
#include "utils/pagedef.h"
#include "recordmanager/ItemList.h"
#include "recordmanager/layout.h"
#include "utils/MyBitMap.h"
#include "sysmanager/DB.h"
#include <cstring>
#include <set>
#include "parser/parser.h"
#include <limits.h>
int e = 0;
FileManager* fm;
BufPageManager* bpm;
MyLinkList* bpl;
DB* cdbs;
multiset <char*, Cmp> dbs;
char buf[1000];
char itemBuf[1000];
char updatebuf[1000];
bool connect[100];
Range cr;
Range range[10];
FILE* fin;
int state;
int primary;
int errwhere;
int bm;
static Create create;
ll llmin = LONG_LONG_MIN;
ll llmax = LONG_LONG_MAX;
Table* ctb, *stb;
int cidx, sidx;
vector <char*> tname;
vector <char*> cname;
int cid[100];
bool isFirst[100];
vector <Table*> tabs;
int cost[100];
int idx[100];
vector <pair<int, int> > ans1, ans2;
vector <Table*> tab1, tab2;
vector <int> col1, col2, op;
bool all;
void setMin(uchar* a, int dt, int cl) {
	switch (dt) {
	case LL_TYPE:
		memcpy(a, &llmin, 8);
		memset(a + 8, 0xff, 8);
	case DB_TYPE:
		//todo
	default:
		a[0] = 0;
		memset(a + cl, 0xff, 8);
	}
}
void setMax(uchar* b, int dt, int cl) {
	switch (dt) {
    case LL_TYPE:
    	memcpy(b, &llmax, 8);
    	memset(b + 8, 0xff, 8);
    case DB_TYPE:
    	//todo
    default:
    	b[0] = 255;
    	b[1] = 0;
	    memset(b + cl, 0xff, 8);
    }
}
void getRange(void* src, int sl, int cl, int dt, int cp) {
	switch (cp) {
	case L:
		cr.rt = RANGE;
		memcpy(cr.b, src, sl);
		memset(cr.b + cl, 0xff, 8);
		setMin(cr.a, dt, cl);
		break;
	case LE:
		cr.rt = RANGE;
		memcpy(cr.b, src, sl);
		memset(cr.b + cl, 0x7f, 8);
		setMin(cr.a, dt, cl);
		break;
	case G:
		cr.rt = RANGE;
		memcpy(cr.a, src, sl);
		memset(cr.a + cl, 0x7f, 8);
		setMax(cr.b, dt, cl);
		break;
	case GE:
		cr.rt = RANGE;
		memcpy(cr.a, src, sl);
		memset(cr.a + cl, 0xff, 8);
		setMax(cr.b, dt, cl);
		break;
	case E:
		cr.rt = RANGE;
		memcpy(cr.a, src, sl);
		memset(cr.a + cl, 0xff, 8);
		memcpy(cr.b, src, sl);
		memset(cr.b + cl, 0x7f, 8);
		break;
	case IS:
		cr.rt = ISNULL;
	}
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
void getsl(char* x, bool e, int len, uchar* a) {
	strcpy((char*)a, x);
	if (e) {
		memset(a + len, 0xff, 8);
	} else {
		memset(a + len, 0x7f, 8);
	}
}
void getsr(char* x, bool e, int len, uchar* a) {
	strcpy((char*)a, x);
	if (e) {
		memset(a + len, 0x7f, 8);
	} else {
		memset(a + len, 0xff, 8);
	}
}
void init() {
	MyBitMap::initConst();
	create.name = new char*[MAX_COL_NUM];
	create.c = new CB[MAX_COL_NUM];
	cr.a = new uchar[1000];
	cr.b = new uchar[1000];
}
void load() {
	dbs.clear();
	int fd = open("sys.b", O_RDWR);
	int n;
	read(fd, &n, sizeof(int));
	for (int i = 0; i < n; ++ i) {
		int m;
		read(fd, &m, sizeof(int));
		char* buf = new char[m];
		read(fd, buf, m);
		dbs.insert(buf);
	}
	close(fd);
}
void clearAll() {
	fm->createFile("sys.b");
	int fd = open("sys.b", O_RDWR);
	int n = 0;
	write(fd, &n, sizeof(int));
	close(fd);
}
bool createdb(char* name) {
	multiset<char*, Cmp>::iterator it = dbs.find(name);
	if (it != dbs.end()) {
		return false;
	}
	fm->createFile(name);
	int l = strlen(name);
	char* n = new char[l + 1];
	strcpy(n, name);
	dbs.insert(n);
	char* fn = new char[l + 5];
	strcpy(fn, n);
	strcpy(fn + l, ".txt");
	fm->createFile(fn);
	int fd = open(fn, O_RDWR);
	int k = 0;
	write(fd, &k, sizeof(int));
	write(fd, &k, sizeof(int));
	close(fd);
	delete[] fn;
	return true;
}
bool dropdb(char* name) {
	multiset<char*, Cmp>::iterator it = dbs.find(name);
	if (it == dbs.end()) {
		return false;
	}
	if (cdbs != NULL && strcmp(cdbs->dname, name) == 0) {
		cdbs->dropDB();
		delete cdbs;
		cdbs = NULL;
	}
	char* n = *it;
	int l = strlen(n);
	char* fn = new char[l + 5];
	strcpy(fn, n);
	strcpy(fn + l, ".txt");
	remove(n);
	remove(fn);
	dbs.erase(it);
	delete[] n;
	delete[] fn;
	return true;
}
bool closedb() {
	if (cdbs == NULL) return false;
	cdbs->closeDB();
	delete cdbs;
	cdbs = NULL;
	return true;
}
bool usedb(char* name) {
	multiset<char*, Cmp>::iterator it = dbs.find(name);
	if (it == dbs.end()) {
		return false;
	}
	if (cdbs != NULL){
		if (strcmp(cdbs->dname, name) == 0) return false;
		closedb();
	}
	cdbs = new DB(
		name, fm, bpm, bpl
	);
	return true;
}
void shutdown() {
	if (cdbs != NULL) {
		cdbs->closeDB();
		delete cdbs;
	}
	multiset<char*, Cmp>::iterator it;
	int fd = open("sys.b", O_RDWR);
	int k = dbs.size();
	write(fd, &k, sizeof(int));
	for (it = dbs.begin(); it != dbs.end(); it++) {
		char* n = *it;
		int len = strlen(n) + 1;
		write(fd, &len, sizeof(int));
		write(fd, n, len);
	}
	close(fd);
	dbs.clear();
	cout << " shut down " << endl;
	exit(0);
}
#endif /* DATABASE_H_ */
