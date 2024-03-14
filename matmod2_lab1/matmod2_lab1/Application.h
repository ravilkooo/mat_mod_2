#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

#include "Random.h"
 
enum class Cell_1 : uint8_t
{
	Dead, Alive
};

enum class State : uint8_t
{
	Simulating, Creating, Searching
};

class Application
{
public:
	Application(unsigned N, unsigned qs, unsigned fps_sim);
	Application(unsigned N, unsigned qs);

	Application();
	void run();

private:
	void handleEvents();
	void updateWorld();
	void updateWorld_2();
	void handleCreateInput();
	void handleSimulateInput();

	void startSeed(uint32_t seed);
	bool check();

	unsigned getCellIndex(unsigned x, unsigned y);
	void setQuadColour(unsigned x, unsigned y, Cell_1 cell);

	template<typename F>
	void cellForEach(F f);

	sf::RenderWindow m_window;

	const int QUAD_SIZE;
	const int WIDTH;
	const int HEIGHT;

	std::vector<sf::Vertex> m_pixels;
	std::vector<Cell_1> m_cells;

	Random m_rand;

	State m_state = State::Creating;

	sf::Font m_font;
	sf::Text m_text;

	bool m_pen = 1;
	bool m_rmb = false;
	bool m_mmb = false;
	bool m_lmb;
	int m_rx = -1, m_ry = -1;

	int m_step = 0;
	unsigned m_fps_sim;

	sf::Clock m_delay;
	sf::Clock m_pause;

	uint32_t m_seed = 0;
};

template<typename F>
void Application::cellForEach(F f)
{
	for (unsigned x = 0; x < WIDTH; x++)
	for (unsigned y = 0; y < HEIGHT; y++)
	{
		f(x, y);
	}
}

#endif