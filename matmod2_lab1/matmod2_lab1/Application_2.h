#ifndef APPLICATION_2_H_INCLUDED
#define APPLICATION_2_H_INCLUDED

#define LOSS 0.3
#define THRESHOLD 3
#define ACTIVE_DURATION 5
#define RECOVER_DURATION 8

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

#include "Random.h"

enum class Cell_2_State : uint8_t
{
	Resting, Active, Recovering
};
class Cell_2
{
public:
	double m_level = 0;
	int m_state_step = 0;
	Cell_2_State m_state = Cell_2_State::Resting;

	Cell_2();
	Cell_2(Cell_2_State state);
	Cell_2(Cell_2_State state, double level);

};
enum class State_2 : uint8_t
{
	Simulating, Creating
};

class Application_2
{
public:
	Application_2();
	Application_2(unsigned N, unsigned qs);
	Application_2(unsigned N, unsigned qs, unsigned fps_sim);
	void run();

private:
	void handleEvents();
	void updateWorld();
	void handleCreateInput();
	void handleSimulateInput();

	unsigned getCellIndex(unsigned x, unsigned y);
	void setQuadColour(unsigned x, unsigned y, Cell_2 cell);

	template<typename F>
	void cellForEach(F f);

	sf::RenderWindow m_window;

	const int QUAD_SIZE;
	const int WIDTH;
	const int HEIGHT;

	std::vector<sf::Vertex> m_pixels;
	std::vector<Cell_2> m_cells;

	Random m_rand;

	State_2 m_state = State_2::Creating;
	double m_threshold = THRESHOLD;
	double m_level = 0;
	double m_loss = LOSS;

	int m_step = 0;

	sf::Font m_font;
	sf::Text m_text;

	bool m_lmb = false;
	bool m_rmb = false;
	bool m_spb = false;
	int m_rx = -1, m_ry = -1;

	unsigned m_fps_sim = 2;

	int m_curr_steps_per_tick = 1;

	int m_source_period = 15;
	std::vector<unsigned> m_source_x;
	std::vector<unsigned> m_source_y;
};

template<typename F>
void Application_2::cellForEach(F f)
{
	for (unsigned x = 0; x < WIDTH; x++)
		for (unsigned y = 0; y < HEIGHT; y++)
		{
			f(x, y);
		}
}

#endif