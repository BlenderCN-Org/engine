/**
 * @file
 */

#include "RawVolume.h"

namespace voxel {

static const uint8_t SAMPLER_INVALIDX = 1 << 0;
static const uint8_t SAMPLER_INVALIDY = 1 << 1;
static const uint8_t SAMPLER_INVALIDZ = 1 << 2;

RawVolume::RawVolume(const Region& regValid) :
		_region(regValid), _mins(std::numeric_limits<int>::max()), _maxs(std::numeric_limits<int>::min()), _boundsValid(false) {
	//Create a volume of the right size.
	initialise(regValid);
}

RawVolume::RawVolume(const RawVolume* copy) :
		_region(copy->region()) {
	setBorderValue(copy->borderValue());
	_data = new Voxel[width() * height() * depth()];
	_mins = copy->_mins;
	_maxs = copy->_maxs;
	_boundsValid = copy->_boundsValid;
	std::copy(copy->_data, copy->_data + (copy->width() * copy->height() * copy->depth()), _data);
}

RawVolume::RawVolume(RawVolume&& move) {
	_data = move._data;
	move._data = nullptr;
	_mins = move._mins;
	_maxs = move._maxs;
	_region = move._region;
	_boundsValid = move._boundsValid;
}

RawVolume::~RawVolume() {
	delete[] _data;
	_data = nullptr;
}

/**
 * This version of the function is provided so that the wrap mode does not need
 * to be specified as a template parameter, as it may be confusing to some users.
 * @param uXPos The @c x position of the voxel
 * @param uYPos The @c y position of the voxel
 * @param uZPos The @c z position of the voxel
 * @return The voxel value
 */
const Voxel& RawVolume::voxel(int32_t uXPos, int32_t uYPos, int32_t uZPos) const {
	if (_region.containsPoint(uXPos, uYPos, uZPos)) {
		const Region& regValidRegion = _region;
		const int32_t iLocalXPos = uXPos - regValidRegion.getLowerX();
		const int32_t iLocalYPos = uYPos - regValidRegion.getLowerY();
		const int32_t iLocalZPos = uZPos - regValidRegion.getLowerZ();

		return _data[iLocalXPos + iLocalYPos * width() + iLocalZPos * width() * height()];
	}
	return _borderVoxel;
}

/**
 * @param[in] voxel The value to use for voxels outside the volume.
 */
void RawVolume::setBorderValue(const Voxel& voxel) {
	_borderVoxel = voxel;
}

/**
 * @param x the @c x position of the voxel
 * @param y the @c y position of the voxel
 * @param z the @c z position of the voxel
 * @param voxel the value to which the voxel will be set
 * @return @c true if the voxel was placed, @c false if it was already the same voxel
 */
bool RawVolume::setVoxel(int32_t x, int32_t y, int32_t z, const Voxel& voxel) {
	return setVoxel(glm::ivec3(x, y, z), voxel);
}

/**
 * @param pos the 3D position of the voxel
 * @param voxel the value to which the voxel will be set
 * @return @c true if the voxel was placed, @c false if it was already the same voxel
 */
bool RawVolume::setVoxel(const glm::ivec3& pos, const Voxel& voxel) {
	const bool inside = _region.containsPoint(pos);
	core_assert_msg(inside, "Position is outside valid region %i:%i:%i (mins[%i:%i:%i], maxs[%i:%i:%i])",
			pos.x, pos.y, pos.z, _region.getLowerX(), _region.getLowerY(), _region.getLowerZ(),
			_region.getUpperX(), _region.getUpperY(), _region.getUpperZ());
	if (!inside) {
		return false;
	}
	const glm::ivec3& lowerCorner = _region.getLowerCorner();
	const int32_t localXPos = pos.x - lowerCorner.x;
	const int32_t localYPos = pos.y - lowerCorner.y;
	const int32_t iLocalZPos = pos.z - lowerCorner.z;
	const int index = localXPos + localYPos * width() + iLocalZPos * width() * height();
	if (_data[index].isSame(voxel)) {
		return false;
	}
	_mins = glm::min(_mins, pos);
	_maxs = glm::max(_maxs, pos);
	_boundsValid = true;
	_data[index] = voxel;
	return true;
}

/**
 * This function should probably be made internal...
 */
void RawVolume::initialise(const Region& regValidRegion) {
	_region = regValidRegion;

	core_assert_msg(width() > 0, "Volume width must be greater than zero.");
	core_assert_msg(height() > 0, "Volume height must be greater than zero.");
	core_assert_msg(depth() > 0, "Volume depth must be greater than zero.");

	//Create the data
	_data = new Voxel[width() * height() * depth()];

	// Clear to zeros
	clear();
}

void RawVolume::clear() {
	std::fill(_data, _data + width() * height() * depth(), Voxel());
	_mins = glm::ivec3(std::numeric_limits<int>::max());
	_maxs = glm::ivec3(std::numeric_limits<int>::min());
	_boundsValid = false;
}

/**
 * @note: This function needs reviewing for accuracy...
 */
uint32_t RawVolume::calculateSizeInBytes() {
	return width() * height() * depth() * sizeof(Voxel);
}

RawVolume::Sampler::Sampler(const RawVolume* volume) :
		_volume(const_cast<RawVolume*>(volume)) {
}

RawVolume::Sampler::Sampler(const RawVolume& volume) :
		_volume(const_cast<RawVolume*>(&volume)) {
}

RawVolume::Sampler::~Sampler() {
}

bool RawVolume::Sampler::setVoxel(const Voxel& voxel) {
	if (!_currentPositionInvalid) {
		*_currentVoxel = voxel;
		_volume->_mins = glm::min(_volume->_mins, _posInVolume);
		_volume->_mins = glm::max(_volume->_maxs, _posInVolume);
		_volume->_boundsValid = true;
		return true;
	}
	return false;
}

bool RawVolume::Sampler::setPosition(int32_t xPos, int32_t yPos, int32_t zPos) {
	_posInVolume.x = xPos;
	_posInVolume.y = yPos;
	_posInVolume.z = zPos;

	const voxel::Region& region = _volume->region();
	_currentPositionInvalid = 0u;
	if (!region.containsPointInX(xPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	}
	if (!region.containsPointInY(yPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	}
	if (!region.containsPointInZ(zPos)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (currentPositionValid()) {
		const glm::ivec3& v3dLowerCorner = region.getLowerCorner();
		const int32_t iLocalXPos = xPos - v3dLowerCorner.x;
		const int32_t iLocalYPos = yPos - v3dLowerCorner.y;
		const int32_t iLocalZPos = zPos - v3dLowerCorner.z;
		const int32_t uVoxelIndex = iLocalXPos + iLocalYPos * _volume->width() + iLocalZPos * _volume->width() * _volume->height();

		_currentVoxel = _volume->_data + uVoxelIndex;
		return true;
	}
	_currentVoxel = nullptr;
	return false;
}

void RawVolume::Sampler::movePositiveX() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.x++;

	if (!_volume->region().containsPointInX(_posInVolume.x)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDX;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		++_currentVoxel;
	} else {
		setPosition(_posInVolume);
	}
}

void RawVolume::Sampler::movePositiveY() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.y++;

	if (!_volume->region().containsPointInY(_posInVolume.y)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDY;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		_currentVoxel += _volume->width();
	} else {
		setPosition(_posInVolume);
	}
}

void RawVolume::Sampler::movePositiveZ() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.z++;

	if (!_volume->region().containsPointInZ(_posInVolume.z)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		_currentVoxel += _volume->width() * _volume->height();
	} else {
		setPosition(_posInVolume);
	}
}

void RawVolume::Sampler::moveNegativeX() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.x--;

	if (!_volume->region().containsPointInX(_posInVolume.x)) {
		_currentPositionInvalid |= SAMPLER_INVALIDX;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDX;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		--_currentVoxel;
	} else {
		setPosition(_posInVolume);
	}
}

void RawVolume::Sampler::moveNegativeY() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.y--;

	if (!_volume->region().containsPointInY(_posInVolume.y)) {
		_currentPositionInvalid |= SAMPLER_INVALIDY;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDY;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		_currentVoxel -= _volume->width();
	} else {
		setPosition(_posInVolume);
	}
}

void RawVolume::Sampler::moveNegativeZ() {
	// We'll need this in a moment...
	const bool bIsOldPositionValid = currentPositionValid();

	_posInVolume.z--;

	if (!_volume->region().containsPointInZ(_posInVolume.z)) {
		_currentPositionInvalid |= SAMPLER_INVALIDZ;
	} else {
		_currentPositionInvalid &= ~SAMPLER_INVALIDZ;
	}

	// Then we update the voxel pointer
	if (currentPositionValid() && bIsOldPositionValid) {
		_currentVoxel -= _volume->width() * _volume->height();
	} else {
		setPosition(_posInVolume);
	}
}

}
