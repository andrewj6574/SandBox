#include "player.h"

Player::Player(float x, float y)
	: Entity(glm::vec3(x, y, y), glm::vec2(32, 32), TextureManager::get("Textures/Player/player_anim.png")), 
	m_Sword(x + 2, y + 8), m_Gun(x + 10, y + 12), m_CollisionBox(x, y, 10, 22)
{
	m_Light = Sprite(glm::vec3(0, 0, 0), glm::vec2(256, 256), TextureManager::get("Textures/light2.png"));
	m_Shield = Sprite(glm::vec3(x, y, 0), glm::vec2(32, 32), TextureManager::get("Textures/Player/Shield.png"));
	//m_Crosshair = Sprite(glm::vec3(x, y, 0), glm::vec2(32, 32), TextureManager::get("Textures/Player/crosshair.png"));
	m_TexSize = 16;

	init();
}

void Player::init()
{
	m_State = PlayerState::NORMAL;
	m_Row = 0;
	m_CumulativeTime = 0.0f;

	m_MoveSpeed = 280.0f;
	m_AttackSpeed = 0.0f;//0.1f;
	m_AttackFrame = 0.0f;
	m_CurrentAttackDuration = 0.0f;
	m_AimDownSight = false;
	m_AimDownSightTime = 0.0f;
	m_AimDownSightZoom = 0.0f;
	m_CameraOffsetX = 0.0f;
	m_CameraOffsetY = 0.0f;

	m_Moving = false;

	setUV(1, 0, m_TexSize, m_TexSize);

	Camera& camera = Window::Instance().getCamera();
	camera.Position = glm::vec3(0, 0, 0);
}

bool Player::playerCollision(float dx, float dy, const std::unique_ptr<QTree<Sprite>>& quadTree)
{
	float x = m_CollisionBox.x + 2 + dx;
	float y = m_CollisionBox.y + dy;
	float w = m_CollisionBox.width - 5;
	float h = 1;			// collision with feet only for movement

	std::vector<std::shared_ptr<Sprite>> tiles;
	quadTree->retrieve(tiles, m_CollisionBox);

	for (auto& tile : tiles)
	{
		const auto& collisionBox = tile->getCollisionBox();
		float tx = collisionBox->x;
		float ty = collisionBox->y;
		float tw = collisionBox->width;
		float th = collisionBox->height;

		//if (tile->intersects(m_CollisionBox))
		if (Utils::quadCollision(x, y, w, h, tx, ty, tw, th))
		{
			return true;
		}
	}

	return false;
}

