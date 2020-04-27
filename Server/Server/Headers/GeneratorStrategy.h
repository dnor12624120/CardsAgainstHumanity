#pragma once

class GeneratorStrategy
{
	public:
		virtual int generateIntInRange(int minValue, int maxValue) = 0;
};