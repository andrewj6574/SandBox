#include "Gun.h"

Gun::Gun(float x, float y)
	: Entity(glm::vec3(x, y, 0), glm::vec2(32, 6), TextureManager::get("Textures/Player/Gun4.png"))
{
	//m_Sprite.setUV(0, 0, 10, 3);
	//m_Sprite.addDirection(10, 0);
	m_Sprite.setUV(0, 0, 16, 3);
	m_FaceRight = true;
}

void Gun::shoot(float x, float y, float angle, float movespeed)
{
	glm::mat4 transform;

	int rtx = 3;
	int rty = 3;
	transform = glm::translate(transform, glm::vec3(m_Sprite.getPosition().x + rtx, m_Sprite.getPosition().y + rty, 0));
	transform = glm::rotate(transform, m_Sprite.getAngle(), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(-m_Sprite.getPosition().x - rtx, -m_Sprite.getPosition().y - rty, 0));
	const glm::vec3& pos = glm::vec3(m_Sprite.getPosition().x + m_Sprite.getSize().x - 12, m_Sprite.getPosition().y + m_Sprite.getSize().y - rty, 0);
	glm::vec4 gunPos = transform * glm::vec4(pos, 1.0f);

	//float mx = Window::Instance().mouseX() + Window::Instance().getCamera().getPosition().x;
	//float my = Window::Instance().mouseY() + Window::Instance().getCamera().getPosition().y;
	float mx = Window::Instance().getMouseWorldPosX();
	float my = Window::Instance().getMouseWorldPosY();

	//std::cout << gunPos.x << ", " << gunPos.y << "\n";

	float dx = mx - gunPos.x;
	float dy = my - gunPos.y;

	float a = std::atan2f(dy, dx);

	for (int i = 0; i < 150; i++) m_Entities.push_back(std::unique_ptr<GunParticle>(new GunParticle(gunPos.x, gunPos.y, angle, movespeed)));
	m_Bullets.push_back(std::unique_ptr<Bullet>(new Bullet(gunPos.x, gunPos.y, angle)));
}

void Gun::move(float x, float y)
{
	m_Sprite.addDirection(x, y);
}

void Gun::update(Region& region, const std::unique_ptr<QTree<BoundingBox>>& quadTree, float timeElapsed)
{
	float mx = Window::Instance().mouseX();
	float my = Window::Instance().mouseY();

	float dx = mx - Window::Instance().getWidth() / 2 - 16.0f + 2;
	float dy = my - Window::Instance().getHeight() / 2 - 16.0f + 8;
	float angle = -std::atan2f(dy, dx);// -glm::radians(45.0f);

	mx = Window::Instance().getMouseWorldPosX();
	my = Window::Instance().getMouseWorldPosY();
	dx = mx - getCenterX();
	dy = my - getCenterY();
	angle = std::atan2f(dy, dx);

	if (m_FaceRight && (std::abs(glm::degrees(angle)) >= 90.0f))
	{
		//m_Sprite.setUV(0, 1, 10, 3);
		m_Sprite.addDirection(5, 0);
		m_Sprite.setUV(0, 1, 16, 3);
		m_FaceRight = false;
	}
	else if (!m_FaceRight && (std::abs(glm::degrees(angle)) < 90.0f))
	{
		//m_Sprite.setUV(0, 0, 10, 3);
		m_Sprite.addDirection(-5, 0);
		m_Sprite.setUV(0, 0, 16, 3);
		m_FaceRight = true;
	}


	m_Sprite.setAngle(angle);

	//for (int i = 0; i < 150; i++) m_Entities.push_back(std::unique_ptr<GunParticle>(new GunParticle(m_Sprite.getPosition().x, m_Sprite.getPosition().y, angle)));

	for (auto& bullet : m_Bullets)
	{
		bullet->update(timeElapsed);
		std::vector<std::shared_ptr<BoundingBox>> collisionBoxes;

		const glm::vec3& pos = bullet->getSprite().getPosition();
		const glm::vec2& size = bullet->getSprite().getSize();
		
		quadTree->retrieve(collisionBoxes, BoundingBox(pos.x, pos.y, size.x, size.y));
		//quadTree->retrieve(collisionBoxes, *(bullet->getCollisionBox()));

		for (auto& tile : collisionBoxes)
		{
			float ex = tile->x;
			float ey = tile->y;
			float ew = tile->width;
			float eh = tile->height;

			float sx = pos.x;
			float sy = pos.y;
			float sw = size.x;
			float sh = size.y;

			//if (sx > ex && sx < ex + ew && sy > ey && sy < ey + eh)
			//if (bullet->collide(*tile))
			if (tile->intersects(*(bullet)->getCollisionBox()))
			{
				for (int i = 0; i < 25; i++)
				{
					m_Entities.push_back(std::unique_ptr<Particle>(new Particle(sx, sy, bullet->getAngle())));
				}
				//enemy->damage(10);
				//m_DamageText.push_back(std::unique_ptr<DamageCounter>(new DamageCounter("1", sx, sy)));


				region.removeTiles(ex, ey, true, true);
				bullet->setDestroy(true);
				break;
			}
		}
	}

	for (auto it = m_Entities.begin(); it != m_Entities.end(); )
	{
		if ((*it)->shouldDestroy())
		{
			it = m_Entities.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto it = m_Bullets.begin(); it != m_Bullets.end(); )
	{
		if ((*it)->shouldDestroy())
		{
			it = m_Bullets.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto it = m_DamageText.begin(); it != m_DamageText.end(); )
	{
		if ((*it)->shouldDestroy())
		{
			it = m_DamageText.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (auto& text : m_DamageText)
	{
		text->update(timeElapsed);
	}

	for (auto& particle : m_Entities)
	{
		particle->update(timeElapsed);
	}
}

void Gun::submit(Renderer& renderer)
{
	renderer.push(glm::mat4(), true);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (auto& bullet : m_Bullets) bullet->submit(renderer);
	for (auto& entity : m_Entities) entity->submit(renderer);
	//renderer.end();
	//renderer.flush();
	//renderer.begin();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (auto& text : m_DamageText)
	{
		text->render(renderer);
	}

	renderer.pop();

	glm::mat4 transform;
	transform = glm::translate(transform, glm::vec3(m_Sprite.getPosition().x + 5, m_Sprite.getPosition().y + 5, 0));
	transform = glm::rotate(transform, m_Sprite.getAngle(), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(-m_Sprite.getPosition().x - 5, -m_Sprite.getPosition().y - 5, 0));

	renderer.push(transform);
	renderer.submit(m_Sprite);
	renderer.pop();
}

void Gun::render(Renderer& renderer)
{
	renderer.push(glm::mat4(), true);
	renderer.render(m_Bullets);
	renderer.render(m_Entities);

	for (auto& text : m_DamageText)
	{
		text->render(renderer);
	}

	renderer.pop();

	int rtx = 3;
	int rty = 3;
	glm::mat4 transform;
	transform = glm::translate(transform, glm::vec3(m_Sprite.getPosition().x + rtx, m_Sprite.getPosition().y + rty, 0));
	transform = glm::rotate(transform, m_Sprite.getAngle(), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(-m_Sprite.getPosition().x - rtx, -m_Sprite.getPosition().y - rty, 0));

	
	renderer.push(transform);
	renderer.render(*this);
	renderer.pop();


	for (auto& b : m_Bullets)
	{
		renderer.render(*(b->getCollisionBox()), TextureManager::get("Textures/collision_box.png"));
	}

}

void Gun::renderLight(Renderer& renderer)
{
	for (auto& bullet : m_Bullets)
	{
		bullet->renderLight(renderer);
	}
}
