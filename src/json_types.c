/* Copyright (C) 2015-2016 Chase
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSON_TYPES_C
#define JSON_TYPES_C


#define JSON_TOP_LVL 1

#include "json_types.h"
#include "json_parser.h"

#include <stdlib.h>
#include <string.h>


const size_t JSON_OBJ_INIT_SIZE = 8;
const double JSON_OBJ_INCR_SIZE = 1.5;

const size_t JSON_ARRAY_INIT_SIZE = 8;
const double JSON_ARRAY_INCR_SIZE = 1.5;

const size_t JSON_ALIGN_SIZE = 8;

#ifdef __cplusplus
extern "C" {
#endif	//#ifdef __cplusplus


static void* json_allocator_alloc(size_t);
static void json_allocator_free(void*);
static void json_allocator_free_noop(void*);


const char* const JSON_VALUE_NAMES[] = {
	"unspecified",
	"string",
	"number",
	"object",
	"array",
	"true",
	"false",
	"null"
};

const char* const JSON_PARSER_STATE_NAMES[] = {
	"init",
	"",
	"complete",
	"",
	"error"
};

const char JSON_TOKEN_NAMES[] = {
	'"',
	'+',
	'-',
	'{',
	'}',
	'[',
	']',
	':',
	',',
	'\\'
};

const char* const JSON_EMPTY_STRING = "";


size_t align_offset(size_t offset, size_t align) {
	size_t rem = offset % align;
	if (!rem) {
		return offset;
	}
	return offset + align - rem;
}


int json_types_init(alloc_function allocFunction, free_function freeFunction, json_allocator** jsAllocOut, json_factory** jsFactoryOut) {
	//If both alloc and free functions passed by user, use those internally
	//If only alloc function passed, assume user will take care of freeing memory and use free_noop internally
	//If only free function or no alloc and free function passed, use defaults (malloc and free)
	int ret = 1;
	json_allocator* jsonAlloc = NULL;
	json_factory* jsonFact = NULL;
	if (allocFunction) {
		jsonAlloc = (json_allocator*) allocFunction(sizeof(json_allocator));
		if (!jsonAlloc) {
			return ret;
		}
		jsonFact = (json_factory*) allocFunction(sizeof(json_factory));
		if (!jsonFact) {
			return ret;
		}
		jsonAlloc->malloc = allocFunction;
		jsonFact->allocator = jsonAlloc;
		if (freeFunction) {
			jsonAlloc->free = freeFunction;
		} else {
			jsonAlloc->free = json_allocator_free_noop;
		}
	} else {
		jsonAlloc = (json_allocator*) json_allocator_alloc(sizeof(json_allocator));
		if (!jsonAlloc) {
			return ret;
		}
		jsonFact = (json_factory*) json_allocator_alloc(sizeof(json_factory));
		if (!jsonFact) {
			json_allocator_free(jsonAlloc);
			return ret;
		}
		jsonAlloc->free = json_allocator_free;
		jsonAlloc->malloc = json_allocator_alloc;
		jsonFact->allocator = jsonAlloc;
	}

	jsonFact->new_json_object = json_factory_new_json_object;
	jsonFact->new_json_value = json_factory_new_json_value;
	jsonFact->new_json_string = json_factory_new_json_string;
	jsonFact->new_json_number = json_factory_new_json_number;
	jsonFact->new_json_array = json_factory_new_json_array;
	jsonFact->new_json_true = json_factory_new_json_true;
	jsonFact->new_json_false = json_factory_new_json_false;
	jsonFact->new_json_null = json_factory_new_json_null;

	*jsAllocOut = jsonAlloc;
	*jsFactoryOut = jsonFact;

	ret = 0;
	return ret;
}

/* Default JSON Alloc and Free functions */

static void* json_allocator_alloc(size_t size) {
	return malloc(size);
}
static void json_allocator_free(void* ptr) {
	return free(ptr);
}
static void json_allocator_free_noop(void* ptr) {
	//Do Nothing
}

