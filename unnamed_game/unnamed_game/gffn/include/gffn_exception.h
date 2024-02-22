#pragma once

#include <stdexcept>

namespace gffn {

class GFFN_Exception : public std::runtime_error {
public:
	GFFN_Exception(const std::string &error_string) : std::runtime_error(error_string) {}
};
} // end namespace gffn