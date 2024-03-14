#include "Application_3.h"

#include <iostream>

Cell_3::Cell_3() : m_state(Cell_3_State::Free) {
	m_P = m_P_max;
};

Cell_3::Cell_3(Cell_3_State state) : m_state(state)
{
	m_P = m_P_max;
	if (state == Cell_3_State::Occupied)
	{
		m_anim_p = m_dr;
		m_anim_age = 0;
	}
};

Application_3::Application_3() : Application_3(30, 10)
{}
Application_3::Application_3(unsigned N, unsigned qs) : Application_3(N, qs, 2)
{}
Application_3::Application_3(unsigned N, unsigned qs, unsigned fps_sim)
	: m_N(N), m_window({ qs * N, qs * N }, "Cell machine")
	/*
	, m_pixels((m_window.getSize().x / QUAD_SIZE) *
		(m_window.getSize().y / QUAD_SIZE))
	*/
	, QUAD_SIZE(qs)
	, WIDTH(m_window.getSize().x / QUAD_SIZE)
	, HEIGHT(m_window.getSize().y / QUAD_SIZE)
	, m_cells(WIDTH * HEIGHT)
	, m_fps_sim(fps_sim)
{
	m_font.loadFromFile("font/arial.ttf");
	m_text.setFont(m_font);
	m_text.setFillColor(sf::Color::White);
	m_text.setOutlineColor(sf::Color::Black);
	m_text.setOutlineThickness(3);
	m_text.setCharacterSize(15);
	m_text.setPosition(5, 5);
	m_text.setString("Pause");
	
	m_state = State_3::Creating;

	m_pixels.reserve(WIDTH * HEIGHT * 4);
	m_window.setFramerateLimit(20);

	//m_proc_cells.resize(WIDTH * HEIGHT, false);

	std::cout << "WIDTH = " << WIDTH << "; HEIGHT = " << HEIGHT << ";\n";
	std::cout << "QUAD_SIZE = " << QUAD_SIZE << "; qs = " << qs << ";\n";
	std::cout << "N = " << m_N << ";\n";

	auto addQuad = [&](int x, int y) {
		sf::Vertex topLeft;
		sf::Vertex topRight;
		sf::Vertex bottomLeft;
		sf::Vertex bottomRight;

		float pixelX = x * QUAD_SIZE;
		float pixelY = y * QUAD_SIZE;

		topLeft.position = { pixelX, pixelY };
		topRight.position = { pixelX + QUAD_SIZE, pixelY };
		bottomLeft.position = { pixelX, pixelY + QUAD_SIZE };
		bottomRight.position = { pixelX + QUAD_SIZE, pixelY + QUAD_SIZE };


		auto cell = m_cells[getCellIndex(x, y)];
		sf::Color colour;
		switch (cell.m_state)
		{
			case Cell_3_State::Free:
			colour = sf::Color(
				255 * (1 - m_cells[getCellIndex(x, y)].m_P * 1. / m_cells[getCellIndex(x, y)].m_P_max),
				255,
				255 * (1 - m_cells[getCellIndex(x, y)].m_P * 1. / m_cells[getCellIndex(x, y)].m_P_max));
			break;
			case Cell_3_State::Occupied:
			colour = sf::Color::Blue;
			/*colour = sf::Color(
				100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1),
				100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1),
				100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1));*/
			break;
		}
		topLeft.color = colour;
		topRight.color = colour;
		bottomLeft.color = colour;
		bottomRight.color = colour;

		m_pixels.push_back(topLeft);
		m_pixels.push_back(bottomLeft);
		m_pixels.push_back(bottomRight);
		m_pixels.push_back(topRight);

	};

	cellForEach([&](int x, int y) {
		//m_cells[getCellIndex(x, y)] = (Cell_3) m_rand.getIntRange(0, 1);
		m_cells[getCellIndex(x, y)] = Cell_3(Cell_3_State::Free);
	});

	int animal_number = (int)std::floor(m_A * m_N * m_N);
	std::cout << "animal_number = " << animal_number << ";\n";

	for (int i = 0; i < animal_number; i++)
	{
		int x = 0;
		int y = 0;
		do
		{
			x = m_rand.getIntRange(0, WIDTH - 1);
			y = m_rand.getIntRange(0, HEIGHT - 1);
			// std::cout << "c2: " << x << " " << y << "\n";
		} while (m_cells[getCellIndex(x, y)].m_state == Cell_3_State::Occupied);
		int idx = getCellIndex(x, y);
		m_cells[idx].m_state = Cell_3_State::Occupied;
		m_cells[idx].m_P = m_cells[idx].m_P_max;
		m_cells[idx].m_anim_age = 0;
		m_cells[idx].m_anim_p = m_cells[idx].m_dr;
	}

	cellForEach([&](int x, int y) {
		addQuad(x, y);
	});
}

