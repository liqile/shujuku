/*
 * layout.h
 *
 *  Created on: 2014年11月12日
 *      Author: lql
 */

#ifndef LAYOUT_H_
#define LAYOUT_H_
#define LEAF_TYPE 0
#define INNER_TYPE 1
#include "../utils/pagedef.h"
#include <iostream>
using namespace std;
#define SIMPLE_ITEM_LEN 8
struct ListHeader {
	int itemNum;
	int listType;
	int next;
	int size;
};
int up(int k, int d) {
	int c = (k >> d);
	if (k != (c << d)) {
		c ++;
	}
	return c;
}
struct ListLayout {
	int itemLen;
	int itemCapacity;
	bool longItem;
	int bitMapOffset;
	int mapIntNum;
	int biasArrayOffset;
	int itemOffset;
	ListLayout(int len) {
		itemLen = len;
		if (itemLen > SIMPLE_ITEM_LEN) {
			initLongItem();
		} else {
			initShortItem();
		}
	}
	void initLongItem() {
		/* x = itemCapacity + 1
		 * upper(x * itemLen / 4) + upper(x * 2 / 4) + upper(x / 32)
		 * + upper(sizeof(ListHeader) / 4) <= PAGE_INT_NUM
		 */
		longItem = true;
		int x = PAGE_SIZE / itemLen;
		while (true) {
			int a = up(x * itemLen, 2);
			int b = up(x, 1);
			int c = up(x, 5);
			int d = up(sizeof(ListHeader), 2);
			if (a + b + c + d <= PAGE_INT_NUM) {
				break;
			}
			x --;
		}
		itemCapacity = x - 1;
		bitMapOffset = up(sizeof(ListHeader), 2);
		mapIntNum = up(x, 5);
		itemOffset = bitMapOffset + mapIntNum;
		int itemIntNum = up(x * itemLen, 2);
		biasArrayOffset = itemOffset + itemIntNum;
	}
	void initShortItem() {
		longItem = false;
		biasArrayOffset = 0;
		bitMapOffset = 0;
		itemOffset = up(sizeof(ListHeader), 2);
		int x = (PAGE_SIZE - sizeof(ListHeader)) / itemLen;
		itemCapacity = x - 1;
	}
	short* getBiasArray(BufType data) {
		return (short*)&data[this->biasArrayOffset];
	}
	uchar* getItemArray(BufType data) {
		return (uchar*)&data[this->itemOffset];
	}
	BufType getBitMapArray(BufType data) {
		return &data[this->bitMapOffset];
	}
	void clear(BufType b, bool isLeaf) {
		ListHeader* header = (ListHeader*) b;
		BufType bitMapArray = getBitMapArray(b);
		header->itemNum = 0;
		header->listType = isLeaf ? LEAF_TYPE : INNER_TYPE;
		header->next = -1;
		if (this->longItem) {
			for (int i = 0; i < this->mapIntNum; ++ i) {
				bitMapArray[i] = 0xffffffff;
			}
		}
	}
};
#endif /* LAYOUT_H_ */
