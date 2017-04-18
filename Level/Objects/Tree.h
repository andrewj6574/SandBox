#pragma once

#include "..\..\Graphics\Sprite.h"
#include "..\..\Entity\Entity.h"

class Tree : public Entity
{
public:
	Tree(int x, int y);
	~Tree() {}

	void damage(int amount) override { m_Destroy = true; }

	void update(float timeElapsed) override;
	void submit(Renderer& renderer) const override;

private:

};