/* JSON Factory functions */

json_object* json_factory_new_json_object(json_factory* jsonFact, json_value* objParentValue) {
	json_object* obj = (json_object*) jsonFact->allocator->malloc( sizeof(json_object) );
	if (!obj) {
		return NULL;
	}

	obj->names = NULL;
	obj->values = NULL;
	obj->size = 0;
	obj->capacity = 0;
	obj->parentValue = objParentValue;

	return obj;
}
json_value* json_factory_new_json_value(json_factory* jsonFact, JSON_VALUE valValueType, void* valValue, JSON_VALUE valParentValueType, void* valParentValue) {
	json_value* value = (json_value*) jsonFact->allocator->malloc( sizeof(json_value) );
	if (!value) {
		return NULL;
	}

	value->valueType = valValueType;
	value->value = valValue;

	value->parentValueType = valParentValueType;
	value->parentValue = valParentValue;

	return value;
}
json_string* json_factory_new_json_string(json_factory* jsonFact, const char* strValue, size_t strValueLen, json_value* strParentValue) {
	json_string* str = (json_string*) jsonFact->allocator->malloc( sizeof(json_string) );
	if (!str) {
		return NULL;
	}

	str->value = strValue;
	str->valueLen = strValueLen;
	str->parentValue = strParentValue;

	return str;
}
json_number* json_factory_new_json_number(json_factory* jsonFact, double numValue, json_value* numParentValue) {
	json_number* num = (json_number*) jsonFact->allocator->malloc( sizeof(json_number) );
	if (!num) {
		return NULL;
	}

	num->value = numValue;
	num->parentValue = numParentValue;

	return num;
}
json_array* json_factory_new_json_array(json_factory* jsonFact, json_value* arrParentValue) {
	json_array* arr = (json_array*) jsonFact->allocator->malloc( sizeof(json_array) );
	if (!arr) {
		return NULL;
	}

	arr->values = NULL;
	arr->size = 0;
	arr->capacity = 0;
	arr->parentValue = arrParentValue;

	return arr;
}
json_true* json_factory_new_json_true(json_factory* jsonFact, json_value* truParentValue) {
	json_true* tru = (json_true*) jsonFact->allocator->malloc( sizeof(json_true) );
	if (!tru) {
		return NULL;
	}

	tru->parentValue = truParentValue;

	return tru;
}
json_false* json_factory_new_json_false(json_factory* jsonFact, json_value* falParentValue) {
	json_false* fal = (json_false*) jsonFact->allocator->malloc( sizeof(json_false) );
	if (!fal) {
		return NULL;
	}

	fal->parentValue = falParentValue;

	return fal;
}
json_null* json_factory_new_json_null(json_factory* jsonFact, json_value* nulParentValue) {
	json_null* nul = (json_null*) jsonFact->allocator->malloc( sizeof(json_null) );
	if (!nul) {
		return NULL;
	}

	nul->parentValue = nulParentValue;

	return nul;
}

/* JSON Value manipulation functions */

