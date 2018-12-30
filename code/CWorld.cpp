#include "CWorld.hpp"
#include <cassert>

namespace djs {
	namespace worldsim {
		CWorld::CWorld(unsigned int world_width, unsigned int world_height) {
			// dimensions
			this->m_world_width = world_width;
			this->m_world_height = world_height;

			// buffer image
			this->m_buffer_image.create(this->m_world_width, this->m_world_height, sf::Color::Black);
			this->m_drawing_image.create(this->m_world_width, this->m_world_height, sf::Color::Black);
			this->m_drawing_texture.loadFromImage(this->m_drawing_image);
			this->m_drawing_sprite.setTexture(this->m_drawing_texture);

			// stats
			this->m_stats_num_predator = 0;
			this->m_stats_predator_total_health = 0;
			this->m_stats_num_prey = 0;
			this->m_stats_prey_total_health = 0;
			this->m_stats_sim_step_count = 0;

			// blank grid
			this->m_entities_grid.resize(this->m_world_width);
			for (unsigned int x = 0; x < this->m_world_width; ++x) {
				this->m_entities_grid[x].resize(this->m_world_height);
				for (unsigned int y = 0; y < this->m_world_height; ++y) {
					this->m_entities_grid[x][y] = nullptr;
				}
			}

			// initial predators
			for (int i = 0; i < INITIAL_PREDATOR; ++i) {
				// find a location
				for (int j = 0; j < 1000; ++j) {
					SEntity* entity = new SEntity;
					entity->type = EEntityType::PREDATOR;
					entity->health = ((std::rand() % 100) / 100.0f) * MAX_HEALTH + 1;
					entity->x = std::rand() % this->m_world_width;
					entity->y = std::rand() % this->m_world_height;

					// make sure it can be placed there
					if (this->m_entities_grid[entity->x][entity->y] == nullptr){
						// we can put it here in the grid
						this->m_entities_grid[entity->x][entity->y] = entity;

						// add it to the list
						this->m_entities_list.push_back(entity);

						// set pixel
						this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Red);

						// count
						this->m_stats_num_predator += 1;

						// break inner j loop
						break;
					}
				}
			}

			// initial prey
			for (int i = 0; i < INITIAL_PREY; ++i) {
				// find a location
				for (int j = 0; j < 1000; ++j) {
					SEntity* entity = new SEntity;
					entity->type = EEntityType::PREY;
					entity->health = ((std::rand() % 100) / 100.0f) * MAX_HEALTH + 1;
					entity->x = std::rand() % this->m_world_width;
					entity->y = std::rand() % this->m_world_height;

					// make sure it can be placed there
					if (this->m_entities_grid[entity->x][entity->y] == nullptr) {
						// we can put it here in the grid
						this->m_entities_grid[entity->x][entity->y] = entity;

						// add it to the list
						this->m_entities_list.push_back(entity);

						// set pixel
						this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Green);

						// count
						this->m_stats_num_prey += 1;

						// break inner j loop
						break;
					}
				}
			}

			// drawing
			this->m_drawing_texture.loadFromImage(this->m_buffer_image);
			this->m_drawing_sprite.setTexture(this->m_drawing_texture);
			this->m_drawing_image_changed = false;

