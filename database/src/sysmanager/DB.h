/*
 * TableManager.h
 *
 *  Created on: 2014年12月4日
 *      Author: lql
 */

#ifndef DB_H
#define DB_H
#include <map>
#include "Table.h"
#include <vector>
#include <fstream>
using namespace std;
struct DB {
	FileManager* fm;
	BufPageManager* bpm;
	MyLinkList* bpl;
	NodeManager* nm;
	int fid;
	char* dname;
	char* fn;
	map<char*, Table*, Cmp> cmap;
	DB(
		char* name,
		FileManager* fmg,
		BufPageManager* bpmg,
		MyLinkList* bpList
	) {
		int l = strlen(name);
		dname = new char[l + 1];
		strcpy(dname, name);
		fn = new char[l + 5];
		strcpy(fn, dname);
		strcpy(fn + l, ".txt");
		fm = fmg;
		bpm = bpmg;
		bpl = bpList;
		fm->openFile(dname, fid);
		int fd = open(fn, O_RDWR);
		int nodenum;
		read(fd, &nodenum, 4);
		nm = new NodeManager(
			bpl, fm, bpm, fid, nodenum
		);
		int tn;
		read(fd, &tn, 4);
		cmap.clear();
		for (int i = 0; i < tn; ++ i) {
			Table* tb = new Table(
				fid, nm, false, &fd
			);
			cmap[tb->tn] = tb;
		}
		close(fd);
	}
	bool createTB(Create* c) {
		map<char*, Table*, Cmp>::iterator it = cmap.find(c->tn);
		if (it != cmap.end()) {
			return false;
		}
		Table* table = new Table(
			fid, nm, true, c
		);
		cmap[c->tn] = table;
		return true;
	}
	Table* getTable(char* name) {
		map<char*, Table*, Cmp>::iterator it = cmap.find(name);
		if (it == cmap.end()) {
			return NULL;
		}
		return it->second;
	}
	bool dropTB(char* name) {
		map<char*, Table*, Cmp>::iterator it = cmap.find(name);
		if (it == cmap.end()) {
			return false;
		}
		Table* tb = it->second;
		tb->closeTB(false);
		cmap.erase(it);
		delete tb;
		return true;
	}
	void dropDB() {
		map<char*, Table*, Cmp>::iterator it;
		for(it = cmap.begin(); it != cmap.end();) {
			Table* t = it->second;
			t->closeTB(false);
			cmap.erase(it++);
			delete t;
		}
		fm->closeFile(fid);
	}
	void closeDB() {
		int fd = open(fn, O_RDWR);
		write(fd, &(nm->num), sizeof(int));
		int n = cmap.size();
		write(fd, &n, sizeof(int));
		map<char*, Table*, Cmp>::iterator it;
		for (it = cmap.begin(); it != cmap.end();) {
			Table* t = it->second;
			t->closeTB(true);
			t->save(fd);
			cmap.erase(it++);
			delete t;
		}
		close(fd);
		fm->closeFile(fid);
	}
	void printtb() {
		printf("%d\n", cmap.size());
		map<char*, Table*, Cmp>::iterator it;
		for (it = cmap.begin(); it != cmap.end(); ++ it) {
			printf("%s ", it->second->tn);
		}
		printf("\n");
	}
	~DB() {
		delete[] dname;
		delete[] fn;
	}
};

#endif /* TABLEMANAGER_H_ */
