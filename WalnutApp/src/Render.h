#pragma once

#include "Camera.h"
#include "Ray.h"
#include "Walnut/Image.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>
#include <limits>

class Renderer {
public:
    Renderer() = default;

    void Render(const Scene& scene, const Camera& camera);

    //  params: SSAA_level: determine the SSAA level, Gets blended in the background if the level is set too high
    void RenderSSAA(float SSAA_level, const Camera& camera, const Scene& scene);

    void OnResize(uint32_t, uint32_t);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
    glm::vec4 TraceRay(const Scene& scene, const Ray& ray);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
};