void Application_3::run(std::vector<unsigned> & data)
{
	while (m_window.isOpen())
	{
		m_window.clear();

		switch (m_state)
		{
			case State_3::Simulating:
			for (int i = 0; i < m_curr_steps_per_tick; i++)
			{
				data.push_back(updateWorld());
			}

			handleSimulateInput();
			break;
			case State_3::Creating:
			handleCreateInput();
			break;
		}
		m_window.draw(m_pixels.data(), m_pixels.size(), sf::Quads);
		m_window.draw(m_text);

		m_window.display();
		handleEvents();
	}
}

void Application_3::handleEvents()
{
	sf::Event e;

	while (m_window.pollEvent(e))
	{
		if (e.type == sf::Event::Closed)
		{
			m_window.close();
		}
	}
}

unsigned Application_3::updateWorld()
{
	std::vector<Cell_3> newCells(WIDTH * HEIGHT);
	std::vector<bool> proc_cells(WIDTH * HEIGHT, false);
	unsigned anim_number = 0;

	cellForEach([&](int x, int y) {
		bool moved = false;
		unsigned idx = getCellIndex(x, y);
		auto cell = m_cells[idx];
		auto& updateCell = newCells[idx];

		updateCell.m_P = cell.m_P + cell.m_r;

		if (cell.m_state == Cell_3_State::Free)
		{
			if (updateCell.m_state != Cell_3_State::Occupied)
				updateCell.m_state = Cell_3_State::Free;
		}
		else if (cell.m_state == Cell_3_State::Occupied)
		{
			updateCell.m_anim_age = cell.m_anim_age + 1;
			if (cell.m_anim_age >= cell.m_L)
				updateCell.m_state = Cell_3_State::Free;
			else
			{
				updateCell.m_state = Cell_3_State::Occupied;
				if (updateCell.m_P >= cell.m_dp)
				{
					updateCell.m_P = cell.m_P - cell.m_dp;
					updateCell.m_anim_p = cell.m_anim_p + cell.m_dp - cell.m_de;
					updateCell.m_anim_p = updateCell.m_anim_p > updateCell.m_p1 ? updateCell.m_p1 : updateCell.m_anim_p;
				}
				else
					updateCell.m_anim_p = cell.m_anim_p - cell.m_de;

				if (updateCell.m_anim_p <= 0)
					updateCell.m_state = Cell_3_State::Free;
				else
					anim_number++;
			}
			if (updateCell.m_state == Cell_3_State::Occupied && cell.m_state == Cell_3_State::Occupied)
			{
				// move or divide
				
				std::vector<unsigned> migr_idxs;
				for (int nX = -1; nX <= 1; nX++)
					for (int nY = -1; nY <= 1; nY++)
					{
						int newX = nX + x;
						int newY = nY + y;
						if (newX == -1 || newX == (int)WIDTH ||
							newY == -1 || newY == (int)HEIGHT ||
							(nX == 0 && nY == 0))
						{
							continue;
						}
						unsigned new_idx = getCellIndex(newX, newY);
						if ((proc_cells[new_idx] && newCells[new_idx].m_state != Cell_3_State::Occupied)
							|| (!proc_cells[new_idx]
								&& newCells[new_idx].m_state == Cell_3_State::Free
								&& m_cells[new_idx].m_state == Cell_3_State::Free))
						{
							migr_idxs.push_back(new_idx);
						}
					}
				if (!migr_idxs.empty())
				{
					unsigned migr_idx = migr_idxs[m_rand.getIntRange(0, migr_idxs.size() - 1)];
					unsigned max_en = proc_cells[migr_idx] ? newCells[migr_idx].m_P :
						std::min(m_cells[migr_idx].m_P + m_cells[migr_idx].m_r, m_cells[migr_idx].m_P_max);
					if (m_smart)
					{
						for (int i = 0; i < migr_idxs.size(); i++)
						{
							unsigned curr_en = proc_cells[migr_idxs[i]] ?
								newCells[migr_idxs[i]].m_P :
								std::min(m_cells[migr_idxs[i]].m_P + m_cells[migr_idxs[i]].m_r,
									m_cells[migr_idxs[i]].m_P_max);

							if (max_en < curr_en)
								migr_idx = migr_idxs[i];
						}
						if (max_en < newCells[idx].m_P)
							migr_idx = idx;


					}
					if (migr_idx != idx)
					{
						//auto pos = migrate_pos[0];
						updateCell.m_state = Cell_3_State::Free;

						newCells[migr_idx].m_state = Cell_3_State::Occupied;
						newCells[migr_idx].m_anim_p = updateCell.m_anim_p;
						newCells[migr_idx].m_anim_age = updateCell.m_anim_age;
						//newCells[migr_idx].m_prev_pos = std::make_pair(x, y);

						auto pos = getCellPos(migr_idx);
						setQuadColour(pos.first, pos.second, newCells[migr_idx]);

						if (newCells[migr_idx].m_anim_age >= newCells[migr_idx].m_T)
						{
							if (newCells[migr_idx].m_anim_p > newCells[migr_idx].m_dr)
							{
								newCells[migr_idx].m_anim_p -= newCells[migr_idx].m_dr;

								updateCell.m_state = Cell_3_State::Occupied;
								updateCell.m_anim_p = updateCell.m_dr;
								updateCell.m_anim_age = 0;
								anim_number++;
							}

						}
					}
				}
			}
		}
		updateCell.m_P = updateCell.m_P > updateCell.m_P_max ? updateCell.m_P_max : updateCell.m_P;
		proc_cells[idx] = true;
		setQuadColour(x, y, updateCell);
	});
	m_cells = std::move(newCells);
	m_step = (m_step + 1);
	m_text.setString(std::to_string(m_step) + " (" + std::to_string(anim_number) + ")");
	if (anim_number == 0)
	{
		m_state = State_3::Creating;
		m_window.setFramerateLimit(60);
		m_text.setString("Pause");
	}
	return anim_number;
}

