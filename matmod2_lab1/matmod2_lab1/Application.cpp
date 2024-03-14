#include "Application.h"

#include <iostream>

Application::Application() : Application(12, 30) {}
Application::Application(unsigned N, unsigned qs) : Application(N, qs, 10)
{}
Application::Application(unsigned N, unsigned qs, unsigned fps_sim)
	: m_window({ N*qs, N*qs }, "Cell machine")
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

	m_state = State::Creating;

	m_pixels.reserve(WIDTH * HEIGHT * 4);

	m_window.setFramerateLimit(60);
	 
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
		auto colour = (cell == Cell_1::Alive) ?
			sf::Color::Black :
			sf::Color::White;
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
		//m_cells[getCellIndex(x, y)] = (Cell_1) m_rand.getIntRange(0, 1);
		m_cells[getCellIndex(x, y)] = (Cell_1) 0;
	});

	cellForEach([&](int x, int y) {
		addQuad(x, y);
	});
}

void Application::run()
{
	while (m_window.isOpen())
	{
		m_window.clear();

		switch (m_state)
		{
			case State::Simulating:
				updateWorld();
				handleSimulateInput();
				break;
			case State::Creating:
				handleCreateInput();
				break;
			case State::Searching:
			{
				std::cout << "Search...\n";
				uint32_t min = (((uint32_t)1) << 5 * (5 - 1));
				uint32_t lim = (((uint32_t)1) << (5 * 5 - 1));
				//for (uint32_t i = min; i < lim; i++)
				for (uint32_t i = lim - 1; i > min; i--)
				{

					//std::cout << "explore [" << i << "]\n";
					startSeed(i);
					int per = -1;
					for (int step = 0; step < (5 * 5); step++)
					{
						updateWorld_2();
						bool equal = check();
						if (equal)
						{
							per = step + 1;
							break;
						}
					}
					if (per != -1)
					{
						std::cout << "FOUND [" << i << "]; per = " << per << "\n";
						for (int i = 0; i < 5; i++)
						{
							for (int j = 0; j < 5; j++)
							{
								int x = 7 + j;
								int y = 7 + i;

								std::cout << ((m_cells[getCellIndex(x, y)] == (Cell_1)0) ? "_" : "#");
							}
							std::cout << "\n";
						}
					}
				}
				m_state = State::Creating;
				break;
			}
		}
		m_window.draw(m_pixels.data(), m_pixels.size() , sf::Quads);
		m_window.draw(m_text);

		m_window.display();
		handleEvents();
	}
}

void Application::handleEvents()
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

void Application::updateWorld()
{
	std::vector<Cell_1> newCells(WIDTH * HEIGHT);

	cellForEach([&](int x, int y)
	{
		unsigned count = 0;
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
			auto cell = m_cells[getCellIndex(newX, newY)];
			if (cell == Cell_1::Alive) count++;

		}

		auto cell = m_cells[getCellIndex(x, y)];
		auto& updateCell = newCells[getCellIndex(x, y)];
		updateCell = (cell == Cell_1::Alive ? Cell_1::Alive : Cell_1::Dead);
		switch (cell)
		{
			case Cell_1::Alive:
			if (count >= 3)
			{
				updateCell = Cell_1::Dead;
			}
			break;
			
			case Cell_1::Dead:
			if (count == 2)
			{
				updateCell = Cell_1::Alive;
			}
			break;

		}

		setQuadColour(x, y, updateCell);

	});
	m_cells = std::move(newCells);

	m_step = (m_step + 1);
	m_text.setString(std::to_string(m_step));
}

void Application::updateWorld_2()
{
	std::vector<Cell_1> newCells(WIDTH * HEIGHT);

	cellForEach([&](int x, int y) {
		unsigned count = 0;
		for (int nX = -1; nX <= 1; nX++)
			for (int nY = -1; nY <= 1; nY++)
			{
				int newX = nX + x;
				int newY = nY + y;

				if (newX == -1 || newX == (int)WIDTH ||
					newY == -1 || newY == (int)HEIGHT ||
					(newX == 0 && newY == 0))
				{
					continue;
				}
				auto cell = m_cells[getCellIndex(newX, newY)];
				if (cell == Cell_1::Alive) count++;

			}

		auto cell = m_cells[getCellIndex(x, y)];
		auto& updateCell = newCells[getCellIndex(x, y)];
		updateCell = cell;
		switch (cell)
		{
			case Cell_1::Alive:
			if (count >= 3)
			{
				updateCell = Cell_1::Dead;
			}
			break;

			case Cell_1::Dead:
			if (count == 2)
			{
				updateCell = Cell_1::Alive;
			}
			break;

		}

	});
	m_cells = std::move(newCells);
}


unsigned Application::getCellIndex(unsigned x, unsigned y)
{
	return x * HEIGHT + y;
}

void Application::setQuadColour(unsigned x, unsigned y, Cell_1 cell)
{
	auto index = getCellIndex(x, y) * 4;
	auto colour = (cell == Cell_1::Alive) ?
		sf::Color::Black :
		sf::Color::White;

	m_pixels[index].color = colour;
	m_pixels[index + 1].color = colour;
	m_pixels[index + 2].color = colour;
	m_pixels[index + 3].color = colour;
}

