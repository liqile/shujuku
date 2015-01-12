#ifndef BUF_PAGE_MANAGER
#define BUF_PAGE_MANAGER
#include "../utils/MyHashMap.h"
#include "../utils/MyBitMap.h"
#include "FindReplace.h"
#include "../utils/pagedef.h"
#include "../fileio/FileManager.h"
#include "../utils/MyLinkList.h"
struct BufPageManager {
public:
	int last;
	FileManager* fileManager;
	MyHashMap* hash;
	FindReplace* replace;
	//MyLinkList* bpl;
	bool* dirty;
	BufType* addr;
	BufType allocMem() {
		return new unsigned int[(PAGE_SIZE >> 2)];
	}
	BufType fetchPage(int typeID, int pageID, int& index) {
		BufType b;
		index = replace->find();
		b = addr[index];
		if (b == NULL) {
			b = allocMem();
			addr[index] = b;
		} else {
			if (dirty[index]) {
				int k1, k2;
				hash->getKeys(index, k1, k2);
				fileManager->writePage(k1, k2, b, 0);
				dirty[index] = false;
			}
		}
		hash->replace(index, typeID, pageID);
		return b;
	}
public:
	BufType allocPage(int fileID, int pageID, int& index, bool ifRead = false) {
		BufType b = fetchPage(fileID, pageID, index);
		if (ifRead) {
			fileManager->readPage(fileID, pageID, b, 0);
		}
		return b;
	}
	BufType getPage(int fileID, int pageID, int& index) {
		index = hash->findIndex(fileID, pageID);
		if (index != -1) {
			access(index);
			return addr[index];
		} else {
			BufType b = fetchPage(fileID, pageID, index);
			fileManager->readPage(fileID, pageID, b, 0);
			return b;
		}
	}
	void access(int index) {
		if (index == last) {
			return;
		}
		replace->access(index);
		last = index;
	}
	void markDirty(int index) {
		dirty[index] = true;
		access(index);
	}
	void release(int index) {
		dirty[index] = false;
		replace->free(index);
		hash->remove(index);
	}
	void writeBack(int index) {
		if (dirty[index]) {
			int f, p;
			hash->getKeys(index, f, p);
			fileManager->writePage(f, p, addr[index], 0);
			dirty[index] = false;
		}
		replace->free(index);
		hash->remove(index);
	}
	void close() {
		for (int i = 0; i < CAP; ++ i) {
			writeBack(i);
		}
	}
	void getKey(int index, int& fileID, int& pageID) {
		hash->getKeys(index, fileID, pageID);
	}
	BufPageManager(FileManager* fm) {
		int c = CAP;
		int m = MOD;
		last = -1;
		fileManager = fm;
		//bpl = new MyLinkList(CAP, MAX_FILE_NUM);
		dirty = new bool[CAP];
		addr = new BufType[CAP];
		hash = new MyHashMap(c, m);
	    replace = new FindReplace(c);
		for (int i = 0; i < CAP; ++ i) {
			dirty[i] = false;
			addr[i] = NULL;
		}
	}
};
#endif
