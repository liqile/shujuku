/*
 * IndexManager.h
 *
 *  Created on: 2014年11月15日
 *      Author: lql
 */

#ifndef INDEXMANAGER_H_
#define INDEXMANAGER_H_
#include <string>
#include "../bufmanager/BufPageManager.h"
#include "../fileio/FileManager.h"
#include "../utils/pagedef.h"
#include "../utils/compare.h"
#include "NodeManager.h"
#include "BPlusTree.h"
#include <cstdlib>
#include <string.h>
using namespace std;
class Test {
private:
	FileManager* fm;
	BufPageManager* bpm;
	BufType c;
	ListLayout* ll;
	//int type;
public:
	Test(FileManager* f, BufPageManager* b) {
		fm = f;
		bpm = b;
		c = new uint[PAGE_INT_NUM];
	}
	void buildTree(const char* name, int itemLen) {
		fm->createFile(name);
        ll = new ListLayout(itemLen);
		ll->clear(c, true);
	}
	void test(const char* name, int itemLen) {
		cout <<"enter" << endl;
		int fileID;
		fm->openFile(name, fileID);
		int type = fm->newType();
		cout << name << " open" << endl;
		int rid = -1;
		NodeManager* nm = new NodeManager (
		    fm, bpm, fileID, 0
		);
		BPlusTree* bpt = new BPlusTree (
		    nm, 4, itemLen, &keyu<int>,
		    rid, type
		);
		cout <<"building" << endl;
		cout << "--------------------" << ll->itemCapacity << endl;
		//cout << "====================" << pl->listMaxNum << endl;
		int num = ll->itemCapacity*10000;
		int* key = new int[num];
		for (int i = 0; i < num; ++ i) key[i] = i;
		bool* f = new bool[num];
		for (int i = 0; i < num; ++ i) f[i] = false;
		for (int i = 0; i < num; i += num) {
			int s = i;
			int t = i + num;
			if (t > num) t = num;
			int n = t - s;
			for (int j = 0; j < n; ++ j) {
			int x = j;
			int y = rand() % n;
			int k = key[s + x];
			key[s + x] = key[s + y];
			key[s + y] = k;
			}
		}
		cout << "inserting" << endl;
		uchar buf[100];
		for (int i = 0; i < num; ++ i) {
			//bpt->insertValue(key[i], key[i] + 1);
			//cout << i<<endl;
			int k = key[i];
			int v = k + 1;
			memcpy(buf, &k, 4);
			memcpy(buf + 4, &v, 4);
			bpt->insertValue(buf);
		}
#if DEBUG_ERASE
		cout << "erasing" << endl;
		int error = 0;
		int remain = ll->itemCapacity * 500;
		int reinsert = ll->itemCapacity * 2000;
		for (int i = 0; i < num - remain; ++ i) {
			memcpy(buf, &key[i], 4);
			bpt->eraseValue(buf);
			f[key[i]] = true;
		}
		for (int i = 0; i < reinsert; ++ i) {
			int k = key[i];
			int v = k + 1;
			memcpy(buf, &k, 4);
			memcpy(buf + 4, &v, 4);
			bpt->insertValue(buf);
			f[key[i]] = false;
		}
#endif
#if DEBUG_NEXT
		cout << "linking" << endl;
		ItemList* t = bpt->findLeft();
		//int error = 0;
		int num1 = 0;
		while (true) {
			int k = t->keyNum();
			for (int i = 0; i < k; ++ i) {
				int a, b;
				uchar* it = t->itemAt(i);
				memcpy(&a, it, 4);
				memcpy(&b, it + 4, 4);
				cout << a << " : " << b << endl;
				num1 ++;
				if (f[a]) {
					error ++;
				} else {
					if (a + 1 != b) {
						error ++ ;
					}
				}
			}
			k = t->header->next;
			free(t);
			if (k == -1) {
				break;
			}
			t = nm->getList(k, bpt->layout);
		}
		cout << remain + reinsert<< " " << num1 << " " << error << endl;
		int aa = 1344523;
		while (!f[aa]) ++ aa;
		int ba = aa;
		while (f[ba]) ++ ba;
		-- ba;
		//int ba = 3826626;
		cout << bpt->count((uchar*)&aa, (uchar*)&ba) << endl;
		//cout << bpt->size() << endl;
		//bpt->judge();
		int cnt = 0;
		for (int i = 0; i < num; ++ i) {
			if (f[i]) continue;
			if (aa <= i && i <= ba) {
				cnt ++;
			}
		}
		cout << cnt << endl;
		cout << "-----------------------------"<<endl;
		bpt->judge(bpt->rootID);
		cout <<"----------------------------"<<endl;
#endif
	}
};

#endif /* INDEXMANAGER_H_ */
