//============================================================================
// Name        : database.cpp
// Author      : lql
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "database.h"
#include "exec_select.h"
#include "exec_insert.h"
#include "exec_delete.h"
#include "exec_update.h"
using namespace std;

void execute(char* sql) {
	int len = strlen(sql);
	char* prev, *next;
	prev = sql;
	switch (state) {
	case START:
	    next = getWord(prev, ' ');
	    if (strcmp(prev, "SHOW") == 0) {
	    	if (cdbs == NULL) {
	    		printf("error: no db\n");
	    		return;
	    	}
	    	cdbs->printtb();
	    	return;
	    }
	    if (strcmp(prev, "DESC") == 0) {
	    	if (cdbs == NULL) {
	    		printf("error: no db\n");
	    		return;
	    	}
	    	prev = next + 1;
	    	next = getWord(prev, ';');
	    	Table* tb = cdbs->getTable(prev);
	    	if (tb == NULL) {
	    		printf("error: no table\n");
	    		return;
	    	}
	    	int kk = tb->cn;
	    	for (int i = 0; i < kk; ++ i) {
	    		printf("name:%s\t", tb->col[i].name);
	    		printf("len:%d\t", tb->col[i].cb.cl);
	    		int kkk = tb->col[i].cb.dt;
	    		if (kkk == LL_TYPE) {
	    			printf("type: integer\n");
	    		}
	    		if (kkk == DB_TYPE) {
	    			printf("type: float\n");
	    		}
	    		if (kkk == ST_TYPE) {
	    			printf("type: string\n");
	    		}
	    	}
	    	return;
	    }
	    if (strcmp(prev, "INSERT") == 0) {
	    	prev = next + 1;
	    	next = getWord(prev, ' ');
	    	if (strcmp(prev, "INTO") == 0) {
	    		prev = next + 1;
	    		next = getWord(prev, ' ');
	    	}
	    	ctb = NULL;
	    	if (cdbs != NULL) ctb = cdbs->getTable(prev);
	    	next ++;
	    	while (*next != '(' && *next != '\0') ++ next;
	    	state = INSERT;
	    	if (ctb == NULL) {
	    		printf("error: no table\n");
	    	}
	    	if (*next != '\0') exec_insert(next);
	    } else if (strcmp(prev, "SELECT") == 0) {
	    	exec_select(next + 1);
	    } else if (strcmp(prev, "DELETE") == 0) {
	    	exec_delete(next + 1);
	    } else if (strcmp(prev, "UPDATE") == 0) {
	    	exec_update(next + 1);
	    } else if (strcmp(prev, "CREATE") == 0) {
	    	prev = next + 1;
	    	next = getWord(prev, ' ');
	    	if (strcmp(prev, "INDEX") == 0) {
	    		if (cdbs == NULL) {
	    			printf("error:no db\n");
	    			return;
	    		}
	    		prev = next + 1;
	    		next = getWord(prev, '(');
	    		Table* tb = cdbs->getTable(prev);
	    		if (tb == NULL) {
	    			printf("error:no table\n");
	    			return;
	    		}
	    		prev = next + 1;
	    		next = getWord(prev, ')');
	    		if (!tb->createIndex(prev)) {
	    			printf("error:in creating index\n");
	    			return;
	    		}
	    	} else if (strcmp(prev, "TABLE") == 0) {
	    		prev = next + 1;
	    		next = getWord(prev, '(');
	    		create.nl = strlen(prev) + 1;
	    		create.tn = new char[create.nl];
	    		strcpy(create.tn, prev);
	    		state = CREATE_TB;
	    		create.n = 0;
	    		primary = -1;
	    		return;
	    	} else if (strcmp(prev, "DATABASE") == 0) {
	    		prev = next + 1;
	    		next = getWord(prev, ';');
	    		if (!createdb(prev)) printf("error4");
	    		return;
	    	}
	    } else if (strcmp(prev, "USE") == 0) {
	    	prev = next + 1;
	    	next = getWord(prev, ';');
	    	if (!usedb(prev)) {
	    		printf("error\n");
	    		return;
	    	}
	    } else if (strcmp(prev, "DROP") == 0) {
	    	prev = next + 1;
	    	next = getWord(prev, ' ');
	    	if (strcmp(prev, "TABLE") == 0) {
	    		prev = next + 1;
	    		next = getWord(prev, ';');
	    		if (cdbs == NULL || !cdbs->dropTB(prev)) {
	    			printf("error\n");
	    		}
	    	} else if (strcmp(prev, "INDEX") == 0) {
	    		if (cdbs == NULL){
	    			printf("error: no db\n");
	    			return;
	    		}
	    		prev = next + 1;
	    		next = getWord(prev, '(');
	    		Table* tb = cdbs->getTable(prev);
	    		if (tb == NULL) {
	    			printf("error: no table\n");
	    			return;
	    		}
	    		prev = next + 1;
	    		next = getWord(prev, ')');
	    		if (!tb->dropIndex(prev)) {
	    			printf("error: in droping index\n");
	    			return;
	    		}
	    	} else {
	    		prev = next + 1;
	    		next = getWord(prev, ';');
	    		if (!dropdb(prev)) {
	    			printf("error\n");
	    		}
	    	}
	    }
		break;
	case CREATE_TB:
		next = getWord(prev, ' ');
		if (strcmp(prev, "PRIMARY") == 0) {
			prev = next + 1;
			next = getWord(prev, '(');
			prev = next + 1;
			next = getWord(prev, ')');
			int n = create.n, i;
			for (i = 0; i < n; ++ i) if (strcmp(create.name[i], prev) == 0) break;
			create.c[i].ut = UNI;
			create.c[i].nt = UN;
			primary = i;
		} else if (strcmp(prev, "UNIQUE") == 0) {
			prev = next + 1;
			next = getWord(prev, '(');
			prev = next + 1;
			next = getWord(prev, ')');
			int n = create.n, i;
			for (i = 0; i < n; ++ i) if (strcmp(create.name[i], prev) == 0) break;
			create.c[i].ut = UNI;
		} else {
			int n = create.n;
			create.c[n].nl = strlen(prev) + 1;
			create.name[n] = new char[create.c[n].nl];
			strcpy(create.name[n], prev);
			prev = next + 1;
			next = getWord(prev, '(');
			create.c[n].nt = N;
			create.c[n].ut = UNUNI;
			switch (*prev) {
			case 'v':
			case 'c':
				create.c[n].dt = ST_TYPE;
				prev = next + 1;
				next = getWord(prev, ')');
				create.c[n].cl = atoi(prev) + 1;
				break;
			case 'i':
				create.c[n].dt = LL_TYPE;
				prev = next + 1;
				next = getWord(prev, ')');
				create.c[n].cl = 8;
				break;
			default:
				create.c[n].dt = DB_TYPE;
				prev = next + 1;
				next = getWord(prev, ')');
				create.c[n].cl = 8;
			}
			next ++;
			if (*next == ' ') {
				//create.c[n].cl += 8;
				create.c[n].nt = UN;
			}
			create.n ++;
		}
		if (sql[len - 1] != ',') {
			state = EOCTB;
		}
		break;
	case EOCTB:
		state = START;
		if (cdbs == NULL) {
			printf("error\n");
			return;
		}
		if (cdbs->createTB(&create)) {
			Table* tb = cdbs->getTable(create.tn);
			if (primary == -1) primary = 0;
			tb->createIndex(create.name[primary]);
		} else {
			printf("error\n");
		}
		break;
	case INSERT:
		exec_insert(sql);
		break;
	default: break;
	}
}
void loadSql(const char* name) {
	fin = fopen(name, "r");
	char* str = fgets(buf, 1000, fin);
	state = START;
	while (str != NULL) {
		int len = strlen(buf);
		while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == 13)) {
			buf[len - 1] = '\0';
			len --;
		}
		if (len != 0) {
		    execute(buf);
		}
		str = fgets(buf, 1000, fin);
	}
	fclose(fin);
}
int main() {
	init();
	fm = new FileManager();
	bpm = new BufPageManager(fm);
	bpl = new MyLinkList(CAP, MAX_TYPE_NUM);
	printf("clear all? y/n\n");
	char ch;
	scanf("%c", &ch);
	if (ch == 'y') {
	    clearAll();
	    load();
	} else {
		load();
	}
	char fname[100];
	while (true) {
		printf(">>");
		scanf("%s", fname);
		if (strcmp(fname, "quit") == 0) break;
		else loadSql(fname);
	}
	//loadSql("test.lql");
	//loadSql("book.lql");
	//loadSql("orders.lql");
	//loadSql("select.lql");
	//loadSql("join.lql");
	//loadSql("delete.lql");
	//loadSql("join.lql");
	//loadSql("update.lql");
	printf("bye\n");
	shutdown();
	return 0;
}
