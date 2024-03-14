#include "Application_2.h"

#include <iostream>

Cell_2::Cell_2() : m_state(Cell_2_State::Resting), m_state_step(0) {};
Cell_2::Cell_2(Cell_2_State state) : m_state(state), m_state_step(0) {};
Cell_2::Cell_2(Cell_2_State state, double level) : m_state(state), m_level(level), m_state_step(0) {};


Application_2::Application_2() : Application_2(30, 10)
{
}
Application_2::Application_2(unsigned N, unsigned qs) : Application_2(N, qs, 2)
{
}
Application_2::Application_2(unsigned N, unsigned qs, unsigned fps_sim)
	: m_window({ qs * N, qs * N }, "Cell machine")
	/*
	, m_pixels((m_window.getSize().x / QUAD_SIZE) *
		(m_window.getSize().y / QUAD_SIZE))
	*/
	, QUAD_SIZE(qs)
	, WIDTH(m_window.getSize().x / QUAD_SIZE)
	, HEIGHT(m_window.getSize().y / QUAD_SIZE)
	, m_cells(WIDTH* HEIGHT)
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

	m_state = State_2::Creating;

	m_pixels.reserve(WIDTH * HEIGHT * 4);
	m_window.setFramerateLimit(20);

	auto addQuad = [&](int x, int y) {
		sf::Vertex topLeft;
		sf::Vertex topRight;
		sf::Vertex bottomLeft;
		sf::Vertex bottomRight;

		float pixelX = x * QUAD_SIZE;
		float pixelY = y * QUAD_SIZE;

		topLeft.position = { pixelX, pixelY };
		topRight.position = { pixelX + WIDTH, pixelY };
		bottomLeft.position = { pixelX, pixelY + WIDTH };
		bottomRight.position = { pixelX + WIDTH, pixelY + WIDTH };


		auto cell = m_cells[getCellIndex(x, y)];
		sf::Color colour; 
		switch (cell.m_state)
		{
			case Cell_2_State::Resting:
			colour = sf::Color::White;
			break;
			case Cell_2_State::Active:
			colour = sf::Color::Black;
			break;
			case Cell_2_State::Recovering:
			colour = sf::Color::Cyan;
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
		//m_cells[getCellIndex(x, y)] = (Cell_2) m_rand.getIntRange(0, 1);
		m_cells[getCellIndex(x, y)] = Cell_2(Cell_2_State::Resting);
	});

	m_source_x.push_back(WIDTH / 2);
	m_source_y.push_back(HEIGHT / 2);

	m_source_x.push_back(20);
	m_source_y.push_back(20);


	m_source_x.push_back(WIDTH - 40);
	m_source_y.push_back(60);


	/*for (unsigned x = 0; x < WIDTH; x++)
	{
		m_cells[getCellIndex(x, 10)] = Cell_2(Cell_2_State::Active);
		m_cells[getCellIndex(x, 10)].m_level = 1.;
	}*/

	cellForEach([&](int x, int y) {
		addQuad(x, y);
	});
}

void Application_2::run()
{
	while (m_window.isOpen())
	{
		m_window.clear();

		switch (m_state)
		{
			case State_2::Simulating:
			for (int i = 0; i < m_curr_steps_per_tick; i++)
			{
				updateWorld();
			}

			handleSimulateInput();
			break;
			case State_2::Creating:
			handleCreateInput();
			break;
		}
		m_window.draw(m_pixels.data(), m_pixels.size(), sf::Quads);
		m_window.draw(m_text);

		m_window.display();
		handleEvents();
	}
}

void Application_2::handleEvents()
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

