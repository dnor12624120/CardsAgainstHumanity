#pragma once

#include "Repository.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

template<typename T>
class FileRepository : public Repository<T>
{
	public:
		FileRepository() = default;
		FileRepository(const std::string& filepath):
			filepath{ filepath }
		{
			loadObjectsFromFile();
		}

		virtual const T& getObject(int index) const override { return objects[index]; }
		virtual inline int size() override 
		{
			return objects.size(); 
		}
	private:
		void loadObjectsFromFile()
		{
			std::ifstream inputFile(filepath);
			T object;
			while (inputFile >> object)
			{
				objects.emplace_back(object);
			}
			inputFile.close();
		}

		std::string filepath;
		std::vector<T> objects;
};