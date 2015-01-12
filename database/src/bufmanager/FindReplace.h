#ifndef BUF_SEARCH
#define BUF_SEARCH
#include "../utils/MyLinkList.h"
#include "../utils/MyHashMap.h"
#include "../utils/pagedef.h"
//template <int CAP_>
class FindReplace {
private:
	MyLinkList* list;
	int CAP_;
public:
	void free(int index) {
		list->insertFirst(0, index);
	}
	void access(int index) {
		list->insert(0, index);
	}
	int find() {
		int index = list->getFirst(0);
		list->del(index);
		list->insert(0, index);
		return index;
	}
	FindReplace(int c) {
		CAP_ = c;
		list = new MyLinkList(c, 1);
		for (int i = 0; i < CAP_; ++ i) {
			list->insert(0, i);
		}
	}
};
#endif