void Application_2::updateWorld()
{
	std::vector<Cell_2> newCells(WIDTH * HEIGHT);

	cellForEach([&](int x, int y) {

		auto cell = m_cells[getCellIndex(x, y)];
		auto& updateCell = newCells[getCellIndex(x, y)];
		bool is_src = false;
		for (int i = 0; i < m_source_x.size(); i++)
		{
			unsigned source_x = m_source_x[i];
			unsigned source_y = m_source_y[i];
			if ((x >= source_x) && (x < (source_x + 3))
				&& (y >= source_y) && (y < (source_y + 3))
				&& ((m_step) % m_source_period == 0))
			{
				updateCell.m_state_step = 0;
				updateCell.m_state = Cell_2_State::Active;
				updateCell.m_level = 1.;
				is_src = true;
				break;
			}
		}
		if (is_src)
		{
			
		}
		else if (cell.m_state == Cell_2_State::Active)
		{
			updateCell.m_state_step = (cell.m_state_step + 1) % ACTIVE_DURATION;
			updateCell.m_state = (updateCell.m_state_step == 0 ? Cell_2_State::Recovering : Cell_2_State::Active);
			updateCell.m_level = 1; // cell.m_level * (1. - m_loss);
		}
		else if (cell.m_state == Cell_2_State::Recovering)
		{
			updateCell.m_state_step = (cell.m_state_step + 1) % RECOVER_DURATION;
			updateCell.m_state = (updateCell.m_state_step == 0 ? Cell_2_State::Resting : Cell_2_State::Recovering);
			updateCell.m_level = cell.m_level * (1. - m_loss);
		}
		else if (cell.m_state == Cell_2_State::Resting)
		{
			double sum_level = 0;
			for (int nX = -1; nX <= 1; nX++)
				for (int nY = -1; nY <= 1; nY++)
				{
					int newX = nX + x;
					int newY = nY + y;

					if (newX == -1 || newX == (int)WIDTH ||
						newY == -1 || newY == (int)HEIGHT)
					{
						continue;
					}
					sum_level += m_cells[getCellIndex(newX, newY)].m_level;
				}
			if (sum_level >= m_threshold)
			{
				updateCell.m_state_step = 0;
				updateCell.m_state = Cell_2_State::Active;
				updateCell.m_level = 1.;
			}
			else
			{
				updateCell.m_level = cell.m_level * (1. - m_loss);
			}
		}

		setQuadColour(x, y, updateCell);

	});
	m_cells = std::move(newCells);

	m_step = (m_step+1) % m_source_period;
	m_text.setString(std::to_string(m_step)); 
}

unsigned Application_2::getCellIndex(unsigned x, unsigned y)
{
	return x * HEIGHT + y;
}

void Application_2::setQuadColour(unsigned x, unsigned y, Cell_2 cell)
{
	auto index = getCellIndex(x, y) * 4;
	sf::Color colour;
	switch (cell.m_state)
	{
		case Cell_2_State::Resting:
		colour = sf::Color::White;
		break;
		case Cell_2_State::Active:
		colour = sf::Color::Black;
		break;
		case Cell_2_State::Recovering:
		colour = sf::Color::Cyan;
		break;
	}

	m_pixels[index].color = colour;
	m_pixels[index + 1].color = colour;
	m_pixels[index + 2].color = colour;
	m_pixels[index + 3].color = colour;
}

void Application_2::handleCreateInput()
{
	//sf::Clock delay;

	//if (delay.getElapsedTime().asSeconds() < 0.1) 
	{


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

				if (cell.m_state == Cell_2_State::Resting)
				{
					cell = Cell_2(Cell_2_State::Recovering);
				}
				else if (cell.m_state == Cell_2_State::Recovering)
				{
					cell = Cell_2(Cell_2_State::Active, 1);
				}
				else if (cell.m_state == Cell_2_State::Active)
				{
					cell = Cell_2(Cell_2_State::Resting, 0.5);
				}
				setQuadColour(newX, newY, cell);
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = Cell_2(Cell_2_State::Resting);
				setQuadColour(x, y, Cell_2(Cell_2_State::Resting));
			});
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = Cell_2((Cell_2_State)m_rand.getIntRange(0, 2));
				m_cells[getCellIndex(x, y)].m_level = (m_cells[getCellIndex(x, y)].m_state == Cell_2_State::Active ? 1. : 0.);

				//m_cells[getCellIndex(x, y)] = (Cell_2)0;
				setQuadColour(x, y, m_cells[getCellIndex(x, y)]);
			});
		}
		if (m_spb && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_spb = false;
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
			m_curr_steps_per_tick = (m_curr_steps_per_tick == 1) ? m_curr_steps_per_tick : ((int)m_curr_steps_per_tick / 2);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			m_curr_steps_per_tick = ((int)m_curr_steps_per_tick * 2);
		}

		if (!m_spb && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_spb = true;
			m_state = State_2::Simulating;
			m_window.setFramerateLimit(m_fps_sim);
			m_text.setString("");
		}

		if (m_rmb && !sf::Mouse::isButtonPressed(sf::Mouse::Right))
		{
			m_rmb = false;
		}

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
							cell = Cell_2(Cell_2_State::Active, 1);
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
								cell = Cell_2(Cell_2_State::Active, 1);
								setQuadColour(xx, yy, cell);
							}
						else
							for (int yy = m_ry; yy < newY + 1; yy++)
							{
								int xx = m_rx + std::round((yy - m_ry) / k);

								auto& cell = m_cells[getCellIndex(xx, yy)];
								cell = Cell_2(Cell_2_State::Active, 1);
								setQuadColour(xx, yy, cell);
							}

					}
					m_rx = -1; m_ry = -1;
				}
			}
		}
	}


}

void Application_2::handleSimulateInput()
{
	//if (m_delay.getElapsedTime().asSeconds() > 0.1)
	{
		if (m_spb && !sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_spb = false;
		}

		if (!m_spb && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_spb = true;
			m_state = State_2::Creating;
			m_text.setString("Pause");
		}
	}
}
