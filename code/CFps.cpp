#include "CFps.hpp"

namespace djs {
	namespace worldsim {
		CFps::CFps() {
			this->m_frames = 0;
			this->m_elapsed_time = 0.0f;
			this->m_fps = 0;
		}

		CFps::~CFps() {

		}

		void CFps::update(float delta_time) {
			this->m_elapsed_time += delta_time;
			if (this->m_elapsed_time >= 1.0f) {
				this->m_elapsed_time -= 1.0f;
				this->m_fps = this->m_frames;
				this->m_frames = 0;
			}
		}

		void CFps::draw(sf::RenderWindow& rw) {
			this->m_frames += 1;
		}

		int CFps::get_fps() {
			return this->m_fps;
		}
	}
}