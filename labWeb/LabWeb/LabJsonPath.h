/*
 *  LabJsonPath.h
 *  ImageProcessing
 *
 *  Created by Nick Porcino on 2010 11/14.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "LabJson.h"

// Implements a subset of http://goessner.net/articles/JsonPath/
// See the testPath function for examples that work

// Not implemented: [,] union operator, eg [1, 3, 4]
// Not implemented: predicator, eg $..book[?(@.isbn)] filters books with an isbn field
// Not impelementd: expression filter, $..book[?(@.price<10)] books costing less than 10
// Not implemented: expression: $..book[(@.length-1)] last book

namespace Lab {
	namespace Json {

		ValuePtr jsonPath(const char*& expr, ValuePtr o);
		
		void testPath();
		
	}
}