void Player::move(const std::unique_ptr<QTree<Sprite>>& quadTree, const std::unique_ptr<QTree<Sprite>>& waterQT, Region& region, float timeElapsed)
{
	Window& window = Window::Instance();

	//float moveSlow = 0.4f;
	float m_MoveSlow = Utils::lerp(1.0f, 0.3f, m_CurrentAttackDuration / 5.0f);
	m_Anim += timeElapsed * 25.0f * (m_CurrentAttackDuration > 0 ? m_MoveSlow: 1.0f);

	float dx = 0.0f;
	float dy = 0.0f;//-200.0f * timeElapsed;

	if (!m_Moving)
	{
		setUV(0, 0, m_TexSize, m_TexSize);
	}
	else
	{
		m_Moving = false;
	}

	if (window.isKeyPressed(GLFW_KEY_W))
	{
		m_Row = 3;
		setUV((int)m_Anim % 6, m_Row, m_TexSize, m_TexSize);
		dy += m_MoveSpeed * timeElapsed * (m_CurrentAttackDuration > 0 ? m_MoveSlow : 1.0f);
		m_Moving = true;
	}

	if (window.isKeyPressed(GLFW_KEY_A))
	{
		m_Row = 2;
		setUV((int)m_Anim % 8, m_Row, m_TexSize, m_TexSize);
		dx -= m_MoveSpeed * timeElapsed * (m_CurrentAttackDuration > 0 ? m_MoveSlow : 1.0f);
		m_Moving = true;
	}

	if (window.isKeyPressed(GLFW_KEY_S))
	{
		m_Row = 3;
		setUV((int)m_Anim % 6, m_Row, m_TexSize, m_TexSize);
		dy -= m_MoveSpeed * timeElapsed * (m_CurrentAttackDuration > 0 ? m_MoveSlow : 1.0f);
		m_Moving = true;
	}

	if (window.isKeyPressed(GLFW_KEY_D))
	{
		m_Row = 1;
		setUV((int)m_Anim % 8, m_Row, m_TexSize, m_TexSize);
		dx += m_MoveSpeed * timeElapsed * (m_CurrentAttackDuration > 0 ? m_MoveSlow : 1.0f);
		m_Moving = true;
	}

	if (!Settings::Instance().noClip)
	{
		if (playerCollision(dx, 0, quadTree) || region.getTileType(getCenterX() + dx, getY()) == TileType::VOID)
		{
			dx = 0.0f;
		}

		if (playerCollision(0, dy, quadTree) || region.getTileType(getCenterX(), getY() + dy) == TileType::VOID)
		{
			dy = 0.0f;
		}
	}

	if (region.getTileType(getCenterX(), getCenterY()) == TileType::SHALLOW_WATER)
	{
		dx *= 0.5f;
		dy *= 0.5f;
		setUV(4, 0, m_TexSize, m_TexSize);
	}

	if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT))
	{
		dx *= 10.0f;
		dy *= 10.0f;
	}

	if (window.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		dx *= 0.1f;
		dy *= 0.1f;
	}

	move(dx, dy);

	m_Light.setPosition(getCenterX() - m_Light.getSize().x / 2, getCenterY() - m_Light.getSize().y / 2);
}

void Player::aimDownSight(float timeElapsed)
{
	m_AimDownSight = Window::Instance().isButtonPressed(GLFW_MOUSE_BUTTON_2);
	
	float ADSTotalTime = 0.1f;
	if (Window::Instance().isButtonPressed(GLFW_MOUSE_BUTTON_2))
	{
		m_AimDownSightTime = std::fminf(ADSTotalTime, m_AimDownSightTime + timeElapsed);
	}
	else
	{
		m_AimDownSightTime = std::fmaxf(0.0f, m_AimDownSightTime - timeElapsed);
	}

	if (Settings::Instance().ADS)
	{
		float zoom = Utils::lerp(0.0f, 0.20f, m_AimDownSightTime / ADSTotalTime);
		Window::Instance().getCamera().Zoom = zoom;

		if (m_AimDownSight)
		{
			//float zoom = Utils::lerp(0.0f, 0.35f, m_AimDownSightTime / 2.0f);
			//Window::Instance().getCamera().Zoom = zoom;
			//Window::Instance().getCamera().processZoom(0.05);
		}
		else
		{
			//float zoom = Utils::lerp(0.35f, 0.0f, m_AimDownSightTime / 2.0f);
			//Window::Instance().getCamera().Zoom = zoom;
			//Window::Instance().getCamera().processZoom(-0.05);
		}
	}
}

void Player::shoot(float angle, float timeElapsed)
{
	m_AttackFrame += timeElapsed;

	// fire projectile
	if (Window::Instance().isButtonPressed(GLFW_MOUSE_BUTTON_1))// && m_AttackFrame > m_AttackSpeed)
	{

		if (m_AttackFrame > m_AttackSpeed)
		{
			float angleOffset = glm::radians(Utils::random(-m_CurrentAttackDuration, m_CurrentAttackDuration));
			float moveSlow = Utils::lerp(1.0f, 0.3f, m_CurrentAttackDuration / 5.0f);
			m_Gun.shoot(getPosition().x, getPosition().y, angle + angleOffset, m_Moving ? m_MoveSpeed * moveSlow : 0.0f);

			m_AttackFrame = 0.0f;
		}

		m_CurrentAttackDuration = std::fminf(5.0f, m_CurrentAttackDuration + timeElapsed * 5);
	}
	else if (m_CurrentAttackDuration > 0)
	{
		m_CurrentAttackDuration = std::fmaxf(0, m_CurrentAttackDuration - timeElapsed * 10);
	}
}

