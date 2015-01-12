#ifndef MY_HASH_MAP
#define MY_HASH_MAP
#include "pagedef.h"
#include "MyLinkList.h"
struct DataNode {
	int key1;
	int key2;
};
class MyHashMap {
private:
	static const int A = 1;
	static const int B = 1;
	int CAP_, MOD_;
	MyLinkList* list;
	DataNode* a;
	int hash(int k1, int k2) {
		return (k1 * A + k2 * B) % MOD_;
	}
public:
	int findIndex(int k1, int k2) {
		int h = hash(k1, k2);
		int p = list->getFirst(h);
		while (!list->isHead(p)) {
			if (a[p].key1 == k1 && a[p].key2 == k2) {
				/*
				list.del(p);
				list.insertFirst(p);
				*/
				return p;
			}
			p = list->next(p);
		}
		return -1;
	}
	void replace(int index, int k1, int k2) {
		int h = hash(k1, k2);
		//cout << h << endl;
		list->insertFirst(h, index);
		a[index].key1 = k1;
		a[index].key2 = k2;
	}
	void remove(int index) {
		list->del(index);
		a[index].key1 = -1;
		a[index].key2 = -1;
	}
	void getKeys(int index, int& k1, int& k2) {
		k1 = a[index].key1;
		k2 = a[index].key2;
	}
	MyHashMap(int c, int m) {
		CAP_ = c;
		MOD_ = m;
		a = new DataNode[c];
		for (int i = 0; i < CAP_; ++ i) {
			a[i].key1 = -1;
			a[i].key2 = -1;
		}
		list = new MyLinkList(CAP_, MOD_);
	}
};
#endif
