#pragma once

template<typename T>
class Repository
{
	public:
		virtual const T& getObject(int index) const = 0;
		virtual int size() = 0;
};