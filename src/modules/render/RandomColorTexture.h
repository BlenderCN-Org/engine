/**
 * @file
 */
#pragma once

#include "video/Texture.h"
#include "noise/Noise.h"
#include "video/Types.h"
#include "core/IComponent.h"

#include <vector>
#include <future>

namespace render {

class RandomColorTexture : public core::IComponent {
private:
	video::TexturePtr _colorTexture;
	noise::Noise _noise;

	struct NoiseGenerationTask {
		NoiseGenerationTask(uint8_t *_buffer, int _width, int _height, int _depth) :
				buffer(_buffer), width(_width), height(_height), depth(_depth) {
		}
		/** @brief pointer to preallocated buffer that was hand into the noise task */
		uint8_t *buffer;
		const int width;
		const int height;
		const int depth;
	};

	typedef std::future<NoiseGenerationTask> NoiseFuture;
	std::vector<NoiseFuture> _noiseFuture;
public:
	bool init() override;
	void shutdown() override;

	void bind(video::TextureUnit unit = video::TextureUnit::Zero);
	void unbind();
};

}
