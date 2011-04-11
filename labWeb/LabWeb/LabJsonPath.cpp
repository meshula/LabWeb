/*
 *  LabJsonPath.cpp
 *  ImageProcessing
 *
 *  Created by Nick Porcino on 2010 11/14.
 *
 */

#include "LabJsonPath.h"
#include "LabJson.h"
#include "TextScanner.h"

#include <iostream>

namespace Lab {
	namespace Json {
		
		void jsonPathR(ArrayPtr& result, const char* curr, const char* end, ValuePtr o)
		{
			if (curr[0] == '.') {
				if (o->typeId() == Value::kObject) {
					ObjectPtr a = boost::dynamic_pointer_cast<Object>(o);
					if (curr[1] == '*')
						result->push_back(o);
					else if (curr[1] == '.')
						for (ObjectConstIter i = a->begin(); i != a->end(); ++i)
							jsonPathR(result, curr + 1, end, i->second);
					else
						jsonPathR(result, curr + 1, end, o);
				}
				else if (o->typeId() == Value::kArray) {
					ArrayPtr a = boost::dynamic_pointer_cast<Array>(o);
					for (ArrayConstIter i = a->begin(); i != a->end(); ++i)
						jsonPathR(result, curr + 1, end, *i);
				}
			}
			else if (curr[0] == '*' || curr[0] == '\0') {
				result->push_back(o);
			}
			else if (curr[0] == '[') {
				if (o->typeId() == Value::kArray) {
					if (curr[1] == '*' && curr[2] == ']') {
						// if it was an array, and [*] was specified, recurse into the array loop at (A)
						jsonPathR(result, curr+3, end, o);
					}
					else {
						// slice parser
						ArrayPtr a = boost::dynamic_pointer_cast<Array>(o);
						int it[3] = {
							0, a->size() - 1, 1	// first, last, step
						};
						int in = 0;
		
						++curr;
						while (curr[0] != ']') {
							if (curr[0] == ':') {
								++curr;
								++in;
							}
							else {
								int32_t x;
								curr = tsGetInt32(curr, end, &x);
								it[in] = x > 0 ? x : a->size() + x - 1;
							}
						}
						++curr;
						if (in == 0)
							it[1] = it[0]; // special case, index only specified
						
						for (int i = it[0]; i < it[1]; i +=it[2]) {
							if (i < a->size())	// failsafe against bad expressions
								jsonPathR(result, curr, end, (*a)[i]);
						}
					}
				}
			}
			else {
				const char* tokenC = 0;
				uint32_t len = 0;
				curr = tsGetTokenAlphaNumeric(curr, end, &tokenC, &len);
				std::string token(tokenC, len);
				bool recur = curr < end;
				
				if (o->typeId() == Value::kObject) {
					ObjectPtr a = boost::dynamic_pointer_cast<Object>(o);
					for (ObjectConstIter i = a->begin(); i != a->end(); ++i) {
						std::string deb = i->first;
						if (token == i->first) {
							if (recur)
								jsonPathR(result, curr, end, i->second);
							else
								result->push_back(i->second);
						} 
						else if (i->second->typeId() == Value::kArray)
							jsonPathR(result, tokenC, end, i->second);
					}
				}
				else if	(o->typeId() == Value::kArray) {
					// array loop (A)
					ArrayPtr a = boost::dynamic_pointer_cast<Array>(o);
					for (ArrayConstIter i = a->begin(); i != a->end(); ++i) {
						jsonPathR(result, tokenC, end, *i);
					}
				}						
			}
		}
	
										 
		ValuePtr jsonPath(const char* expr, ValuePtr o)
		{
			ArrayPtr result(new Array());
			if (*expr == '$') {
				const char* curr = expr + 1;
				jsonPathR(result, curr, expr + strlen(expr), o);
			}
			return result;
		}


		void testPath()
		{
			const char* test =
"{ \"store\": {\
	\"book\": [ \
			 { \"category\": \"reference\",\
			 \"author\": \"Nigel Rees\",\
			 \"title\": \"Sayings of the Century\",\
			 \"price\": 8.95\
			 },\
			 { \"category\": \"fiction\",\
			 \"author\": \"Evelyn Waugh\",\
			 \"title\": \"Sword of Honour\",\
			 \"price\": 12.99\
			 },\
			 { \"category\": \"fiction\",\
			 \"author\": \"Herman Melville\",\
			 \"title\": \"Moby Dick\",\
			 \"isbn\": \"0-553-21311-3\",\
			 \"price\": 8.99\
			 },\
			 { \"category\": \"fiction\",\
			 \"author\": \"J. R. R. Tolkien\",\
			 \"title\": \"The Lord of the Rings\",\
			 \"isbn\": \"0-395-19395-8\",\
			 \"price\": 22.99\
			 }\
			 ],\
	\"bicycle\": {\
		\"color\": \"red\",\
		\"price\": 19.95\
	}\
  }\
}";

			ValuePtr v(parseJsonObject(test, test + strlen(test)));
			std::cout << v->asJsonString() << std::endl;
			std::cout << "-- SECOND BOOK         ----\n";
			std::cout << jsonPath("$.store.book[1]", v)->asJsonString() << std::endl;
			std::cout << "-- LAST BOOK           ----\n";
			std::cout << jsonPath("$.store.book[-1:]", v)->asJsonString() << std::endl;
			std::cout << "-- FIRST TWO BOOKS     ----\n";
			std::cout << jsonPath("$.store.book[:2]", v)->asJsonString() << std::endl;
			std::cout << "-- SECOND & THIRD BOOKS ---\n";
			std::cout << jsonPath("$.store.book[1:3]", v)->asJsonString() << std::endl;
			std::cout << "-- EVEN BOOKS            ---\n";
			std::cout << jsonPath("$.store.book[::2]", v)->asJsonString() << std::endl;
			std::cout << "-- ALL BOOK PRICES      ----\n";
			std::cout << jsonPath("$.store.book[*].price", v)->asJsonString() << std::endl;
			std::cout << "-- ALL AUTHORS          ----\n";
			std::cout << jsonPath("$..author", v)->asJsonString() << std::endl;
			std::cout << "-- ALL PRICES           ----\n";
			std::cout << jsonPath("$.store..price", v)->asJsonString() << std::endl;
			std::cout << "-- ALL BICYCLES         ----\n";
			std::cout << jsonPath("$.store.bicycle", v)->asJsonString() << std::endl;
			std::cout << "-- BICYCLE COLOR        ----\n";
			std::cout << jsonPath("$.store.bicycle.color", v)->asJsonString() << std::endl;
			std::cout << "-- ALL BOOKS            ----\n";
			std::cout << jsonPath("$.store.book", v)->asJsonString() << std::endl;
			std::cout << "-- ALL BOOK TITLES      ----\n";
			std::cout << jsonPath("$.store.book.title", v)->asJsonString() << std::endl;
			std::cout << "-- EVERYTHING IN THE STORE -\n";
			std::cout << jsonPath("$.store.*", v)->asJsonString() << std::endl;
			std::cout << "-- EVERYTHING --------------\n";
			std::cout << jsonPath("$..*", v)->asJsonString() << std::endl;
			std::cout << "----------------------------\n";
		}


	} // Json
} // Lab
