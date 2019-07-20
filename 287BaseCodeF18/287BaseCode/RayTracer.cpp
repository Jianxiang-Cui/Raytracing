#include "RayTracer.h"
#include "IShape.h"

/**
 * @fn	RayTracer::RayTracer(const color &defa)
 * @brief	Constructs a raytracers.
 * @param	defa	The clear color.
 */

RayTracer::RayTracer(const color &defa)
	: defaultColor(defa) {
}

/**
 * @fn	void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth, const IScene &theScene) const
 * @brief	Raytrace scene
 * @param [in,out]	frameBuffer	Framebuffer.
 * @param 		  	depth	   	The current depth of recursion.
 * @param 		  	theScene   	The scene.
 */

void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth,
								const IScene &theScene) const {
	// anti-aliasing should be done here
	const RaytracingCamera &camera = *theScene.camera;
	const std::vector<VisibleIShapePtr> &objs = theScene.visibleObjects;
	const std::vector<PositionalLightPtr> &lights = theScene.lights;

	
	if (!myTwoViewOn) {
		for (int y = 0; y < frameBuffer.getWindowHeight(); y++) {
			for (int x = 0; x < frameBuffer.getWindowWidth(); x++) {

				if (anti_aliasing == 1) {
					Ray ray = camera.getRay((float)x, (float)y);
					color colorForPixel = traceIndividualRay(ray, theScene, depth);
					frameBuffer.setColor(x, y, colorForPixel);
				}
				else {
					// assume the anti-aliasing is 2
					Ray ray1 = camera.getRay((float)(x - 0.25f), (float)(y + 0.25f));
					color colorForPixel1 = traceIndividualRay(ray1, theScene, depth);
					//frameBuffer.setColor(x, y, colorForPixel);
					Ray ray2 = camera.getRay((float)(x + 0.25f), (float)(y + 0.25f));
					color colorForPixel2 = traceIndividualRay(ray2, theScene, depth);
					//frameBuffer.setColor(x, y, colorForPixel);
					Ray ray3 = camera.getRay((float)(x - 0.25f), (float)(y - 0.25f));
					color colorForPixel3 = traceIndividualRay(ray3, theScene, depth);
					//frameBuffer.setColor(x, y, colorForPixel);
					Ray ray4 = camera.getRay((float)(x + 0.25f), (float)(y + 0.25f));
					color colorForPixel4 = traceIndividualRay(ray4, theScene, depth);
					color avarageColor = (colorForPixel1 + colorForPixel2 + colorForPixel3 + colorForPixel4) / 4.0f;
					frameBuffer.setColor(x, y, avarageColor);
				}
			}
		}
	}
	else {
		// left one
		for (int y = 0; y < frameBuffer.getWindowHeight(); y++) {
			for (int x = 0; x < frameBuffer.getWindowWidth() / 2; x++) {
				Ray ray = camera.getRay((float)x, (float)y);
				color colorForPixel = traceIndividualRay(ray, theScene, depth);
				frameBuffer.setColor(x, y, colorForPixel);
			}
		}
		// right one
		theScene.camera->changeConfiguration(glm::vec3(0, 15, 15), glm::vec3(-8, 2, 3), Y_AXIS);
		for (int y = 0; y < frameBuffer.getWindowHeight(); y++) {
			for (int x = frameBuffer.getWindowWidth() / 2; x < frameBuffer.getWindowWidth(); x++) {
				Ray ray = camera.getRay((float)x, (float)y);
				color colorForPixel = traceIndividualRay(ray, theScene, depth);
				frameBuffer.setColor(x, y, colorForPixel);
			}
		}
	}

	frameBuffer.showColorBuffer();
}

/**
 * @fn	bool shadowFeeler(const Ray &ray, const IScene &theScene)
 * @brief	Determine shadow
 * @param	ray			  	The ray.
 * @param	theScene	  	The scene.
 * @return	if the pixel is in shadow
 */
bool shadowFeeler(const Ray &ray, const IScene &theScene) {
	HitRecord theHit = VisibleIShape::findIntersection(ray, theScene.visibleObjects);
	// distance between light source and object
	bool inShadow = false;
	float distance = glm::distance(theHit.interceptPoint, theScene.lights[0]->lightPosition);
	const Ray &checkRay = Ray(theScene.lights[0]->lightPosition, (theHit.interceptPoint - theScene.lights[0]->lightPosition));
	HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
	if (checkHit.t < (distance - EPSILON)
		) {
		inShadow = true;
	}
	return inShadow;
}

/**
 * @fn	color RayTracer::traceIndividualRay(const Ray &ray, const IScene &theScene, int recursionLevel) const
 * @brief	Trace an individual ray.
 * @param	ray			  	The ray.
 * @param	theScene	  	The scene.
 * @param	recursionLevel	The recursion level.
 * @return	The color to be displayed as a result of this ray.
 */

