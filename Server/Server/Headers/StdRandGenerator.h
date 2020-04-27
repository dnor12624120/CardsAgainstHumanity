#pragma once

#include "Exceptions.h"
#include "GeneratorStrategy.h"

#include <algorithm>
#include <ctime>

class StdRandGenerator : public GeneratorStrategy
{
	public:
		StdRandGenerator()
		{
			
		}

		virtual int generateIntInRange(int minValue, int maxValue) override
		{
			if (maxValue < minValue)
			{
				throw StdRandGeneratorException("minValue cannot exceed maxValue.");
			}
			return rand() % maxValue + minValue;
		}
};