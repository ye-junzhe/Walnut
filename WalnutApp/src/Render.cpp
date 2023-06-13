#include "Render.h"

namespace Utils {

    static uint32_t ConvertToRGBA(const glm::vec4& color) {
        uint8_t r = uint8_t(color.r * 255.0f);
        uint8_t g = uint8_t(color.g * 255.0f);
        uint8_t b = uint8_t(color.b * 255.0f);
        uint8_t a = uint8_t(color.a * 255.0f);
        
        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }
}

void Renderer::Render(float SSAA_level) {
    float aspectRatio = float(m_FinalImage->GetWidth()) / float(m_FinalImage->GetHeight());

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            glm::vec4 color = glm::vec4(0.0f);
            glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight()};
            coord = coord * 2.0f - 1.0f;
            if (SSAA_level > 0) {
                color = Renderer::SSAA(coord, aspectRatio, SSAA_level);
            } else {
                coord.x *= aspectRatio;
                color = PerPixel(coord);
            }
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->SetData(m_ImageData);
}

void Renderer::OnResize(uint32_t width, uint32_t height) {
    if (m_FinalImage) {
        if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) return;
        m_FinalImage->Resize(width, height);
    }
    else {
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[width * height];
}

glm::vec4 Renderer::SSAA(glm::vec2 coord, float aspectRatio, float level) {
    float stride = 1.0f / level;
    glm::vec4 color = glm::vec4(0.0f);

    for (float i = -stride; i <= stride; i += stride) {
        for (float j = -stride; j <= stride; j += stride) {
            glm::vec2 subCoord = coord + glm::vec2(i, j) / glm::vec2(m_FinalImage->GetWidth(), m_FinalImage->GetHeight());
            subCoord.x *= aspectRatio;
            glm::vec4 subColor = PerPixel(subCoord);
            color += subColor;
        }
    }

    color /= std::pow(2 * level, 2);
    return color;
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
    // WARNING: Negative is farther
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
    glm::vec3 sphereColor(1, 0 ,1);
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return glm::vec4(0, 0, 0, 1);
    }

    float closerT = ((-b - glm::sqrt(discriminant)) / (2.0f * a));
    float fartherT = ((-b + glm::sqrt(discriminant)) / (2.0f * a));
    glm::vec3 closerPoint = rayOrigin + rayDirection * closerT;
    glm::vec3 fartherPoint = rayOrigin + rayDirection * fartherT;

    glm::vec3 normal = glm::normalize(closerPoint);
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
    // how strong the light will be
    // if cos value < 0, which means the angle between the normal and the -lightdir is too big
    // therefor causing the lightness to be very low
    float lightStrongness = glm::max(glm::dot(normal, -lightDir), 0.0f);
    sphereColor *= lightStrongness;

    return glm::vec4(sphereColor, 1);
}