unsigned Application_3::getCellIndex(unsigned x, unsigned y)
{
	return x * HEIGHT + y;
}

std::pair<unsigned, unsigned> Application_3::getCellPos(unsigned idx)
{
	return std::make_pair(idx / HEIGHT, idx % HEIGHT);
}

void Application_3::setQuadColour(unsigned x, unsigned y, Cell_3 cell)
{
	auto index = getCellIndex(x, y) * 4;
	sf::Color colour;
	switch (cell.m_state)
	{
		case Cell_3_State::Free:
		colour = sf::Color(
			255 * (1 - m_cells[getCellIndex(x, y)].m_P * 1. / m_cells[getCellIndex(x, y)].m_P_max),
			255,
			255 * (1 - m_cells[getCellIndex(x, y)].m_P * 1. / m_cells[getCellIndex(x, y)].m_P_max));
		break;
		case Cell_3_State::Occupied:
		colour = sf::Color::Blue;
		/*colour = sf::Color(
			100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1),
			100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1),
			100 * (1 - m_cells[getCellIndex(x, y)].m_anim_p * 1. / m_cells[getCellIndex(x, y)].m_p1));*/
		break;
	}
	m_pixels[index].color = colour;
	m_pixels[index + 1].color = colour;
	m_pixels[index + 2].color = colour;
	m_pixels[index + 3].color = colour;
}

