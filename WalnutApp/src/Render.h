#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer {
public:
    Renderer() = default;

    //  params: SSAA_level: determine the SSAA level, Gets blended in the background if the level is set too high
    void Render(float SSAA_level);

    void OnResize(uint32_t, uint32_t);

    std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
    glm::vec4 PerPixel(glm::vec2 coord);
    glm::vec4 SSAA(glm::vec2 coord, float aspectRatio, float level);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData = nullptr;
};
