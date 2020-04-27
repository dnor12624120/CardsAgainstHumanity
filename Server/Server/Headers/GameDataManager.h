#pragma once

#include "GeneratorStrategy.h"
#include "Repository.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>

class GameDataManager
{
	public:
		GameDataManager(std::unique_ptr<GeneratorStrategy> indexGenerator):
			indexGenerator{ std::move(indexGenerator) }
		{

		}

		void addRepository(std::string associatedRepositoryName)
		{
			usedIndices[associatedRepositoryName] = std::set<int>();
		}

		int generateUniqueRepositoryIndex(std::string associatedRepositoryName, int repositorySize)
		{
			if (usedIndices[associatedRepositoryName].size() == repositorySize)
			{
				// throw exception
			}
			int index = indexGenerator->generateIntInRange(0, repositorySize);
			auto found = std::find(usedIndices[associatedRepositoryName].begin(), usedIndices[associatedRepositoryName].end(), index);
			while (found != usedIndices[associatedRepositoryName].end())
			{
				int index = indexGenerator->generateIntInRange(0, repositorySize);
				found = std::find(usedIndices[associatedRepositoryName].begin(), usedIndices[associatedRepositoryName].end(), index);
			}
			usedIndices[associatedRepositoryName].emplace(index);
			return index;
		}

		int generatePlayerIndex(int numOfPlayers)
		{
			return indexGenerator->generateIntInRange(0, numOfPlayers);
		}
	private:
		std::unique_ptr<GeneratorStrategy> indexGenerator;

		std::map<std::string, std::set<int>> usedIndices; 
};