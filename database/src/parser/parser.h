/*
 * parser.h
 *
 *  Created on: 2014年12月20日
 *      Author: lql
 */

#ifndef PARSER__H_
#define PARSER__H_

#define START 0
#define INSERT 1
#define CREATE_TB 2
#define EOCTB 3
#include <iostream>
using namespace std;
char* getWord(char*& s, char e) {
	while (*s == ' ') ++ s;
	if (*s == '\0') {
		return NULL;
	}
	char* t = s;
	while (*t != e) ++ t;
	char* k = t - 1;
	while (*k == ' ') --k;
	*(k + 1) = '\0';
	return t;
}
bool isCmp(char k) {
	return (k == '>' || k == '<' || k == '=');
}
bool isCha(char k) {
	return (('a' <= k && k <= 'z') || ('A' <= k && k <= 'Z'));
}
bool isDig(char k) {
	return '0' <= k && k <= '9';
}
#endif /* PARSER_H_ */
