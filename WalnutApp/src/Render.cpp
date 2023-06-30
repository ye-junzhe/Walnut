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

void Renderer::Render(const Camera& camera) {

    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
            glm::vec4 color = TraceRay(ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->SetData(m_ImageData);
}

void Renderer::RenderSSAA(float SSAA_level, const Camera& camera) {
    uint32_t width = m_FinalImage->GetWidth();
    uint32_t height = m_FinalImage->GetHeight();
    uint32_t newWidth = width * SSAA_level;
    uint32_t newHeight = height * SSAA_level;
    float invSSAA_level = 1.0f / SSAA_level;

    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < newHeight; y++) {
        for (uint32_t x = 0; x < newWidth; x++) {
            uint32_t sampleCount = 0;
            glm::vec4 color(0.0f);

            // Generate multiple rays per pixel
            for (float subY = 0; subY < SSAA_level; subY += 1.0f) {
                for (float subX = 0; subX < SSAA_level; subX += 1.0f) {
                    float sampleX = (x + subX) * invSSAA_level;
                    float sampleY = (y + subY) * invSSAA_level;
                    ray.Direction = camera.GetRayDirections()[(uint32_t)sampleX + (uint32_t)sampleY * width];
                    color += TraceRay(ray);
                    sampleCount++;
                }
            }

            color /= sampleCount;
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * newWidth] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->Resize(newWidth, newHeight);
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


// WARNING: Negative is farther
glm::vec4 Renderer::TraceRay(const Ray& ray)
{
	float radius = 0.5f;
    glm::vec3 sphereColor(1, 0 ,1);
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(ray.Origin, ray.Direction);
	float c = glm::dot(ray.Origin, ray.Origin) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return glm::vec4(0, 0, 0, 1);
    }

    float closerT = ((-b - glm::sqrt(discriminant)) / (2.0f * a));
    float fartherT = ((-b + glm::sqrt(discriminant)) / (2.0f * a));
    glm::vec3 closerPoint = ray.Origin + ray.Direction * closerT;
    glm::vec3 fartherPoint = ray.Origin + ray.Direction * fartherT;

    glm::vec3 normal = glm::normalize(closerPoint);
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
    // how strong the light will be
    // if cos value < 0, which means the angle between the normal and the -lightdir is too big
    // therefor causing the lightness to be very low
    float lightStrongness = glm::max(glm::dot(normal, -lightDir), 0.0f);
    sphereColor *= lightStrongness;

    return glm::vec4(sphereColor, 1);
}
