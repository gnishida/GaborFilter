#include "CGA.h"
#include "GLUtils.h"
#include <map>
#include <iostream>
#include <QDir>
#include "RuleParser.h"

namespace cga {

CGA::CGA() {
}

/**
 * axiomからスタートし、ルールを適用してモデルを生成する。
 * モデルは、terminalの集合である。この時点では、まだgeometryは生成されていない。
 */
void CGA::derive() {
	shapes.clear();
	stack.clear();
	stack.push_back(axiom->clone(axiom->_name));

	while (!stack.empty()) {
		boost::shared_ptr<Shape> shape = stack.front();
		stack.pop_front();

		if (ruleSet.contain(shape->_name)) {
			ruleSet.getRule(shape->_name).apply(shape, ruleSet, stack);
		} else {
			if (shape->_name.back() != '!' && shape->_name.back() != '.') {
				//std::cout << "Warning: " << "no rule is found for " << shape->_name << "." << std::endl;
			}
			shapes.push_back(shape);
		}
	}
}

/**
 * 生成されたモデルに基づいて、geometryを生成する。
 *
 * @param renderManager		生成したgeometryを格納する。
 * @param showScopeCoordinateSystem		Scopeの座標系のgeometryを生成する。
 */
void CGA::generateGeometry(RenderManager* renderManager, bool showScopeCoordinateSystem) {
	renderManager->removeObject("shape");

	for (int i = 0; i < shapes.size(); ++i) {
		shapes[i]->render(renderManager, "shape", 1.0f, showScopeCoordinateSystem);
	}
}

}
