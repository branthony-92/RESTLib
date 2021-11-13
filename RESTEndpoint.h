#ifndef RESTENDPOINT_H
#define RESTENDPOINT_H

#include "RESTModels.h"
#include "MdlResponseInfo.h"

struct RequestData {
	std::string  target;
	JSON queries;
	JSON body;
};

typedef  std::function<BodyInfoPtr(RequestData&)> RequestCallback;
typedef  std::map<std::string,RequestCallback> CallbackMap;

#endif // !RESTENDPOINT_H