//Increase size of object; realloc if necessary
//Returns zero on success, nonzero on error
int json_object_resize(json_factory* jsonFact, json_object* obj, const size_t newSize) {
	int retVal = 1;

	if (!jsonFact || !obj || newSize < obj->size) {
		return retVal;
	}

	if (!obj->capacity) {//Uninitialized
		const size_t size = (newSize <= JSON_OBJ_INIT_SIZE) ? JSON_OBJ_INIT_SIZE : align_offset(newSize, JSON_ALIGN_SIZE);
		obj->names = (json_string**) jsonFact->allocator->malloc( sizeof(json_string*) * size );
		if (!obj->names) {
			return retVal;
		}
		obj->values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * size );
		if (!obj->values) {
			return retVal;
		}
		obj->capacity = size;
	} else if (obj->size < obj->capacity) {
		//There is enough space
	} else {//Realloc
		const size_t sizeIncr = align_offset(obj->capacity * JSON_OBJ_INCR_SIZE, JSON_ALIGN_SIZE);
		const size_t size = (newSize <= sizeIncr) ? sizeIncr : align_offset(newSize, JSON_ALIGN_SIZE);
		json_string** names = (json_string**) jsonFact->allocator->malloc( sizeof(json_string*) * size );
		if (!names) {
			return retVal;
		}
		json_value** values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * size );
		if (!values) {
			jsonFact->allocator->free(names);
			return retVal;
		}
		memcpy(names, obj->names, sizeof(obj->names) * obj->capacity);
		memcpy(values, obj->values, sizeof(obj->values) * obj->capacity);
		jsonFact->allocator->free(obj->names);
		jsonFact->allocator->free(obj->values);
		obj->names = names;
		obj->values = values;
		obj->capacity = size;
	}

	retVal = 0;
	return retVal;
}

//Returns zero on success, nonzero on error
int json_object_add_pair(json_factory* jsonFact, json_object* obj, json_string* name, json_value* value) {
	int retVal = 1;

	if (!jsonFact || !obj || !name || !value) {
		return retVal;
	}

	const size_t size = obj->size;
	retVal = json_object_resize(jsonFact, obj, size + 1);
	if (retVal) {
		return retVal;
	}

	obj->names[size] = name;
	obj->values[size] = value;
	obj->size += 1;

	retVal = 0;
	return retVal;
}

//Increase size of array; realloc if necessary
//Returns zero on success, nonzero on error
int json_array_resize(json_factory* jsonFact, json_array* arr, const size_t newSize) {
	int retVal = 1;

	if (!jsonFact || !arr || newSize < arr->size) {
		return retVal;
	} else if (arr->size == newSize) {
		retVal = 0;
		return retVal;
	}

	if (!arr->capacity) {//Uninitialized
		const size_t size = (newSize <= JSON_ARRAY_INIT_SIZE) ? JSON_ARRAY_INIT_SIZE : align_offset(newSize, JSON_ALIGN_SIZE);
		arr->values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * size );
		if (!arr->values) {
			return retVal;
		}
		arr->capacity = size;
	} else if (newSize < arr->capacity ) {//Size < Capacity
		//There is enough space
	} else {//Realloc
		const size_t sizeIncr = align_offset(arr->capacity * JSON_ARRAY_INCR_SIZE, JSON_ALIGN_SIZE);
		const size_t size = (newSize <= sizeIncr) ? sizeIncr : align_offset(newSize, JSON_ALIGN_SIZE);
		json_value** values = (json_value**) jsonFact->allocator->malloc( sizeof(json_value*) * size );
		if (!values) {
			return retVal;
		}

		memcpy(values, arr->values, sizeof(arr->values) * arr->capacity);
		jsonFact->allocator->free(arr->values);
		arr->values = values;
		arr->capacity = size;
	}

	retVal = 0;
	return retVal;
}

//Returns zero on success, nonzero on error
int json_array_add_element(json_factory* jsonFact, json_array* arr, json_value* value) {
	int retVal = 1;

	if (!jsonFact || !arr || !value) {
		return retVal;
	}

	const size_t size = arr->size;
	retVal = json_array_resize(jsonFact, arr, size + 1);
	if (retVal) {
		return retVal;
	}

	arr->values[size] = value;
	arr->size += 1;

	retVal = 0;
	return retVal;
}

//Convenience function to get a string representing the value
const char* json_value_get_type(json_value* value) {
	const char* str = JSON_EMPTY_STRING;

	if (!value) {
		return str;
	}

	JSON_VALUE valueType = value->valueType;

	if (valueType < unspecified_value || valueType >= JSON_VALUE_COUNT) {
		return str;
	}

	return JSON_VALUE_NAMES[valueType];
}

