#include "std_include.hpp"

#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		65536

namespace Utils
{
	const char *VA(const char *fmt, ...)
	{
		static char g_vaBuffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int g_vaNextBufferIndex = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = g_vaBuffer[g_vaNextBufferIndex];
		vsnprintf(g_vaBuffer[g_vaNextBufferIndex], VA_BUFFER_SIZE, fmt, ap);
		g_vaNextBufferIndex = (g_vaNextBufferIndex + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	bool ReadFile(std::string name, std::string& data)
	{
		std::ifstream stream(name, std::ios::binary);
		if(stream.is_open())
		{
			stream.seekg(0, std::ios::end);
			std::streampos size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data.clear();
				data.resize(size_t(size));

				stream.read(const_cast<char*>(data.data()), size);
				return true;
			}
		}

		return false;
	}
}
