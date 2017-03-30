#include "Player.h"


#pragma once

Player::Player(Entity* e, Rail** r)
{
	entity = e;
	rails = r;
}


Player::~Player()
{
}

//move one rail up or down sequence of rails
void Player::MoveLeft()
{
	if (currentRail > 0) {
		currentRail--;
		entity->SetPosition(rails[currentRail]->GetAttachPoint());
	}
}

void Player::MoveRight()
{
	if (currentRail < railCount-1) {
		currentRail++;
		entity->SetPosition(rails[currentRail]->GetAttachPoint());
	}
}
void Player::MoveDefault()
{
	currentRail = defaultRail;
	entity->SetPosition(rails[currentRail]->GetAttachPoint());
}