#include "psf_1.h"
#include "psf_2.h"

#include <memory>

std::unique_ptr <psf_t> open_psf (std::ifstream && _psf_file, std::ofstream && _header_file);