color RayTracer::traceIndividualRay(const Ray &ray, const IScene &theScene, int recursionLevel) const {
	//if (recursionLevel == 0) {
		// opaqueHit
		HitRecord theHit = VisibleIShape::findIntersection(ray, theScene.visibleObjects);
		HitRecord transHit = VisibleIShape::findIntersection(ray, theScene.transparentObjects);
		color result;
		Frame eyeFrame = theScene.camera->cameraFrame;
		if (theHit.t < FLT_MAX && transHit.t >= FLT_MAX) { // hit opaqueHit but no hit for transHit
			if (theHit.texture != nullptr) {
				float u = glm::clamp(theHit.u, 0.0f, 1.0f);
				float v = glm::clamp(theHit.v, 0.0f, 1.0f);

				//50 and 50 weright for obj color and texture color
				for (PositionalLightPtr light : theScene.lights) {

					bool inShadow = false;
					float distance = glm::distance(light->lightPosition, theHit.interceptPoint);
					glm::vec3 dirOfCheckRay = light->lightPosition - theHit.interceptPoint;
					Ray checkRay = Ray(theHit.interceptPoint + theHit.surfaceNormal * EPSILON, dirOfCheckRay);
					HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
					if (checkHit.t < distance) {
						inShadow = true;
					}
					result += 0.5f * theHit.texture->getPixel(u, v) +
						0.5f * light->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, inShadow);
				}
				//result = 0.5f * theHit.texture->getPixel(u, v) +
					//0.5f * theScene.lights[0]->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, shadowFeeler(ray, theScene));
			}
			else {
				//result = theScene.lights[0]->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, shadowFeeler(ray, theScene));
				for (PositionalLightPtr light : theScene.lights) {

					bool inShadow = false;
					float distance = glm::distance(light->lightPosition, theHit.interceptPoint);
					glm::vec3 dirOfCheckRay = light->lightPosition - theHit.interceptPoint;
					Ray checkRay = Ray(theHit.interceptPoint + theHit.surfaceNormal * EPSILON, dirOfCheckRay);
					HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
					if (checkHit.t < distance) {
						inShadow = true;
					}
					result += light->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, inShadow);
				}
			}
		}
		else if (theHit.t >= FLT_MAX && transHit.t < FLT_MAX) { // hit transHit but no hit for opaqueHit
			//result = theScene.lights[0]->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, shadowFeeler(ray, theScene));
			//result = (1 - transHit.material.alpha) * transHit.material.ambient + transHit.material.alpha * result;
			for (PositionalLightPtr light : theScene.lights) {
				bool inShadow = false;
				float distance = glm::distance(light->lightPosition, theHit.interceptPoint);
				glm::vec3 dirOfCheckRay = light->lightPosition - theHit.interceptPoint;
				Ray checkRay = Ray(theHit.interceptPoint + theHit.surfaceNormal * EPSILON, dirOfCheckRay);
				HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
				if (checkHit.t < distance) {
					inShadow = true;
				}
				result += light->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, inShadow);
				result = (1 - transHit.material.alpha) * transHit.material.ambient + transHit.material.alpha * result;
			}
		}
		else if (theHit.t < FLT_MAX && transHit.t < FLT_MAX) { // hit both
			if (theHit.texture != nullptr) {
				float u = glm::clamp(theHit.u, 0.0f, 1.0f);
				float v = glm::clamp(theHit.v, 0.0f, 1.0f);

				//50 and 50 weright for obj color and texture color
				//result = 0.5f * theHit.texture->getPixel(u, v) +
					//0.5f * theScene.lights[0]->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, shadowFeeler(ray, theScene));
				for (PositionalLightPtr light : theScene.lights) {
					bool inShadow = false;
					float distance = glm::distance(light->lightPosition, theHit.interceptPoint);
					glm::vec3 dirOfCheckRay = light->lightPosition - theHit.interceptPoint;
					Ray checkRay = Ray(theHit.interceptPoint + theHit.surfaceNormal * EPSILON, dirOfCheckRay);
					HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
					if (checkHit.t < distance) {
						inShadow = true;
					}
					result += 0.5f * theHit.texture->getPixel(u, v) +
						0.5f * light->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, inShadow);
				}
				// first hit transparent obj and then opaque
				if (theHit.t > transHit.t) {
					result = (1 - transHit.material.alpha) * result + transHit.material.alpha * transHit.material.ambient;
				}
			}
			else {
				//result = theScene.lights[0]->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, shadowFeeler(ray, theScene));
				for (PositionalLightPtr light : theScene.lights) {
					bool inShadow = false;
					float distance = glm::distance(light->lightPosition, theHit.interceptPoint);
					glm::vec3 dirOfCheckRay = light->lightPosition - theHit.interceptPoint;
					Ray checkRay = Ray(theHit.interceptPoint + theHit.surfaceNormal * EPSILON, dirOfCheckRay);
					HitRecord checkHit = VisibleIShape::findIntersection(checkRay, theScene.visibleObjects);
					if (checkHit.t < distance) {
						inShadow = true;
					}
					result += light->illuminate(theHit.interceptPoint, theHit.surfaceNormal, theHit.material, eyeFrame, inShadow);
				}
				// first hit transparent obj and then opaque
				if (theHit.t > transHit.t) {
					result = (1 - transHit.material.alpha) * result + transHit.material.alpha * transHit.material.ambient;
				}
			}
		}
		else {
			result = defaultColor;
		}
		// no hit for opaqueHit (theHit) and no hit for transHit
		return result;
	//}
	/*
	else {
		//color result = defaultColor;
		color result;
		HitRecord theHit = VisibleIShape::findIntersection(ray, theScene.visibleObjects);
		glm::vec3 origin = theHit.interceptPoint;
		glm::vec3 dir = ray.direction - 2 * glm::dot(ray.direction, theHit.surfaceNormal) * theHit.surfaceNormal;
		const Ray reflectionRay = Ray(origin, dir);
		return result += traceIndividualRay(reflectionRay, theScene, recursionLevel - 1);
	}
	*/
}