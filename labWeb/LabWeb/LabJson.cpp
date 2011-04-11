/*
 *  Variables.cpp
 *  ImageProcessing
 *
 *  Created by Nick Porcino on 2010 08/10.
 *
 */

#include "LabJson.h"
#include "ftoa.h"
#include "itoa.h"
#include "TextScanner.h"

#include <string>

namespace Lab {
	namespace Json {
		
		std::string jsonEscapedString(const std::string& s)
		{
			std::string ret;
			ret.reserve(s.size());
			for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
				char c = *i;
				switch(c) {
					case '"': ret += "\\\""; break;
					case '\\': ret += "\\\\"; break;
					case '/': ret += "\\/"; break;
					case 8: ret += "\\b"; break;
					case 9: ret += "\\t"; break;
					case 10: ret += "\\n"; break; 
					case 12: ret += "\\f"; break;
					case 13: ret += "\\r"; break;
					default: ret += c; break;
				}
			}
			return ret;
		}
		
		std::string Object::asJsonString() const
		{
			std::string ret = "{ ";
			for (std::map<std::string, ValuePtr>::const_iterator i = vars.begin(); i != vars.end(); ) {
				ret += "\"" + i->first + "\":" + i->second->asJsonString();
				++i;
				if (i != vars.end())
					ret += ", ";
			}
			return ret + " }";
		}
		
		std::string Array::asJsonString() const
		{
			std::string ret = "[ ";
			for (std::vector<ValuePtr>::const_iterator i = array.begin(); i != array.end(); ) {
				ret += (*i)->asJsonString();
				++i;
				if (i != array.end())
					ret += ", ";
			}
			return ret + " ]";
		}
		
		std::string Value::asJsonString() const
		{
			switch (_type)
			{
				case kInt:
				{
					char buff[FTOA_BUFFER_SIZE+1];
					itoa(*(boost::any_cast<int>(&_data)), buff, 10);
					return buff;
				}
					
				case kFloat:
				{
					char buff[FTOA_BUFFER_SIZE+1];
					ftoa(*(boost::any_cast<float>(&_data)), buff);
					return buff;
				}
					
				case kString:
				{
					std::string val = jsonEscapedString(*(boost::any_cast<std::string>(&_data)));
					return "\"" + val + "\"";
				}
					
				case kBool:
					return *(boost::any_cast<bool>(&_data)) ? "true" : "false";

				default:
				case kNone:
					return "No data";
			}
		}
		
		const char* Value::typeName() const
		{
			switch (_type)
			{
				case kNone:   return "none";
				case kInt:    return "int";
				case kFloat:  return "float";
				case kString: return "string";
				case kBool:   return "bool";
				case kArray:  return "array";
				case kObject: return "object";
			}
			return "unknown";
		}
		
		Object* parseJsonObject(const char*& curr, const char* end)
		{
			curr = tsSkipCommentsAndWhitespace(curr, end);
			if (*curr != '{') {
				return 0;
			}
			++curr;

			Object* v = new Object;
			while (curr < end) {
				curr = tsSkipCommentsAndWhitespace(curr, end);
				if (*curr != '"') {
					return v;
				}
				++curr;
				
				// find a string, followed by :, remove any spaces between name and colon
				const char* name;
				uint32_t nameLen;
				curr = tsGetToken(curr, end, '"', &name, &nameLen);
				if (curr == end) {
					delete v;
					return 0;
				}
				++curr;
				
				curr = tsSkipCommentsAndWhitespace(curr, end);
				if (*curr != ':') {
					delete v;
					return 0;
				}
				++curr;

				v->insert(name, nameLen, parseJsonValue(curr, end));
				curr = tsSkipCommentsAndWhitespace(curr, end);
				
				if (*curr == '}') {
					++curr;
					break;
				}
				
				if (*curr == ',') {
					++curr;
					continue;
				}
				
				delete v;
				return 0;
			}
			
			return v;
		}
		
		Array* parseJsonArray(const char*& curr, const char* end)
		{
			if (*curr != '[') {
				return 0;
			}
			
			++curr;
			Array* array = new Array();
			
			while (*curr != ']') {
				ValuePtr newValue = ValuePtr(parseJsonValue(curr, end));
				array->push_back(newValue);
				
				curr = tsSkipCommentsAndWhitespace(curr, end);
				if (*curr == ',') {
					++curr;
					continue;
				}
				
				if (*curr == ']') {
					++curr;
					return array;
				} // if ']'
			} // while
			return 0;
		}
		
		Value* parseJsonValue(const char*& curr, const char* end)
		{
			curr = tsSkipCommentsAndWhitespace(curr, end);
			switch (*curr) {
				case '{':
					// parse a json object and consume closing brace
					return parseJsonObject(curr, end);
					
				case '[':
					return parseJsonArray(curr, end);
					
				case 't': {
						char const* newCurr = tsExpect(curr, end, "true");
						if (newCurr == curr) {
							return 0;
						}
						return new Value(true);
					}
					
				case 'f': {
						char const* newCurr = tsExpect(curr, end, "false");
						if (newCurr == curr) {
							return 0;
						}
						return new Value(false);
					}
					
				case 'n': {
						char const* newCurr = tsExpect(curr, end, "null");
						if (newCurr == curr) {
							return 0;
						}
						Value* v = new Value;
						v->setNull();
						return v;
					}
					
				case '\"': {
						char const* str;
						uint32_t len;
						curr = tsGetString(curr, end, true, &str, &len);
						return new Value(str, len);
					}
					
				case '-':
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': {
						float result = 0;
						curr = tsGetFloat(curr, end, &result);
						return new Value(result);
					}
			}
			
			return 0;
		} // parseJsonValue

		float Value::asFloat() const
		{
			if (_type == kFloat || _type == kInt)
				return *(boost::any_cast<float>(&_data));
			
			if (_type == kString) {
				float f;
				const std::string& s = *(boost::any_cast<const std::string>(&_data));
				tsGetFloat(s.c_str(), s.c_str() + s.length(), + &f);
				return f;
			}
			
			return 0;
		}
		
		int Value::asInt() const
		{
			if (_type == kFloat || _type == kInt)
				return *(boost::any_cast<int>(&_data));
			
			if (_type == kString) {
				float f;
				const std::string& s = *(boost::any_cast<const std::string>(&_data));
				tsGetFloat(s.c_str(), s.c_str() + s.length(), + &f);
				return int(f);
			}
			
			return 0;
		}
		
		std::string Value::asString() const
		{
			if (_type == kString) {
				return *(boost::any_cast<const std::string>(&_data));
			}
			if (_type == kFloat) {
				char buff[12];
				ftoa(*(boost::any_cast<float>(&_data)), buff);
				return std::string(buff);
			}
			if (_type == kInt) {
				char buff[12];
				itoa(*(boost::any_cast<float>(&_data)), buff, 10); // radix 10
				return std::string(buff);
			}
			return "";
		}
		
		std::vector<ValuePtr>* Value::asArray() const {
			if (_type == kArray) {
				return *(boost::any_cast<std::vector<ValuePtr>*>(&_data));
			}
			return 0;
		}
		
	} // Json
}	// Lab

void testVariables() {
	
}
