//
// Created by vagrant on 6/7/19.
//

#include <boost/process.hpp>

#include "AudioContainer.h"
#include "base64.h"

namespace bp = boost::process;

std::string AudioContainer::getFlacBase64(uid_t id) const {
	bp::pstream io;
	bp::child c("ffmpeg -f u16le -i pipe: -f flac pipe:", bp::std_out > io, bp::std_in < io);

	io << audio.str();

	std::string out;

	io >> out;

	return base64_encode((const unsigned char*) out.data(), out.length());
}
