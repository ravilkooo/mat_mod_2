#ifndef APPLICATIon_3_H_INCLUDED
#define APPLICATIon_3_H_INCLUDED

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

#include "Random.h"

enum class Cell_3_State : uint8_t
{
	Free, Occupied
};
class Cell_3
{
public:
	int m_P = 0;
	int m_P_max = 10;
	int m_dp = 5;
	int m_r = 1;
	Cell_3_State m_state = Cell_3_State::Free;

	int m_anim_p = 0;
	int m_p1 = 35;
	int m_de = 2;
	int m_dr = 3;
	int m_T = 3;
	int m_L = 15;

	int m_anim_age = 0;

	Cell_3();
	Cell_3(Cell_3_State state);

private:
	int anim_idx = -1;
};
enum class State_3 : uint8_t
{
	Simulating, Creating
};

class Application_3
{
public:
	int m_N;
	double m_A = 0.3;

	sf::Clock m_delay;
	sf::Clock m_pause;

	unsigned m_fps_sim = 2;

	bool m_smart = false;

	Application_3();
	Application_3(unsigned N, unsigned qs);
	Application_3(unsigned N, unsigned qs, unsigned fps_sim);
	void run(std::vector<unsigned>& data);

private:
	void handleEvents();
	unsigned updateWorld();
	void handleCreateInput();
	void handleSimulateInput();

	unsigned getCellIndex(unsigned x, unsigned y);
	std::pair<unsigned, unsigned> getCellPos(unsigned idx);
	void setQuadColour(unsigned x, unsigned y, Cell_3 cell);

	template<typename F>
	void cellForEach(F f);

	template<typename F>
	void cellForEach_cond(F f, bool cond);

	sf::RenderWindow m_window;

	const int QUAD_SIZE;
	const int WIDTH;
	const int HEIGHT;

	std::vector<sf::Vertex> m_pixels;
	std::vector<Cell_3> m_cells;

	Random m_rand;

	State_3 m_state = State_3::Creating;

	int m_step = 0;

	sf::Font m_font;
	sf::Text m_text;

	bool m_lmb = false;
	bool m_rmb = false;
	bool m_spb = false;
	int m_rx = -1, m_ry = -1;

	int m_curr_steps_per_tick = 1;

};

template<typename F>
void Application_3::cellForEach(F f)
{
	for (unsigned x = 0; x < WIDTH; x++)
		for (unsigned y = 0; y < HEIGHT; y++)
		{
			f(x, y);
		}
}

template<typename F>
void Application_3::cellForEach_cond(F f, bool cond)
{
	for (unsigned x = 0; cond && (x < WIDTH); x++)
		for (unsigned y = 0; cond && (y < HEIGHT); y++)
		{
			f(x, y);
		}
}

#endif