/**
 * @file
 */

#pragma once

#include "Renderer.h"

namespace video {

/**
 * @ingroup Video
 */
class Cubemap {
private:
	std::string _filename;
	Id _textureHandle;
public:
	/**
	 * @brief Loads 6 textures that belongs to a cubemap. The naming schema must be "<filename>-cm-X" (where X is replaced by 1-6)
	 *
	 * The order of the 1-6 is as followed:
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	 *   - GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	 *   - GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 */
	Cubemap(const std::string& filename);
	~Cubemap();

	void shutdown();

	bool load();
	void bind(video::TextureUnit texUnit = video::TextureUnit::Zero);
	void unbind(video::TextureUnit texUnit = video::TextureUnit::Zero);
};

}
