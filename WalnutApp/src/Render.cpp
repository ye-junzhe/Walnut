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

void Renderer::Render(const Scene& scene, const Camera& camera) {

    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
            glm::vec4 color = TraceRay(scene, ray);
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->SetData(m_ImageData);
}

void Renderer::RenderSSAA(float SSAA_level, const Camera& camera, const Scene& scene) {
    Ray ray;
    ray.Origin = camera.GetPosition();

    for (uint32_t y = 0; y < m_FinalImage->GetWidth() * SSAA_level; y++) {
        for (uint32_t x = 0; x < m_FinalImage->GetHeight() * SSAA_level; x++) {
            uint32_t sampleCount = 0;
            glm::vec4 color(0.0f);

            // Generate multiple rays per pixel
            for (uint32_t subY = -SSAA_level; subY <= SSAA_level; subY += SSAA_level) {
                for (uint32_t subX = -SSAA_level; subX <= SSAA_level; subX += SSAA_level) {
                    uint32_t sampleX = (x + subX);
                    uint32_t sampleY = (y + subY);

                    if (sampleX >= 0 && sampleX < (m_FinalImage->GetWidth()) &&
                        sampleY >= 0 && sampleY < (m_FinalImage->GetHeight())) {
                        ray.Direction = camera.GetRayDirections()[sampleX + sampleY * m_FinalImage->GetWidth()];
                        color += TraceRay(scene, ray);
                        sampleCount++;
                    }
                }
            }

            color /= sampleCount;
            color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    m_FinalImage->Resize(m_FinalImage->GetWidth(), m_FinalImage->GetHeight());
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
glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

    if (scene.Spheres.size() == 0)
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    const Sphere* closestSphere = nullptr;
    float hitDistance = std::numeric_limits<float>::max();

    for (const Sphere& sphere : scene.Spheres) {
        glm::vec3 origin = ray.Origin - sphere.Position;

        float a = glm::dot(ray.Direction, ray.Direction);
        float b = 2.0f * glm::dot(origin, ray.Direction);
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f) {
            continue; // move to next sphere
        }
        float closerT = ((-b - glm::sqrt(discriminant)) / (2.0f * a));
        /*
            The farther point which is not using currently
            float fartherT = ((-b + glm::sqrt(discriminant)) / (2.0f * a));
            glm::vec3 fartherPoint = origin + ray.Direction * fartherT;
        */
        if (closerT < hitDistance) {
            hitDistance = closerT;
            closestSphere = &sphere;
        }
    }
    if (closestSphere == nullptr)
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::vec3 origin = ray.Origin - closestSphere -> Position;
    glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
    // The normal on the sphere of the hit point
    glm::vec3 normal = glm::normalize(hitPoint);

    // The direction of the light
    glm::vec3 lightDir = scene.lightDir;
    // how strong the light will be
    // if cos value < 0, which means the angle between the normal and the -lightdir is too big
    // therefor causing the lightness to be very low
    float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f);

    glm::vec3 sphereColor = closestSphere->Albedo;
    sphereColor *= lightIntensity;

    return glm::vec4(sphereColor, 1);
}