/*
 * Functions to free the structures allocated during runtime
 * Return zero on success, nonzero on failure
 */

int json_visitor_free_all(json_parser_state* parserState, json_value* topVal) {
	int ret = 1;
	if (!parserState || !topVal) {
		return ret;
	}

	return json_visitor_free_value(parserState->JSON_Factory, topVal);
}

int json_visitor_free_value(json_factory* jsonFact, json_value* value) {
	int ret = (value) ? 0 : 1;
	if (ret) {
		return ret;
	}

	switch (value->valueType) {
		default:
		case unspecified_value:
			ret = 1;
		break;
		case string_value:
			ret = json_visitor_free_string(jsonFact, value->value);
		break;
		case number_value:
			ret = json_visitor_free_number(jsonFact, value->value);
		break;
		case object_value:
			ret = json_visitor_free_object(jsonFact, value->value);
		break;
		case array_value:
			ret = json_visitor_free_array(jsonFact, value->value);
		break;
		case true_value:
			ret = json_visitor_free_true(jsonFact, value->value);
		break;
		case false_value:
			ret = json_visitor_free_false(jsonFact, value->value);
		break;
		case null_value:
			ret = json_visitor_free_null(jsonFact, value->value);
		break;
	}

	if (!ret) {
		jsonFact->allocator->free(value);
	}

	return ret;
}
int json_visitor_free_object(json_factory* jsonFact, json_object* obj) {
	int ret = (!obj || (obj->size > 0 && (!obj->names || !obj->values))) ? 1 : 0;

	if (!ret) {
		for (size_t k = 0, n = obj->size; k < n; k += 1) {
			ret = json_visitor_free_value(jsonFact, obj->values[k]);
			if (ret) {
				break;
			}
			ret = json_visitor_free_string(jsonFact, obj->names[k]);
			if (ret) {
				break;
			}
		}
	}

	if (!ret) {
		jsonFact->allocator->free(obj->values);
		jsonFact->allocator->free(obj->names);
		jsonFact->allocator->free(obj);
	}

	return ret;
}
int json_visitor_free_array(json_factory* jsonFact, json_array* arr) {
	int ret = (!arr || (arr->size > 0 && !arr->values)) ? 1 : 0;

	if (!ret) {
		for (size_t k = 0, n = arr->size; k < n; k += 1) {
			ret = json_visitor_free_value(jsonFact, arr->values[k]);
			if (ret) {
				break;
			}
		}
	}

	if (!ret) {
		jsonFact->allocator->free(arr->values);
		jsonFact->allocator->free(arr);
	}

	return ret;
}
int json_visitor_free_string(json_factory* jsonFact, json_string* str) {
	int ret = (str) ? 0 : 1;

	if (!ret) {
		jsonFact->allocator->free((void*) str->value);
		jsonFact->allocator->free(str);
	}

	return ret;
}
int json_visitor_free_number(json_factory* jsonFact, json_number* num) {
	int ret = (num) ? 0 : 1;

	if (!ret) {
		jsonFact->allocator->free(num);
	}

	return ret;
}
int json_visitor_free_true(json_factory* jsonFact, json_true* tru) {
	int ret = (tru) ? 0 : 1;

	if (!ret) {
		jsonFact->allocator->free(tru);
	}

	return ret;
}
int json_visitor_free_false(json_factory* jsonFact, json_false* fals) {
	int ret = (fals) ? 0 : 1;

	if (!ret) {
		jsonFact->allocator->free(fals);
	}

	return ret;
}
int json_visitor_free_null(json_factory* jsonFact, json_null* nul) {
	int ret = (nul) ? 0 : 1;

	if (!ret) {
		jsonFact->allocator->free(nul);
	}

	return ret;
}


#ifdef __cplusplus
}
#endif	//#ifdef __cplusplus


#endif	//#ifndef JSON_TYPES_C
