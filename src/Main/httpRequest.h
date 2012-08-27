/* httpRequest.h */

#include "stdafx.h"

class Extractor;

bool HandleHttpRequest(char* host, const char* location, Extractor* extractor, bool return_result_only = false);