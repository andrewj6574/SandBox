#include "Bullet.h"

Bullet::Bullet(float x, float y, float angle)
	: Entity(glm::vec3(x, y, 0), glm::vec2(8, 8), TextureManager::get("Textures/Bullet2.png"))
{
	m_Angle = angle;

	float velocity = 1000;
	m_Dx = std::cosf(angle) * velocity;
	m_Dy = std::sinf(angle) * velocity;

	m_Duration = 2.0f;

	m_Light = Sprite(glm::vec3(0, 0, 0), glm::vec2(32, 32), TextureManager::get("Textures/light2.png"));
}

float Bullet::getAngle() const
{
	return m_Angle;
}

void Bullet::update(float timeElapsed)
{
	m_Duration -= timeElapsed;

	m_Sprite.addDirection(m_Dx * timeElapsed, m_Dy * timeElapsed);
	m_CollisionBox->x = m_Sprite.getPosition().x;
	m_CollisionBox->y = m_Sprite.getPosition().y;

	if (m_Duration <= 0)
	{
		m_Destroy = true;
	}

	m_Light.setPosition(getCenterX() - m_Light.getSize().x / 2, getCenterY() - m_Light.getSize().y / 2);
}

void Bullet::render(Renderer& renderer)
{
	renderer.render(*this);
}

void Bullet::renderLight(Renderer& renderer)
{
	renderer.submit(m_Light);
}
