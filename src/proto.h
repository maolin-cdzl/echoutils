#include <cassert>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "proto/echo.pb.h"

static google::protobuf::Message* create_message(const std::string& type_name)
{
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
	if (descriptor) {
		const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			message = prototype->New();
		}
	}
	return message;
}

static google::protobuf::Message* unpack_message(const void* buf,size_t size) {
	assert( buf && size );

	echo::Package pkg;
	if( pkg.ParseFromArray(buf,size) ) {
		google::protobuf::Message* msg = create_message(pkg.name());
		if( msg ) {
			if( pkg.has_body() ) {
				if( msg->ParseFromString(pkg.body()) ) {
					return msg;
				} else {
					delete msg;
				}
			} else {
				return msg;
			}
		}
	}
	return NULL;
}

static std::string pack_message(const google::protobuf::Message* msg) {
	assert( msg );

	echo::Package pkg;
	pkg.set_name(msg->GetTypeName());
	std::string body = msg->SerializeAsString();
	if( ! body.empty() ) {
		pkg.set_body(body);
	}
	return pkg.SerializeAsString();
}