void Player::moveCamera()
{
	// move camera based on player position and mouse
	float px = getCenterX();// *1280.0f / Window::Instance().getWidth();
	float py = getCenterY();// *720.0f / Window::Instance().getHeight();

	float mx = Window::Instance().getMouseWorldPosX();
	float my = Window::Instance().getMouseWorldPosY();

	float mcw = 500.0f;
	float mch = 500.0f;
	float dcx = mx - px;
	float dcy = my - py;

	float cx = std::max(-mcw, std::min(dcx, mcw));
	float cy = std::max(-mch, std::min(dcy, mch));

	cx /= (mcw / 150.0f);
	cy /= (mcw / 150.0f);

	float ccx = px - Settings::Instance().PROJECTION_WIDTH / 2.0f;
	float ccy = py - Settings::Instance().PROJECTION_HEIGHT / 2.0f;

	Window::Instance().getCamera().Offset = glm::vec2(cx, cy);

	//Window::Instance().getCamera().moveCameraPosition(ccx + cx, ccy + cy);

	float mx1 = Window::Instance().getMouseWorldPosX();
	float mx2 = Window::Instance().getMouseWorldPosX(false);
	float my1 = Window::Instance().getMouseWorldPosY();
	float my2 = Window::Instance().getMouseWorldPosY(false);

	float dx = mx2 - mx1;
	float dy = my2 - my1;

	m_CameraOffsetX = cx;
	m_CameraOffsetY = cy;

	Window::Instance().getCamera().moveCameraPosition(ccx + cx + dx, ccy + cy + dy);
}

void Player::move(float dx, float dy)
{
	addDirection(dx, dy);
	m_Shield.addDirection(dx, dy);
	m_Sword.move(dx, dy);
	m_Gun.move(dx, dy);
	m_CollisionBox.x = getCenterX() - 5;
	m_CollisionBox.y = getPosition().y + 2;

	Camera& camera = Window::Instance().getCamera();
	camera.moveCamera(dx, dy);

	ResourceManager::getInstance().shader("outline_shader")->setUniform("view", camera.GetViewMatrix());
	ResourceManager::getInstance().shader("basic_shader")->setUniform("view", camera.GetViewMatrix());
	ResourceManager::getInstance().shader("lightShadow")->setUniform("view", camera.GetViewMatrix());
}

void Player::dodge()
{
	float dodgeDistance = 2.0f;
	float dx = dodgeDistance * std::cosf(m_DodgeAngle);
	float dy = dodgeDistance * std::sinf(m_DodgeAngle);
	float dodgeDuration = 0.5f;

	dx /= dodgeDuration;
	dy /= dodgeDuration;

	move(dx, dy);
	
	setAngle(m_Angle + glm::radians(-20.0f));

	if (m_CumulativeTime >= dodgeDuration)
	{
		m_State = PlayerState::NORMAL;
		setAngle(0.0f);
	}
}

void Player::update(Region& region, const std::unique_ptr<QTree<Sprite>>& quadTree, const std::unique_ptr<QTree<Sprite>>& waterQT, float timeElapsed)
{
	update(timeElapsed);
	move(quadTree, waterQT, region, timeElapsed);
	m_Gun.update(region, quadTree, timeElapsed);

	m_Position.z = -m_Position.y;
	if (m_Gun.getAngle() < 0)
	{
		m_Gun.setDepth(m_Position.z + 0.5f);
	}
	else
	{
		m_Gun.setDepth(m_Position.z - 0.5f);
	}

	Window& win = Window::Instance();
	//std::cout << m_Position.z << "\n";
	moveCamera();
}

