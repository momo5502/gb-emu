#pragma once

namespace Utils
{
	const char *VA(const char *fmt, ...);

	template <typename T> void Merge(std::vector<T>* target, T* source, size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T> void Merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto &entry : source)
		{
			target->push_back(entry);
		}
	}
}
