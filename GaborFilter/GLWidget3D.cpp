#include "GLWidget3D.h"
#include "MainWindow.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "GLUtils.h"
#include "Rectangle.h"
#include "RuleParser.h"
#include "CVUtils.h"

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers)) {
	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-100, 100, -100, 100, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;
}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	camera.mousePress(e->x(), e->y());
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
}

/**
 * This event handler is called when the mouse move events occur.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::LeftButton) { // Rotate
		camera.rotate(e->x(), e->y());
	} else if (e->buttons() & Qt::MidButton) { // Move
		camera.move(e->x(), e->y());
	} else if (e->buttons() & Qt::RightButton) { // Zoom
		camera.zoom(e->x(), e->y());
	}

	updateGL();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	renderManager.init("../shaders/vertex.glsl", "../shaders/geometry.glsl", "../shaders/fragment.glsl", false);
	showWireframe = true;
	showScopeCoordinateSystem = false;

	// set the clear color for the screen
	//qglClearColor(QColor(113, 112, 117));
	qglClearColor(QColor(224, 224, 224));
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;
	glViewport(0, 0, width, height);
	camera.updatePMatrix(width, height);
}

/**
 * This function is called whenever the widget needs to be painted.
 */
void GLWidget3D::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

	// pass the light direction to the shader
	//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
	glUniform3f(glGetUniformLocation(renderManager.program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	
	drawScene(0);	
}

/**
 * Draw the scene.
 */
void GLWidget3D::drawScene(int drawMode) {
	if (drawMode == 0) {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);
	} else {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 1);
	}
	
	if (showScopeCoordinateSystem) {
		renderManager.renderAll(showWireframe);
	} else {
		renderManager.renderAllExcept("axis", showWireframe);
	}
}

void GLWidget3D::loadCGA(const std::string& filename) {
	renderManager.removeObjects();

	cga::Rectangle* lot = new cga::Rectangle("Start", glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::mat4(), 10, 10, glm::vec3(1, 1, 1));
	cga_system.axiom = boost::shared_ptr<cga::Shape>(lot);

	try {
		cga::parseRules(filename, cga_system.ruleSet);
		cga_system.derive();
		cga_system.generateGeometry(&renderManager, true);
	} catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	
	updateGL();
}

void GLWidget3D::renderImage() {
	glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);

	renderManager.removeObjects();
	try {
		cga_system.derive();
		cga_system.generateGeometry(&renderManager, true);
	} catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}

	camera.xrot = 90.0f;
	camera.yrot = 0.0f;
	camera.zrot = 0.0f;
	camera.pos.z = 20.0f;

	camera.updateMVPMatrix();



	glClearColor(1, 1, 1, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

	renderManager.render("shape", true);


	unsigned char* data = new unsigned char[sizeof(unsigned char) * 3 * width() * height()];
	glReadPixels(0, 0, width(), height(), GL_BGR, GL_UNSIGNED_BYTE, data);
	cv::Mat img(height(), width(), CV_8UC3, data);
	cv::flip(img, img, 0);
	cv::imwrite("results/test1.jpg", img);

	cv::cvtColor(img, img, CV_BGR2GRAY);
	cv::imwrite("results/test2.jpg", img);
	cv::threshold(img, img, 230, 255, 0);
	cv::imwrite("results/test3.jpg", img);

	cv::Rect roi = cvutils::computeBoundingBoxFromImage(img);
	cv::Mat img2;
	img(roi).copyTo(img2);
	cv::imwrite("results/test4.jpg", img2);
				
	delete [] data;


}