void Application_3::handleCreateInput()
{
	if (m_delay.getElapsedTime().asSeconds() > 0.1)
	{
		m_delay = sf::Clock();
		if (m_lmb && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			m_lmb = false;
		}
		if (!m_lmb && sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			m_lmb = true;
			//delay.restart();
			auto mousePosition = sf::Mouse::getPosition(m_window);
			auto newX = mousePosition.x / QUAD_SIZE;
			auto newY = mousePosition.y / QUAD_SIZE;
			//std::cout << newX << ", " << newY << "\n";
			if (!(newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT))
			{
				auto& cell = m_cells[getCellIndex(newX, newY)];
				if (cell.m_state == Cell_3_State::Free)
				{
					cell = Cell_3(Cell_3_State::Occupied);
				}
				else if (cell.m_state == Cell_3_State::Occupied)
				{
					cell = Cell_3(Cell_3_State::Free);
				}
				setQuadColour(newX, newY, cell);
			}
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = Cell_3(Cell_3_State::Free);
				setQuadColour(x, y, Cell_3(Cell_3_State::Free));
			});
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			cellForEach([&](int x, int y) {
				//m_cells[getCellIndex(x, y)] = (Cell_3) m_rand.getIntRange(0, 1);
				m_cells[getCellIndex(x, y)] = Cell_3(Cell_3_State::Free);
			});			

			int animal_number = (int)std::floor(m_A * m_N * m_N);

			for (int i = 0; i < animal_number; i++)
			{
				int x = 0;
				int y = 0;
				do
				{
					x = m_rand.getIntRange(0, WIDTH - 1);
					y = m_rand.getIntRange(0, HEIGHT - 1);
				} while (m_cells[getCellIndex(x, y)].m_state == Cell_3_State::Occupied);
				int idx = getCellIndex(x, y);
				m_cells[idx].m_state = Cell_3_State::Occupied;
				m_cells[idx].m_P = m_cells[idx].m_P_max;
				m_cells[idx].m_anim_age = 0;
				m_cells[idx].m_anim_p = m_cells[idx].m_dr;
			}

			cellForEach([&](int x, int y) {
				setQuadColour(x, y, m_cells[getCellIndex(x, y)]);
			});
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			for (int i = 0; i < m_curr_steps_per_tick; i++)
			{
				updateWorld();
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			m_curr_steps_per_tick = (m_curr_steps_per_tick == 1) ? m_curr_steps_per_tick : ((int)m_curr_steps_per_tick - 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			m_curr_steps_per_tick = ((int)m_curr_steps_per_tick + 1);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_pause = sf::Clock();

			m_state = State_3::Simulating;
			m_window.setFramerateLimit(m_fps_sim);
			m_text.setString("");
		}

		if (m_rmb && !sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			m_rmb = false;
		}

		/*
		if (!m_rmb && sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			m_rmb = true;

			auto mousePosition = sf::Mouse::getPosition(m_window);

			auto newX = mousePosition.x / QUAD_SIZE;
			auto newY = mousePosition.y / QUAD_SIZE;

			//std::cout << newX << ", " << newY << "\n";

			if (!(newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT))
			{
				if (m_rx == -1)
				{
					m_rx = newX;
					m_ry = newY;
				}
				else
				{
					if (m_rx == newX)
					{
						if (m_ry > newY)
						{
							int _z = newY;
							newY = m_ry;
							m_ry = _z;
						}
						for (int yy = m_ry; yy < newY + 1; yy++)
						{
							auto& cell = m_cells[getCellIndex(m_rx, yy)];
							cell = Cell_3(Cell_3_State::Active, 1);
							setQuadColour(m_rx, yy, cell);
						}

					}
					else
					{
						if (m_rx > newX)
						{
							int _z = newX;
							newX = m_rx;
							m_rx = _z;
							_z = newY;
							newY = m_ry;
							m_ry = _z;
						}
						double k = (newY - m_ry) * 1. / (newX - m_rx);
						if (std::abs(k) < 0.5)
							for (int xx = m_rx; xx < newX + 1; xx++)
							{
								int yy = m_ry + std::round(k * (xx - m_rx));

								auto& cell = m_cells[getCellIndex(xx, yy)];
								cell = Cell_3(Cell_3_State::Active, 1);
								setQuadColour(xx, yy, cell);
							}
						else
							for (int yy = m_ry; yy < newY + 1; yy++)
							{
								int xx = m_rx + std::round((yy - m_ry) / k);

								auto& cell = m_cells[getCellIndex(xx, yy)];
								cell = Cell_3(Cell_3_State::Active, 1);
								setQuadColour(xx, yy, cell);
							}

					}
					m_rx = -1; m_ry = -1;
				}
			}
		}
		*/
	}


}

void Application_3::handleSimulateInput()
{
	if (m_delay.getElapsedTime().asSeconds() > 0.1)
	{
		m_delay = sf::Clock();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_pause = sf::Clock();

			m_state = State_3::Creating;
			m_window.setFramerateLimit(60);
			m_text.setString("Pause");
		}
	}
}
