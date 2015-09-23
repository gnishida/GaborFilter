#include "Shape.h"
#include "GLUtils.h"
#include <iostream>
#include <sstream>
#include "CGA.h"
#include "BoundingBox.h"
#include "OBJLoader.h"
#include "GeneralObject.h"

namespace cga {

std::map<std::string, Asset> Shape::assets;

boost::shared_ptr<Shape> Shape::clone(const std::string& name) const {
	throw "clone() is not supported.";
}

void Shape::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	throw "comp() is not supported.";
}

boost::shared_ptr<Shape> Shape::extrude(const std::string& name, float height) {
	throw "extrude() is not supported.";
}

boost::shared_ptr<Shape> Shape::insert(const std::string& name, const std::string& geometryPath) {
	Asset asset = getAsset(geometryPath);

	// compute scale
	float scaleX = 1.0f;
	float scaleY = 1.0f;
	float scaleZ = 1.0f;

	BoundingBox bbox(asset.points);
	if (_scope.x != 0 && _scope.y != 0 && _scope.z != 0) {			// all non-zero
		scaleX = _scope.x / bbox.sx();
		scaleY = _scope.y / bbox.sy();
		scaleZ = _scope.z / bbox.sz();
	} else if (_scope.x == 0 && _scope.y != 0 && _scope.z != 0) {	// sx == 0
		scaleY = _scope.y / bbox.sy();
		scaleZ = _scope.z / bbox.sz();
		scaleX = (scaleY + scaleZ) * 0.5f;
	} else if (_scope.x != 0 && _scope.y == 0 && _scope.z != 0) {	// sy == 0
		scaleX = _scope.x / bbox.sx();
		scaleZ = _scope.z / bbox.sz();
		scaleY = (scaleX + scaleZ) * 0.5f;
	} else if (_scope.x != 0 && _scope.y != 0 && _scope.z == 0) {	// sz == 0
		scaleX = _scope.x / bbox.sx();
		scaleY = _scope.y / bbox.sy();
		scaleZ = (scaleX + scaleY) * 0.5f;
	} else if (_scope.x != 0) {										// sy == 0 && sz == 0
		scaleX = _scope.x / bbox.sx();
		scaleY = scaleX;
		scaleZ = scaleX;
	} else if (_scope.y != 0) {										// sx == 0 && sz == 0
		scaleY = _scope.y / bbox.sy();
		scaleX = scaleY;
		scaleZ = scaleY;
	} else if (_scope.z != 0) {										// sx == 0 && sy == 0
		scaleZ = _scope.z / bbox.sz();
		scaleX = scaleZ;
		scaleY = scaleZ;
	} else { // all zero
		// do nothing
	}

	// scale the points
	for (int i = 0; i < asset.points.size(); ++i) {
		for (int k = 0; k < asset.points[i].size(); ++k) {
			asset.points[i][k].x = (asset.points[i][k].x - bbox.minPt.x) * scaleX;
			asset.points[i][k].y = (asset.points[i][k].y - bbox.minPt.y) * scaleY;
			asset.points[i][k].z = (asset.points[i][k].z - bbox.minPt.z) * scaleZ;
		}
	}

	// if texCoords are not defined in obj file, generate them automatically.
	if (_texCoords.size() > 0 && asset.texCoords.size() == 0) {
		asset.texCoords.resize(asset.points.size());
		for (int i = 0; i < asset.points.size(); ++i) {
			asset.texCoords[i].resize(asset.points[i].size());
			for (int k = 0; k < asset.points[i].size(); ++k) {
				asset.texCoords[i][k].x = asset.points[i][k].x / _scope.x * (_texCoords[1].x - _texCoords[0].x) + _texCoords[0].x;
				asset.texCoords[i][k].y = asset.points[i][k].y / _scope.y * (_texCoords[2].y - _texCoords[0].y) + _texCoords[0].y;
			}
		}
	}

	if (asset.texCoords.size() > 0) {
		return boost::shared_ptr<Shape>(new GeneralObject(name, _pivot, _modelMat, asset.points, asset.normals, _color, asset.texCoords, _texture));
	} else {
		return boost::shared_ptr<Shape>(new GeneralObject(name, _pivot, _modelMat, asset.points, asset.normals, _color));
	}
}

void Shape::nil() {
	_removed = true;
}

void Shape::split(int splitAxis, const std::vector<float>& sizes, const std::vector<std::string>& names, std::vector<boost::shared_ptr<Shape> >& objects) {
	throw "split() is not supported.";
}

void Shape::render(RenderManager* renderManager, const std::string& name, float opacity, bool showScopeCoordinateSystem) const {
	throw "render() is not supported.";
}

void Shape::drawAxes(RenderManager* renderManager, const glm::mat4& modelMat) const {
	std::vector<Vertex> vertices;
	glutils::drawAxes(0.1, 3, modelMat, vertices);
	renderManager->addObject("axis", "", vertices);
}

Asset Shape::getAsset(const std::string& filename) {
	if (assets.find(filename) == assets.end()) {
		std::vector<std::vector<glm::vec3> > points;
		std::vector<std::vector<glm::vec3> > normals;
		std::vector<std::vector<glm::vec2> > texCoords;
		if (!OBJLoader::load(filename.c_str(), points, normals, texCoords)) {
			std::stringstream ss;
			ss << "OBJ file cannot be read: " << filename.c_str() << "." << std::endl;
			std::cout << ss.str() << std::endl;
			throw ss.str();
		}

		assets[filename] = Asset(points, normals, texCoords);
	}

	return assets[filename];
}

}