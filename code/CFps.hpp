#ifndef djs_worldsim_CFps_hpp
#define djs_worldsim_CFps_hpp

#include <SFML/Graphics.hpp>
#include <list>

namespace djs {
	namespace worldsim {
		class CFps {
			public:
				CFps();
				~CFps();

				void update(float delta_time);
				void draw(sf::RenderWindow& rw);

				int get_fps();

			protected:

			private:
				int m_frames;
				float m_elapsed_time;
				int m_fps;
		};
	}
}
#endif