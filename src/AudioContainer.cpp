//
// Created by vagrant on 6/7/19.
//

#include <iostream>
#include <vector>

#include <boost/process.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/streambuf.hpp>

#include "AudioContainer.h"
#include "base64.h"

namespace bp = boost::process;
namespace asio = boost::asio;

std::string AudioContainer::getFlacBase64(uid_t id) const {

	boost::filesystem::path ffmpeg = bp::search_path("ffmpeg");

	if (ffmpeg.empty()) {
		throw std::runtime_error("Failed to find ffmpeg on the path! Make sure it is installed!");
	}

	std::vector<unsigned char> flacData;

	asio::io_service ios;

	bp::async_pipe out(ios); // Sends data to the ffmpeg process
	bp::async_pipe in(ios); // Reads data back from ffmpeg

	bp::child c(ffmpeg, "-f", "u16le", "-ar", "48000", "-i", "pipe:0", "-f", "flac", "pipe:1", bp::std_in < out,
	            bp::std_out > in);

	auto outBuffer = asio::buffer(audio);
	asio::async_write(out, outBuffer, [&](const boost::system::error_code& ec, std::size_t n) {
		out.close();
	});

	std::array<unsigned char, 4096> tmpFlacData = std::array<unsigned char, 4096>();
	auto inBuffer = asio::buffer(tmpFlacData);
	std::function<void(const boost::system::error_code& ec, size_t)> inFunc =
			[&](const boost::system::error_code& ec, size_t n) {
				flacData.reserve(flacData.size() + n);
				flacData.insert(flacData.end(), tmpFlacData.begin(), tmpFlacData.begin() + n);
				if (!ec) {
					asio::async_read(in, inBuffer, asio::transfer_at_least(1), inFunc);
				} else {
					std::cerr << "An error occurred!" << std::endl;
				}
			};
	asio::async_read(in, inBuffer, asio::transfer_at_least(1), inFunc);

	ios.run();
	c.wait();

	return base64_encode((const unsigned char*) flacData.data(), flacData.size());
}
