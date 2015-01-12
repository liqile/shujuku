/*
 * NodeManager.h
 *
 *  Created on: 2014年11月10日
 *      Author: lql
 */

#ifndef NODEMANAGER_H_
#define NODEMANAGER_H_
#include "../utils/MyLinkList.h"
#include "../recordmanager/ItemList.h"
#include "../recordmanager/layout.h"
#include <string>
using namespace std;
struct Layout {
	ListLayout* innerLayout;
	ListLayout* leafLayout;
};
struct NodeManager {
public:
	int fileID;
	int cur;
	int num;
	BufPageManager* bpm;
	FileManager* fm;
	MyLinkList* bpl;
	int rootBufIndex;
	int getPageID(int pos) {
		return pos;
	}
	int getPos(int pageID) {
		return pageID;
	}
public:
	NodeManager(
		MyLinkList* bplist,
	    FileManager* fmg,
	    BufPageManager* bufPM,
	    int fid,
	    int nodeNum
	) {
		bpl = bplist;
		cur = 1;
		fm = fmg;
		rootBufIndex = -1;
		bpm = bufPM;
		fileID = fid;
		num = nodeNum;
	}
	ItemList* getList(int tid, int nodeID, const Layout& layout) {
		int index;
		BufType b = bpm->getPage(fileID, nodeID, index);
		bpl->insert(tid, index);
		ListHeader* header = (ListHeader*) b;
		ListLayout* lo = (header->listType == LEAF_TYPE)
				         ? layout.leafLayout : layout.innerLayout;
		ItemList* list = new ItemList(
			b, lo, bpm, index, nodeID
		);
		return list;
	}
	ItemList* getList(int tid, int nodeID, ListLayout* lo) {
		int index;
		BufType b = bpm->getPage(fileID, nodeID, index);
		bpl->insert(tid, index);
		ItemList* list = new ItemList(
			b, lo, bpm, index, nodeID
		);
		return list;
	}
	ItemList* newList(int tid, bool isLeaf, ListLayout* layout) {
		int pos = num ++;
		int index;
		BufType b = bpm->allocPage(fileID, pos, index);
		bpl->insert(tid, index);
		layout->clear(b, isLeaf);
		ItemList* list = new ItemList(
			b, layout, bpm, index, pos
		);
		list->markDirty();
		return list;
	}
	void release(ItemList* node) {
		int index = node->bufIndex;
		bpm->release(index);
		bpl->del(index);
	}
	int nt() {
		return fm->newType();
	}
	void ct(int t, bool reserve) {
		int k = bpl->getFirst(t);
		while (!bpl->isHead(k)) {
			int n = bpl->next(k);
			if (reserve)
				bpm->writeBack(k);
			else
			    bpm->release(k);
			bpl->del(k);
			k = n;
		}
		fm->closeType(t);
	}
};

#endif /* NODEMANAGER_H_ */
