/*
 * BPlusTree.h
 *
 *  Created on: 2014年11月9日
 *      Author: lql
 */

#ifndef BPLUSTREE_H_
#define BPLUSTREE_H_

#include <iostream>
#include "NodeManager.h"
#include "../utils/pagedef.h"
#include "../recordmanager/ItemList.h"
#include "../utils/compare.h"
#include <string.h>
using namespace std;
struct BPlusTree {
public:
    NodeManager* nodeManager;
    Layout layout;
    int keyLen;
    int itemLen;
    int rootID;
    int sp;
    uchar* cur;
    cf* cmp;
    int cp, cs;
    int tid;
    bool findItemIndex(ItemList* list, int& index) {
    	if (list->keyNum() == 0) {
    		index = -1;
    		return false;
    	}
        int l = 0;
        uchar* it = list->itemAt(l);
        int c = cmp(it, cur);
        if (c == 1) {
        	index = l;
            return false;
        }
        if (c == 0) {
        	index = l;
        	return true;
        }
        int r = list->keyNum() - 1;
        it = list->itemAt(r);
        c = cmp(it, cur);
        if (c == -1) {
        	index = -1;
            return false;
        }
        if (c == 0) {
        	index = r;
            return true;
        }
        while (l + 1 < r) {
            int mid = ((l + r) >> 1);
            it = list->itemAt(mid);
            c = cmp(it, cur);
            if (c == 1) {
                r = mid;
            } else {
                l = mid;
            }
        }
        //it = list->itemAt(r - 1);
        if (cmp(list->itemAt(r - 1), cur) == 0) {
            r --;
            index = r;
            return true;
        }
        index = r;
        return false;
    }
    void enlarge(int childIndex, uchar* childItem, ItemList* child, ItemList* parent) {
    	child->markAccess();
    	//parent->markAccess();
    	int low = (child->maxKeyNum() + 1) >> 1;
    	if (parent == NULL) {
    		/*
    		 * child is root
    		 */
    		if (!child->isLeaf() && child->keyNum() == 1) {
    			//int newRootID = innerParser->getValue(child->itemAt(0));
    			int newRootID;
    			memcpy(&newRootID, child->itemAt(0) + keyLen, 4);
    			//nodeManager->setRoot(newRootID);
    			rootID = newRootID;
    			nodeManager->release(child);
    		}
    		return;
    	}
    	if (child->keyNum() < low) {
    		bool isLeft = true;
    		ItemList* brother;
    		uchar* brotherItem;
    		int brotherIndex;
    		if (childIndex > 0) {
    			brotherIndex = childIndex - 1;
    			brotherItem = parent->itemAt(brotherIndex);
    		} else {
    			brotherIndex = childIndex + 1;
    			brotherItem = parent->itemAt(brotherIndex);
    			isLeft = false;
    		}
    		int bid;
    		memcpy(&bid, brotherItem + keyLen, 4);
    		brother = nodeManager->getList(tid, bid, child->layout);
    		//bpl->insert(typeID, brother->bufIndex);
    		if (brother->keyNum() == low) {
    			/*
    			 * merge
    			 */
    			if (isLeft) {
    				brother->mergeFromRight(child);
    				brother->header->size += child->header->size;
    				memcpy(brotherItem, childItem, keyLen);
    				memcpy(brotherItem + keyLen + 4, &(brother->header->size), 4);
    				parent->eraseAt(childIndex);
    				nodeManager->release(child);
    				brother->markDirty();
    			} else {
    				child->mergeFromRight(brother);
    				child->header->size += brother->header->size;
    				memcpy(childItem, brotherItem, keyLen);
    				memcpy(childItem + keyLen + 4, &(child->header->size), 4);
    				parent->eraseAt(brotherIndex);
    				nodeManager->release(brother);
    				child->markDirty();
    			}
    		} else {
    			/*
    			 * move
    			 */
        	//	int start = child->isLeaf() ? keyStart : 0;
    			if (isLeft) {
    				int pos = brother->keyNum() - 1;
    				uchar* item = brother->itemAt(pos);
    				int d = 1;
    				if (!brother->isLeaf()) {
    					memcpy(&d, item + keyLen + 4, 4);
    				}
    				brother->header->size -= d;
    				child->header->size += d;
    				child->insertBefore(0, item);
    				brother->eraseAt(pos);
    				item = brother->itemAt(pos - 1);
    				memcpy(brotherItem, item, keyLen);
    			} else {
    				uchar* item = brother->itemAt(0);
    				int d = 1;
    				if (!brother->isLeaf()) {
    					memcpy(&d, item + keyLen + 4, 4);
    				}
    				brother->header->size -= d;
    				child->header->size += d;
    				child->insertBefore(child->keyNum(), item);
    				memcpy(childItem, item, keyLen);
    				brother->eraseAt(0);
    			}
    			memcpy(brotherItem + keyLen + 4, &(brother->header->size), 4);
    			memcpy(childItem + keyLen + 4, &(child->header->size), 4);
    			child->markDirty();
    			brother->markDirty();
    		}
    		parent->markDirty();
    	}
    }
    void split(int childIndex, ItemList* child, ItemList* parent, uchar* ci) {
    	int M = child->capacity();
        if (child->keyNum() > M) {
        	current ++;
       // 	int start = child->isLeaf() ? keyStart : 0;
        	M ++;
            ItemList* t = nodeManager->newList(tid, child->isLeaf(), child->layout);
            if (parent != NULL) {
            	int reserve = (M - (M >> 1));
            	child->splitToRight(t, reserve);
            	//child->updateSize();
            	t->updateSize();
            	child->header->size -= t->header->size;
            	uchar* it1 = child->itemAt(reserve - 1);
            	uchar* it2 = t->itemAt((M >> 1) - 1);
            	memcpy(ci, it1, keyLen);
            	//size
            	memcpy(ci + keyLen + 4, &(child->header->size), 4);
            	int nc = childIndex + 1;
            	uchar* it = parent->insertBefore(nc);
            	memcpy(it, it2, keyLen);
            	memcpy(it + keyLen, &(t->id), 4);
            	//size
            	memcpy(it + keyLen + 4, &(t->header->size), 4);
                parent->markDirty();
            } else {
            	int reserve = (M - (M >> 1));
                parent = nodeManager->newList(tid, false, layout.innerLayout);
                parent->header->size = child->header->size;
                //nodeManager->setRoot(parent);
                rootID = parent->id;
                child->splitToRight(t, reserve);
            	//child->updateSize();
            	t->updateSize();
            	child->header->size -= t->header->size;
                uchar* it1 = parent->insertBefore(0);
                uchar* it2 = parent->insertBefore(1);
                memcpy(it1, child->itemAt(reserve - 1), keyLen);
                memcpy(it1 + keyLen, &(child->id), 4);
                //size
                memcpy(it1 + keyLen + 4, &(child->header->size), 4);
                memcpy(it2, t->itemAt((M>>1)-1), keyLen);
                memcpy(it2 + keyLen, &(t->id), 4);
                //size
                memcpy(it2 + keyLen + 4, &(t->header->size), 4);
                parent->markDirty();
                delete parent;
            }
            t->markDirty();
            child->markDirty();
            delete t;
        }
    }
    bool find(ItemList* t) {
    	t->markAccess();
        if (t->isLeaf()) {
        	int r;
        	bool f = findItemIndex(t, r);
        	if (cp < 0) {
        		if (f) {
        			t->header->size --;
        			t->eraseAt(r);
        			t->markDirty();
        		}
        	} else {
        		cp = t->id;
        		cs = r;
        	}
        	return f;
        } else {
        	int index;
        	/*uchar* it = */
        	findItemIndex(t, index);
            if (index == -1) {
            	return false;
            }
            uchar* it = t->itemAt(index);
            //int id = innerParser->getValue(it);
            int id;
            memcpy(&id, it + keyLen, 4);
            ItemList* c = nodeManager->getList(tid, id, layout);
            //bpl->insert(typeID, c->bufIndex);
            bool ret = find(c);
            if (ret && cp < 0) {
            	t->header->size --;
            	t->markDirty();
                memcpy(it + keyLen + 4, &(c->header->size), 4);
            	enlarge(index, it, c, t);
            }
            free(c);
            return ret;
        }
    }
    bool insert(ItemList* t, uchar* item) {
    	t->header->size ++;
    	t->markDirty();
    	bool ret;
        if (t->isLeaf()) {
            int index;
            ret = findItemIndex(t, index);
            if (ret) {
            	return false;
            }
            if (index == -1) {
            	index = t->keyNum();
            }
            t->insertBefore(index, item);
            return true;
        } else {
        	int index;
        	findItemIndex(t, index);
        	uchar* childItem;
            if (index == -1) {
                index = t->keyNum() - 1;
                childItem = t->itemAt(index);
            	//t->updateItem(item, index, 0, parser->keyLen());
                memcpy(childItem, item, keyLen);
            } else {
            	childItem = t->itemAt(index);
            }
            //int id = innerParser->getValue(t->itemAt(index));
            int id;
            memcpy(&id, childItem + keyLen, 4);
            ItemList* c = nodeManager->getList(tid, id, layout);
            ret = insert(c, item);
            memcpy(childItem + keyLen + 4, &(c->header->size), 4);
            split(index, c, t, childItem);
            delete c;
            return ret;
        }
    }
    int rangeCount(bool flag, ItemList* t, uchar* a, uchar* b) {
    	t->markAccess();
		int ia, ib;
		cur = a;
		findItemIndex(t, ia);
		cur = b;
		bool fb = findItemIndex(t, ib);
		if (ia == -1) return 0;
		if (ib == -1) ib = t->keyNum();
		if (ia > ib) return 0;
    	if (t->isLeaf()) {
    		return (fb) ?ib - ia + 1: ib - ia;
    	} else {
    		uchar* item = t->itemAt(ia);
    		int id;
    		memcpy(&id, item + keyLen, 4);
    		ItemList* c;
    		int s = 0;
    		int is = ia;
    		if (!flag) {
    		    c = nodeManager->getList(tid, id, layout);
    		    s = rangeCount(flag, c, a, b);
    		    is = ia + 1;
    		    free(c);
    		}
    		for (int i = is; i < ib; ++ i) {
    			item = t->itemAt(i);
    			int w;
    			memcpy(&w, item + keyLen + 4, 4);
    			s += w;
    		}
    		if (ib > ia && ib < t->keyNum()) {
    		    item = t->itemAt(ib);
    		    memcpy(&id, item + keyLen, 4);
    		    c = nodeManager->getList(tid, id, layout);
    		    s += rangeCount(true, c, a, b);
    		    free(c);
    		}
    		return s;
    	}
    }
public:
    BPlusTree (
    	int typeID,
    	NodeManager* nm,
    	int kl, int il, cf* c,
    	int root, int s
    ) {
    	tid = typeID;
    	nodeManager = nm;
    	keyLen = kl;
    	rootID = root;
    	itemLen = il;
    	cmp = c;
    	layout.leafLayout = new ListLayout(itemLen);
    	layout.innerLayout = new ListLayout(keyLen + 8);
    	if (rootID < 0) {
    		ItemList* r = nm->newList(tid, true, layout.leafLayout);
    		rootID = r->id;
    		delete r;
    	}
    	cur = NULL;
    	sp = s;
    	cs = cp = 0;
    }
    void findValue(uchar* key, int& p, int& s) {
    	tmp = sp;
    	cp = 0;
    	cs = 0;
        ItemList* root = nodeManager->getList(tid, rootID, layout);
        //bpl->insert(typeID, )
        cur = key;
        find(root);
        free(root);
        p = cp;
        s = cs;
    }
    bool insertValue(uchar* item) {
    	tmp = sp;
    	cur = item;
        ItemList* root = nodeManager->getList(tid, rootID, layout);
        bool ret = insert(root, item);
        split(0, root, NULL, NULL);
        delete root;
        return ret;
    }
    bool eraseValue(uchar* key) {
    	tmp = sp;
    	cur = key;
    	cp = -1;
    	cs = -1;
    	ItemList* root = nodeManager->getList(tid, rootID, layout);
    	bool ret = find(root);
    	if (ret) {
    		enlarge(0, NULL, root, NULL);
    	}
    	delete root;
    	return ret;
    }
    ItemList* findLeft() {
    	tmp = sp;
    	ItemList* t = nodeManager->getList(tid, rootID, layout);
    	while (!t->isLeaf()) {
    		int k;
    		memcpy(&k, t->itemAt(0) + keyLen, 4);
    		free(t);
    		t = nodeManager->getList(tid, k, layout);
    	}
    	return t;
    }
    int count(uchar* a, uchar* b) {
    	tmp = sp;
    	ItemList* root = nodeManager->getList(tid, rootID, layout);
    	int k = rangeCount(false, root, a, b);
    	free(root);
    	return k;
    }
    int size() {
    	ItemList* root = nodeManager->getList(tid, rootID, layout);
    	int k = root->header->size;
    	free(root);
    	return k;
    }
    void judge(int id) {
    	ItemList* root = nodeManager->getList(tid, id, layout);
    	if (root->isLeaf()) {
    		if (root->header->itemNum != root->header->size) {
    			cout <<"error"<<endl;
    		}
    		free(root);
    		return;
    	}
    	int n = root->keyNum();
    	int s = 0;
    	for (int i = 0; i < n; ++ i) {
    		uchar* it = root->itemAt(i);
    		int k;
    		memcpy(&k, it + keyLen + 4, 4);
    		s += k;
    		int cid;
    		memcpy(&cid, it + keyLen, 4);
    		judge(cid);
    	}
    	if (root->header->size - s != 0) {
    		cout <<"error"<<endl;
    	}
    	free(root);
    }
    ~BPlusTree() {
    	delete layout.innerLayout;
    	delete layout.leafLayout;
    }
};

#endif /* BPLUSTREE_H_ */
