/*
 *  LabJson.h
 *  ImageProcessing
 *
 *  Created by Nick Porcino on 2010 08/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <string>
#include <map>
#include <vector>

namespace Lab {
	namespace Json {
		
		class Value;
		typedef boost::shared_ptr<Value> ValuePtr;
		
		// A Json value is a string, number, object, array, true, false, or null
		// This object detects when arrays are of numbers, and of size 2, 3, 4, or 16
		// and specializes those into float arrays.
		
		class Value
		{
		public:	
			
			Value() : _type(kNone), _data((void*) 0) { }
			Value(const Value& rhs) : _type(rhs._type), _data(rhs._data) { }
			Value(bool b) : _type(kBool), _data(b) { }
			Value(int i) : _type(kInt), _data(i) { }
			Value(float f) : _type(kFloat), _data(f) { }
			Value(char const*const s) : _type(kString), _data(std::string(s)) { }
			Value(char const*const s, int n) : _type(kString), _data(std::string(s, n)) { }
			Value(const std::string& s) : _type(kString), _data(s) { }
			
			enum Type
			{
				kNone, kNull, kInt, kFloat, kString, kBool, kObject, kArray
			};
			
			virtual Type typeId() const {
				return _type;
			}
			
			virtual const char* typeName() const;
			
			// In a subclass, new serialized types can be added; subclasses must
			// call their superclass' asJsonString to get more basic type serialization
			virtual std::string asJsonString() const;
			
			void setNull() { _data = 0; _type = kNull; } 
			void setInt(int v) { _data = v; _type = kInt; }
			void setFloat(float v) { _data = v; _type = kFloat; }
			void setBool(bool v) { _data = v; _type = kBool; }
			void setString(char const*const v) { _data = std::string(v); _type = kString; }
			void setString(char const*const v, int n) { _data = std::string(v, n); _type = kString; }
			void setString(const std::string& v) { _data = v; _type = kString; }

			float asFloat() const;
			int asInt() const;
			std::vector<ValuePtr>* asArray() const;
			std::string asString() const;
			
		protected:
			Type _type;
			boost::any _data;
		};
		
		typedef boost::shared_ptr<Value> ValuePtr;
		
		// A Json array is an ordered collection of values
		
		typedef std::vector<ValuePtr>::const_iterator ArrayConstIter;
		
		class Array : public Value
		{
		public:
			Array() {
				_type = kArray;
				_data = &array;
			}
			
			~Array()
			{
				for (std::vector<ValuePtr>::iterator i = array.begin(); i != array.end(); ++i) {
					(*i).reset();
				}
			}
			
			size_t size() {
				return array.size();
			}
			
			ValuePtr& operator[](int i) {
				return array[i];
			} 	
			
			void push_back(ValuePtr v) {
				array.push_back(v);
			}
			
			ArrayConstIter begin() const {
				return array.begin();
			}
			
			ArrayConstIter end() const {
				return array.end();
			}
			
			virtual std::string asJsonString() const;
			
		private:
			std::vector<ValuePtr> array;
		};
		
		typedef boost::shared_ptr<Array> ArrayPtr;
		
		// A Json object is an unordered set of name/value pairs
		
		typedef std::map<const std::string, ValuePtr>::iterator ObjectIter;
		typedef std::map<const std::string, ValuePtr>::const_iterator ObjectConstIter;
		
		class Object : public Value
		{
		public:
			Object() {
				_type = kObject;
				_data = &vars;
			}
			
			virtual ~Object() {
				clear();
			}
			
			void clear() {
				for (std::map<std::string, ValuePtr>::iterator i = vars.begin(); i != vars.end(); ++i) {
					i->second.reset();
				}
				vars.clear();
			}
			
			void insert(const char* name, int nameLen, Value* v) {
				vars[std::string(name, nameLen)] = ValuePtr(v);
			}
			
			ValuePtr& operator[](const std::string& key) {
				return vars[key];
			} 
			
			ObjectConstIter begin() const {
				return vars.begin();
			}
			
			ObjectConstIter end() const {
				return vars.end();
			}
			
			ObjectConstIter find(const std::string& s) const {
				return vars.find(s);
			}

			ObjectIter begin() {
				return vars.begin();
			}
			
			ObjectIter end() {
				return vars.end();
			}

			ObjectIter find(const std::string& s) {
				return vars.find(s);
			}
			
			virtual std::string asJsonString() const;
			
		private:
			std::map<const std::string, ValuePtr> vars;
		};
		
		typedef boost::shared_ptr<Object> ObjectPtr;
		
		Value* parseJsonValue(const char*& curr, const char* end);
		Object* parseJsonObject(const char*& curr, const char* end);
		Array* parseJsonArray(const char*& curr, const char* end);
		
	}} // Lab::Json

void testVariables();

