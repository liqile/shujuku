/*
 * ItemList.h
 *
 *  Created on: 2014年11月12日
 *      Author: lql
 */

#ifndef ITEM_LIST_H_
#define ITEM_LIST_H_
#include "../utils/MyBitMap.h"
#include "../recordmanager/layout.h"
#include "../bufmanager/BufPageManager.h"
#include <string.h>
#include <cstdlib>
//#include "../bufmanager/BufPageManager.h"
struct ItemList {
public:
	BufType data;
	ListLayout* layout;
	ListHeader* header;
	short* biasArray;
	uchar* itemArray;
	uint* bitMapArray;
	int bufIndex;
	BufPageManager* bpm;
	int id;
public:
	void changeSlot(int index) {
		int k = (index >> 5);
		uint i = (1 << (/*index & 0x0000001f*/ index % 32));
		bitMapArray[k] ^= i;
	}
	short getSlot() {
		for (int i = 0; i < layout->mapIntNum; ++ i) {
			if (bitMapArray[i] != 0) {
				uint k = MyBitMap::lowbit(bitMapArray[i]);
				bitMapArray[i] ^= k;
//				cout << "the slot " << i << " " << MyBitMap::getIndex(k) << endl;
				return (i << 5) + MyBitMap::getIndex(k);
			}
		}
		return 0;
	}
public:
	ItemList (
	    BufType b, ListLayout* listLayout,
	    BufPageManager* bpManager, int bufPageIndex,
	    int nodeID
	) {
		id = nodeID;
		data = b;
		header = (ListHeader*) b;
		layout = listLayout;
		biasArray = layout->getBiasArray(b);
		bitMapArray = layout->getBitMapArray(b);
		itemArray = layout->getItemArray(b);
		bpm = bpManager;
		bufIndex = bufPageIndex;
	}
	int keyNum() {
		return header->itemNum;
	}
	int maxKeyNum() {
		return layout->itemCapacity;
	}
	uchar* itemAt(int index) {
		if (layout->longItem) {
			short p = biasArray[index];
			//cout <<"slot "<< p << " " << index << endl;
//			checkSlot("find bias Array");
			return itemArray + p * layout->itemLen;
		} else {
			return itemArray + index * layout->itemLen;
		}
	}/*
	uchar* itemAtSlot(int slot) {
		return itemArray + slot * layout->itemLen;
	}*/
	void updateItem(const uchar* buf, int index, int start, int len) {
		uchar* b = itemAt(index);
		memcpy(b + start, buf, len);
	}
	void eraseAt(int index) {
		if (layout->longItem) {
			changeSlot(biasArray[index]);
			if (index == header->itemNum) {
				header->itemNum --;
				return;
			}
			int num = header->itemNum - index - 1;
			memmove((uchar*)&biasArray[index], (uchar*)&biasArray[index + 1], (num << 1));
			header->itemNum --;
		} else {
			if (index == header->itemNum) {
				header->itemNum--;
				return;
			}
			int num = header->itemNum - index - 1;
			memmove(itemAt(index), itemAt(index + 1), num * layout->itemLen);
			header->itemNum --;
		}
	}
	uchar* insertBefore(int index, uchar* item = NULL) {
		if (layout->longItem) {
			short slot = getSlot();
			if (index == header->itemNum) {
				biasArray[header->itemNum] = slot;
				header->itemNum ++;
			} else {
				int num = header->itemNum - index;
				memmove((uchar*)&biasArray[index + 1], (uchar*)&biasArray[index], (num << 1));
				biasArray[index] = slot;
				header->itemNum ++;
			}
			uchar* it = itemAt(index);
			if (item != NULL) {
			    memcpy(it, item, layout->itemLen);
			}
			return it;
		} else {
			uchar* it = itemAt(index);
			if (index == header->itemNum) {
				if (item != NULL) {
				    memcpy(it, item, layout->itemLen);
				}
				header->itemNum ++;
				return it;
			} else {
			    int num = header->itemNum - index;
			    memmove(itemAt(index + 1), itemAt(index), num * layout->itemLen);
			    if (item != NULL) {
			        memcpy(it, item, layout->itemLen);
			    }
			    header->itemNum ++;
			    return it;
			}
		}
	}
	void fill(int num) {
		header->itemNum = num;
		if (layout->longItem) {
		    int k = (num >> 5);
		    for (int i = 0; i < k; ++ i) {
    			bitMapArray[i] = 0;
		    }
		    uint j = (num & 0x0000001f);
		    if (j > 0) {
    			bitMapArray[k] = (0xffffffff ^ ((1 << j) - 1));
		    }
		    for (short i = 0; i < num; ++ i) {
    			biasArray[i] = i;
		    }
		}
	}
	void updateItemAtSlot(int i, uchar* item) {
		uchar* k = itemArray + i * layout->itemLen;
		memcpy(k, item, layout->itemLen);
	}
	void splitToRight(ItemList* a, int reserve) {
		a->header->next = header->next;
		header->next = a->id;
		if (layout->longItem) {
			int num = header->itemNum - reserve;
			a->fill(num);
			for (int i = reserve; i < header->itemNum; ++ i) {
				int j = i - reserve;
				a->updateItemAtSlot(j, itemAt(i));
				int slot = biasArray[i];
				changeSlot(slot);
			}
		} else {
			int num = header->itemNum - reserve;
			a->fill(num);
			uchar* ia = a->itemArray;
			memcpy(ia, itemAt(reserve), num * layout->itemLen);
		}
		header->itemNum = reserve;
	}
	void updateSize() {
		if (header->listType == LEAF_TYPE) {
			header->size = header->itemNum;
			return;
		}
		header->size = 0;
		for (int i = 0; i < header->itemNum; ++ i) {
			int k;
			memcpy(&k, itemAt(i) + layout->itemLen - 4, 4);
			header->size += k;
		}
	}
	void mergeFromRight(ItemList* a) {
		header->next = a->header->next;
		if (layout->longItem) {
			for (int i = 0; i < a->keyNum(); ++ i) {
				insertBefore(keyNum(), a->itemAt(i));
			}
		} else {
			int k = header->itemNum;
			memcpy(itemAt(k), a->itemArray, a->keyNum() * layout->itemLen);
			header->itemNum += a->keyNum();
		}
	}
	void markAccess() {
		bpm->access(bufIndex);
	}
	void markDirty() {
		bpm->markDirty(bufIndex);
	}
	bool isLeaf() {
		return (header->listType == LEAF_TYPE);
	}
	int capacity() {
		return layout->itemCapacity;
	}
#if 0
	void checkSlot(string s) {
		for (int i = 0; i < header->itemNum; ++ i) {
			if (biasArray[i] < 0) {
				cout <<s << " " << "find < 0"<<endl;
				exit(0);
			}
		}
	}
#endif
};

#endif /* RECORDMANAGER_H_ */
