#include <SFML/Graphics.hpp>
#include "CWorld.hpp"
#include "CFps.hpp"
#include <string>

#ifdef _DEBUG
	#pragma comment(lib, "sfml-system-d.lib")
	#pragma comment(lib, "sfml-window-d.lib")
	#pragma comment(lib, "sfml-graphics-d.lib")
#else
	#pragma comment(lib, "sfml-system.lib")
	#pragma comment(lib, "sfml-window.lib")
	#pragma comment(lib, "sfml-graphics.lib")
#endif

int main(int argc, char* argv[]) {
	// the render window
	sf::RenderWindow render_window;
	render_window.create(sf::VideoMode(1280, 720, 32), "WorldSim", sf::Style::Close | sf::Style::Titlebar);
	render_window.setPosition(sf::Vector2i(2200, 100));

	// fps
	djs::worldsim::CFps fps;

	// timing
	sf::Clock clock;
	clock.restart();

	// the world
	djs::worldsim::CWorld world(1280 / 2, 720 / 2);

	// text for stats
	sf::Font font;
	font.loadFromFile("resources/fonts/droid_bold.ttf");
	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);
	text.setFillColor(sf::Color::White);
	text.setStyle(sf::Text::Style::Regular);

	// loop
	while (render_window.isOpen()) {
		// events
		sf::Event event;
		while (render_window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed: {
					render_window.close();
				} break;
			}
		}

		// update
		float elapsed = clock.restart().asSeconds();
		world.update(elapsed);
		fps.update(elapsed);
		render_window.setTitle("WorldSim FPS: " + std::to_string(fps.get_fps()));
		text.setString("Steps: " + std::to_string(world.get_stat_sim_step_count()) +
			"\nPredator: " + std::to_string(world.get_stat_number_predator()) +
			"\nPrey: " + std::to_string(world.get_stat_number_prey()));

		// draw
		render_window.clear();
		world.draw(render_window);
		fps.draw(render_window);
		render_window.draw(text);
		render_window.display();
	}
}