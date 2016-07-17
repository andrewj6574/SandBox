#pragma once

#include "Entity.h"

class Particle : public Entity
{
public:
	Particle(float x, float y);
	Particle(float x, float y, float angle);
	
	void update(float timeElapsed) override;
	void render(Renderer& renderer) override;

protected:
	float m_Duration;

};