void Application::handleCreateInput()
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

				cell = cell == Cell_1::Alive ?
					Cell_1::Dead :
					Cell_1::Alive;

				setQuadColour(newX, newY, cell);


			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = (Cell_1)0;
				setQuadColour(x, y, (Cell_1)0);
			});
		}

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = (Cell_1) (x % 2);
				setQuadColour(x, y, (Cell_1) (x % 2));
			});
		}*/

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
		{
			cellForEach([&](int x, int y) {

				bool col = (x == 0 || x == 2 || x == 4 || y == 0 || y == 2 || y == 4)
					|| (x == WIDTH-1 || x == WIDTH-3 || x == WIDTH - 5 || y == HEIGHT - 5 || HEIGHT-3 == y || HEIGHT-1 == y);

				m_cells[getCellIndex(x, y)] = (Cell_1)col;
				setQuadColour(x, y, (Cell_1)col);
			});
			// for (unsigned x = 0; x < WIDTH; x++)
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = (Cell_1)0;
				setQuadColour(x, y, (Cell_1)0);
			});
			for (unsigned y = 0; y < HEIGHT; y++)
			{
				m_cells[getCellIndex(0, y)] = (Cell_1)1;
				setQuadColour(0, y, (Cell_1)1);
				m_cells[getCellIndex(HEIGHT - 1, y)] = (Cell_1)1;
				setQuadColour(HEIGHT - 1, y, (Cell_1)1);

				if (y < HEIGHT / 2 && y % 2 == 0)
					for (unsigned x = 2; x < WIDTH - 2; x++)
					{
						m_cells[getCellIndex(x, y)] = (Cell_1)1;
						setQuadColour(x, y, (Cell_1)1);

						m_cells[getCellIndex(x, HEIGHT - y - 1)] = (Cell_1)1;
						setQuadColour(x, HEIGHT - y - 1, (Cell_1)1);
					}
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = (Cell_1)0;
				setQuadColour(x, y, (Cell_1)0);
			});
			for (unsigned y = 0; y < HEIGHT; y++)
			{
				m_cells[getCellIndex(0, y)] = (Cell_1)1;
				setQuadColour(0, y, (Cell_1)1);
				m_cells[getCellIndex(HEIGHT - 1, y)] = (Cell_1)1;
				setQuadColour(HEIGHT - 1, y, (Cell_1)1);

				if (y < HEIGHT / 2 && y % 2 == 1)
					for (unsigned x = 2; x < WIDTH - 2; x++)
					{
						m_cells[getCellIndex(x, y)] = (Cell_1)1;
						setQuadColour(x, y, (Cell_1)1);

						m_cells[getCellIndex(x, HEIGHT - y - 1)] = (Cell_1)1;
						setQuadColour(x, HEIGHT - y - 1, (Cell_1)1);
					}
			}
		}
		
		if (m_mmb && !sf::Mouse::isButtonPressed(sf::Mouse::Middle))
		{
			m_mmb = false;
		}
		if (!m_mmb && sf::Mouse::isButtonPressed(sf::Mouse::Middle))
		{
			m_mmb = true;
			m_pen = !m_pen;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			updateWorld();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
		{
			cellForEach([&](int x, int y) {
				m_cells[getCellIndex(x, y)] = (Cell_1)m_rand.getIntRange(0, 1);
				//m_cells[getCellIndex(x, y)] = (Cell_1)0;
				setQuadColour(x, y, m_cells[getCellIndex(x, y)]);
			});
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_pause = sf::Clock();

			m_state = State::Simulating;
			m_window.setFramerateLimit(m_fps_sim);
			m_text.setString("");
		}

		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
		{
			m_pause = sf::Clock();

			m_state = State::Searching;
			m_window.setFramerateLimit(FPS_SIM);
			m_text.setString("Search");
		}*/

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
							cell = (Cell_1) m_pen;
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
								cell = (Cell_1) m_pen;
								setQuadColour(xx, yy, cell);
							}
						else
							for (int yy = m_ry; yy < newY + 1; yy++)
							{
								int xx = m_rx + std::round((yy - m_ry) / k);

								auto& cell = m_cells[getCellIndex(xx, yy)];
								cell = (Cell_1) m_pen;
								setQuadColour(xx, yy, cell);
							}

					}
					m_rx = -1; m_ry = -1;
				}
			}
		}
	}
}

void Application::handleSimulateInput()
{
	if (m_delay.getElapsedTime().asSeconds() > 0.1)
	{
		m_delay = sf::Clock();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		{
			m_pause = sf::Clock();

			m_state = State::Creating;
			m_window.setFramerateLimit(60);
			m_text.setString("Pause");
		}
	}
}

void Application::startSeed(uint32_t seed)
{	m_cells.clear();

	m_seed = seed;

	m_cells.resize(WIDTH * HEIGHT);

	m_window.setFramerateLimit(60);

	cellForEach([&](int x, int y) {
		//m_cells[getCellIndex(x, y)] = (Cell_1) m_rand.getIntRange(0, 1);
		m_cells[getCellIndex(x, y)] = (Cell_1)0;
	});

	for (int i = 0; i < (5 * 5); i++)
	{
		int x = 7 + i % 5;
		int y = 7 + (i / 5);

		bool alive = (seed >> i) & 1;

		m_cells[getCellIndex(x, y)] = (Cell_1)alive;
	}
}

bool Application::check()
{
	bool equal = true;
	for (int i = 0; (i < (5 * 5)) && equal; i++)
	{
		int x = 7 + i % 5;
		int y = 7 + (i / 5);

		bool alive = (m_seed >> i) & 1;

		equal = equal && (m_cells[getCellIndex(x, y)] == (Cell_1)alive);

	}
	for (int x = 6; (x < 7 + 5 + 1) && equal; x++)
	{
		equal = equal && (m_cells[getCellIndex(x, 6)] == Cell_1::Dead);
		equal = equal && (m_cells[getCellIndex(x, 7 + 5 + 1)] == Cell_1::Dead);

		equal = equal && (m_cells[getCellIndex(6, x)] == Cell_1::Dead);
		equal = equal && (m_cells[getCellIndex(7 + 5 + 1, x)] == Cell_1::Dead);
	}
	return equal;
}