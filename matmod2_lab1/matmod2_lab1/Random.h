#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#include <random>
#include <ctime>

class Random
{
public:
	Random();

	int getIntRange(int low, int high);
	//double getDoubleRange(double low, double high);

private:
	std::mt19937 m_randNumGenerator;
	//std::default_random_engine m_randNumGenerator_dbl;
};

#endif // !RANDOM_H_INCLUDED
