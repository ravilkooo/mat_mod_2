#include "Random.h"

Random::Random()
	: m_randNumGenerator(std::time(nullptr))
{}

int Random::getIntRange(int low, int high)
{
	std::uniform_int_distribution<int> dist(low, high);
	return dist(m_randNumGenerator);
}