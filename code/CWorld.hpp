#ifndef djs_worldsim_CWorld_hpp
#define djs_worldsim_CWorld_hpp

#include <SFML/Graphics.hpp>
#include <list>
#include <thread>
#include <mutex>
#include <vector>

namespace djs {
	namespace worldsim {
		enum EEntityType {
			PREDATOR, PREY
		};

		struct SEntity {
			EEntityType type;
			unsigned char health;
			unsigned short x;
			unsigned short y;
		};

		class CWorld {
			public:
				CWorld(unsigned int world_width, unsigned int world_height);
				~CWorld();

				void update(float delta_time);
				void draw(sf::RenderWindow& rw);

				unsigned int get_stat_number_predator();
				unsigned int get_stat_number_prey();
				unsigned int get_stat_sim_step_count();

			protected:

			private:
				void sim_thread_function();
				void stop_sim();
				sf::Vector2u get_random_move_location(SEntity* entity, bool wrap);

				// sim
				bool m_sim_running;
				std::thread m_sim_thread;

				// dimensions
				unsigned int m_world_width;
				unsigned int m_world_height;

				// list
				std::list<SEntity*> m_entities_list;
				std::vector<std::vector<SEntity*>> m_entities_grid;

				// drawing
				sf::Image m_buffer_image;
				sf::Image m_drawing_image;
				sf::Texture m_drawing_texture;
				sf::Sprite m_drawing_sprite;
				std::mutex m_drawing_mutex;
				bool m_drawing_image_changed;


				// stats
				std::mutex m_stats_mutex;
				unsigned int m_stats_num_predator;
				unsigned int m_stats_predator_total_health;
				unsigned int m_stats_num_prey;
				unsigned int m_stats_prey_total_health;
				unsigned int m_stats_sim_step_count;

				// config
				const int MAX_HEALTH = 50;
				const int REPRODUCE_HEALTH = 10;
				const int INITIAL_PREDATOR = 100;
				const int INITIAL_PREY = 10000;
				const int SIM_STEP_MILLISECOND = 16;
		};
	}
}
#endif