void Player::update(float timeElapsed)
{
	Window& window = Window::Instance();
	float mouseX = Window::Instance().mouseX();
	float mouseY = Window::Instance().mouseY();

	//float dx = mouseX - m_X;
	//float dy = mouseY - m_Y;
	//float angle = -std::atan2f(dy, dx);

	float dx = mouseX - Window::Instance().getWidth() / 2 - 16.0f + 2;
	float dy = mouseY - Window::Instance().getHeight() / 2 - 16.0f + 8;
	float angle = -std::atan2f(dy, dx);// -glm::radians(45.0f);

	float mx = Window::Instance().getMouseWorldPosX();
	float my = Window::Instance().getMouseWorldPosY();
	dx = mx - getCenterX();
	dy = my - getCenterY();
	angle = std::atan2f(dy, dx);

	aimDownSight(timeElapsed);
	shoot(angle, timeElapsed);
}

void Player::submit(Renderer& renderer)
{
	//glm::mat4 transform;
	//transform = glm::translate(transform, glm::vec3(getPosition().x + getSize().x / 2.0f, getPosition().y + getSize().y / 2.0f, 0));
	//transform = glm::rotate(transform, getAngle(), glm::vec3(0, 0, 1));
	//transform = glm::translate(transform, glm::vec3(-getPosition().x - getSize().x / 2.0f, -getPosition().y - getSize().y / 2.0f, 0));

	//if (m_Row == 3)
	//{
	//	m_Gun.submit(renderer);
	//}

	//renderer.push(transform);
	//renderer.end();
	//renderer.flush();
	//renderer.render(*this);
	//renderer.pop();
	//renderer.begin();

	//if (m_Row != 3)
	//{
	//	m_Gun.submit(renderer);
	//}
}

void Player::render(Renderer& renderer)
{
	float scale = 1.1f;
	glm::mat4 transform;
	glm::mat4 outline;
	outline = glm::translate(outline, glm::vec3(getPosition().x + getSize().x / 2.0f, getPosition().y + getSize().y / 2.0f, 0));
	outline = glm::scale(outline, glm::vec3(scale, scale, scale));
	outline = glm::rotate(outline, getAngle(), glm::vec3(0, 0, 1));
	outline = glm::translate(outline, glm::vec3(-getPosition().x - getSize().x / 2.0f, -getPosition().y - getSize().y / 2.0f, 0));

	transform = glm::translate(transform, glm::vec3(getPosition().x + getSize().x / 2.0f, getPosition().y + getSize().y / 2.0f, 0));
	transform = glm::rotate(transform, getAngle(), glm::vec3(0, 0, 1));
	transform = glm::translate(transform, glm::vec3(-getPosition().x - getSize().x / 2.0f, -getPosition().y - getSize().y / 2.0f, 0));

	renderer.push(transform);
	renderer.render(*this);
	renderer.pop();

	m_Gun.render(renderer);

	//if (m_ShieldActive)
	//{
	//	transform = glm::mat4();
	//	transform = glm::translate(transform, glm::vec3(m_Shield.getPosition().x + m_Shield.getSize().x / 2.0f, m_Shield.getPosition().y + m_Shield.getSize().y / 2.0f, 0));
	//	transform = glm::rotate(transform, m_Shield.getAngle(), glm::vec3(0, 0, 1));
	//	transform = glm::translate(transform, glm::vec3(-m_Shield.getPosition().x - m_Shield.getSize().x / 2.0f, -m_Shield.getPosition().y - m_Shield.getSize().y / 2.0f, 0));
	//	renderer.push(transform);
	//	renderer.render(m_Shield);
	//	renderer.pop();
	//}

	ResourceManager::getInstance().shader("basic_shader")->use();
	//renderer.render(m_Crosshair);

	if (Settings::Instance().debugShowCollisionBoxes)
	{
		//renderer.render(m_CollisionBox, TextureManager::get("Textures/collision_box.png"));
		renderer.render(Sprite(glm::vec3(m_CollisionBox.x, m_CollisionBox.y, getY() + 1), glm::vec2(m_CollisionBox.width, m_CollisionBox.height), TextureManager::get("Textures/collision_box.png")));
	}

}

void Player::renderLight(Renderer& renderer)
{
	renderer.submit(m_Light);

	m_Gun.renderLight(renderer);

}