			// run the sim
			this->m_sim_running = true;
			this->m_sim_thread = std::thread(&CWorld::sim_thread_function, this);
		}

		CWorld::~CWorld() {
			this->stop_sim();
			for (auto entity : this->m_entities_list) {
				delete entity;
			}
		}

		void CWorld::update(float delta_time) {

		}

		void CWorld::draw(sf::RenderWindow& rw) {
			this->m_drawing_mutex.lock();
			if (this->m_drawing_image_changed) {
				this->m_drawing_texture.loadFromImage(this->m_drawing_image);
				this->m_drawing_image_changed = false;
			}
			this->m_drawing_mutex.unlock();
			this->m_drawing_sprite.setScale(2.0f, 2.0f);
			rw.draw(this->m_drawing_sprite);
		}

		unsigned int CWorld::get_stat_number_predator() {
			this->m_stats_mutex.lock();
			unsigned int num = this->m_stats_num_predator;
			this->m_stats_mutex.unlock();
			return num;
		}

		unsigned int CWorld::get_stat_number_prey() {
			this->m_stats_mutex.lock();
			unsigned int num = this->m_stats_num_prey;
			this->m_stats_mutex.unlock();
			return num;
		}

		unsigned int CWorld::get_stat_sim_step_count() {
			this->m_stats_mutex.lock();
			unsigned int num = this->m_stats_sim_step_count;
			this->m_stats_mutex.unlock();
			return num;
		}

		sf::Vector2u CWorld::get_random_move_location(SEntity* entity, bool wrap) {
			// current location
			int target_x = entity->x;
			int target_y = entity->y;

			// random 4 directions
			switch (std::rand() % 4) {
				case 0: {
					// up
					target_y -= 1;
				} break;
				case 1: {
					// down
					target_y += 1;
				} break;
				case 2: {
					// left
					target_x -= 1;
				} break;
				case 3: {
					// right
					target_x += 1;
				} break;
			}

			// wrap or bound
			if (wrap) {
				target_x = target_x < 0 ? target_x + this->m_world_width : target_x >= this->m_world_width ? target_x - this->m_world_width : target_x;
				target_y = target_y < 0 ? target_y + this->m_world_height : target_y >= this->m_world_height ? target_y - this->m_world_height : target_y;
			}
			else {
				target_x = target_x < 0 ? 0 : target_x >= this->m_world_width ? this->m_world_width - 1 : target_x;
				target_y = target_y < 0 ? 0 : target_y >= this->m_world_height ? this->m_world_height - 1 : target_y;
			}

			return sf::Vector2u(target_x, target_y);
		}

		void CWorld::sim_thread_function() {
			// timing
			sf::Clock clock;

			// loop while running
			while (this->m_sim_running) {
				if (clock.getElapsedTime().asMilliseconds() >= SIM_STEP_MILLISECOND) {
					// print step time
					printf("%d\n", clock.getElapsedTime().asMilliseconds());

					// count steps
					this->m_stats_sim_step_count += 1;

					// restart clock
					clock.restart();

					// loop through all the entities
					for (auto it = this->m_entities_list.begin(); it != this->m_entities_list.end(); ) {
						// get the entity
						SEntity* entity = *it;

						// what kind of entity is it
						if (entity->type == EEntityType::PREDATOR) {
							// decrement health
							entity->health -= 1;

							// check for dead
							if (entity->health <= 0) {
								// remove it
								this->m_entities_grid[entity->x][entity->y] = nullptr;
								this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Black);
								delete entity;
								it = this->m_entities_list.erase(it);
								this->m_stats_num_predator -= 1;
								continue;
							}

							// get random location to move to
							sf::Vector2u loc = this->get_random_move_location(entity, true);

							// see if there is another entity there
							SEntity* other = this->m_entities_grid[loc.x][loc.y];
							if (other == nullptr) {
								// nothing there so just move there
								this->m_entities_grid[entity->x][entity->y] = nullptr;
								this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Black);
								entity->x = loc.x;
								entity->y = loc.y;
								this->m_entities_grid[entity->x][entity->y] = entity;
								this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Red);
							}
							else {
								// something there...what?
								if (other->type == EEntityType::PREDATOR) {
									// do nothing
								}
								else if (other->type == EEntityType::PREY) {
									// eat it!!!
									other->type = EEntityType::PREDATOR;
									entity->health = std::min(entity->health + other->health, MAX_HEALTH);
									this->m_stats_num_predator += 1;
									this->m_stats_num_prey -= 1;
									this->m_buffer_image.setPixel(other->x, other->y, sf::Color::Red);
								}
								else {
									// shouldnt get here
									assert(false);
								}
							}

							// increment iterator
							++it;
						}
						else if (entity->type == EEntityType::PREY) {
							// increment health
							entity->health += 1;

							// flag if can reproduce
							bool reproduce = false;
							if (entity->health >= MAX_HEALTH) {
								reproduce = true;
								entity->health = REPRODUCE_HEALTH;
							}

							// get random location
							sf::Vector2u loc = this->get_random_move_location(entity, true);

							// see if there is another entity there
							if (this->m_entities_grid[loc.x][loc.y] == nullptr) {
								// nothing there so either reproduce there or move there
								if (reproduce) {
									// reproduce
									SEntity* child = new SEntity;
									child->type = EEntityType::PREY;
									child->x = loc.x;
									child->y = loc.y;
									child->health = REPRODUCE_HEALTH;
									this->m_entities_grid[child->x][child->y] = child;
									this->m_entities_list.push_back(child);
									this->m_stats_num_prey += 1;
									this->m_buffer_image.setPixel(child->x, child->y, sf::Color::Green);
								}
								else {
									// move there
									this->m_entities_grid[entity->x][entity->y] = nullptr;
									this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Black);
									entity->x = loc.x;
									entity->y = loc.y;
									this->m_entities_grid[entity->x][entity->y] = entity;
									this->m_buffer_image.setPixel(entity->x, entity->y, sf::Color::Green);
								}
							}
							else {
								// something there...doesnt matter...cant do anything
							}

							// increment iterator
							++it;
						}
						else {
							// shouldnt get here
							assert(false);
						}
					}

					// update the texture on the image
					this->m_drawing_mutex.lock();
					this->m_drawing_image.copy(this->m_buffer_image, 0, 0);
					this->m_drawing_image_changed = true;
					this->m_drawing_mutex.unlock();
				}
			}
		}

		void CWorld::stop_sim() {
			if (this->m_sim_running) {
				this->m_sim_running = false;
				this->m_sim_thread.join();
			}
		}
	}
}