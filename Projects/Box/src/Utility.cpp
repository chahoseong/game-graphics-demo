module;
#include <d3d11.h>

export module utility;

import <format>;
import <iostream>;
import <vector>;

export inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		std::cerr << std::format("hr=0x{:X}", hr) << "\n";
		throw std::exception();
	}
}
