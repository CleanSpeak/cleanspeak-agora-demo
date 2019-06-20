//
// Created by vagrant on 6/7/19.
//

//#include <iostream>
//#include <vector>
//#include <fstream>
//#include <sstream>

//#include <boost/process.hpp>
//#include <boost/asio/buffer.hpp>
//#include <boost/asio/streambuf.hpp>

#include "AudioContainer.h"
#include "base64.h"

//namespace bp = boost::process;
//namespace asio = boost::asio;

//std::string AudioContainer::getFlacBase64() const {
//
//	boost::filesystem::path ffmpeg = bp::search_path("ffmpeg");
//
//	if (ffmpeg.empty()) {
//		throw std::runtime_error("Failed to find ffmpeg on the path! Make sure it is installed!");
//	}
//
//	bp::opstream out;
//
//	// Stream data to ffmpeg and save to a file. (It has to be able to seek to write the header when its done, which is not possible with a pipe)
//	bp::child c(ffmpeg, "-f", "s16le", "-ar", "48000", "-i", "pipe:0", "-f", "flac", "-y", "out.flac",
//	            bp::std_in < out);
//
//	out.pipe().write((const char*) audio.data(), audio.size());
//	out.pipe().close();
//
//	c.wait();
//
//	// Open the file and seek to the end
//	std::ifstream in("out.flac", std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
//
//	std::vector<unsigned char> flacData;
//
//	// Get the size of the file
//	int size = in.tellg();
//	// Create a buffer for the file
//	flacData.resize(size);
//	// Seek to the beginning of the file
//	in.seekg(0);
//	// Read the whole file
//	in.read((char*) flacData.data(), size);
//
//	// Return the base64 version of the file
//	return base64_encode((const unsigned char*) flacData.data(), flacData.size());
//}

std::string AudioContainer::getRawBase64() const {
	return base64_encode(audio.data(), audio.size());
}
