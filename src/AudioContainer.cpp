//
// Created by vagrant on 6/7/19.
//

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

#include "AudioContainer.h"
#include "base64.h"

std::string AudioContainer::getFlacBase64(uid_t id) {

	std::string filename = std::to_string(id) + "_chunk.pipe";

	mkfifo(filename, 0) // TODO


	return std::string();
}
