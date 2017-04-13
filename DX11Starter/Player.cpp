#include "Player.h"


#pragma once

Player::Player(Entity* e, RailSet* r)
{
	entity = e;
	rails = r;

	entity->SetScale({ defaultScale,defaultScale,defaultScale });
}


Player::~Player()
{
}

void Player::Update(float deltaTime) {
	int move = 0; //sum movement, if both left and right are held, it averages out

	//left and right user input
	if (GetAsyncKeyState('A') && 0x8000) {
		MoveLeft();
		move--;
	}
	if (GetAsyncKeyState('D') && 0x8000) {
		MoveRight();
		move++;
	}
	//revert to default rail when not moving
	if (defaultReset && move == 0) {
		MoveDefault();
	}
	prevMove = move; 

	//apply scaling when necessary
	if (currentScale != defaultScale) {
		currentScale -= deltaTime * animationSpeed;
		if (currentScale < defaultScale) { currentScale = defaultScale; }
		entity->SetScale({ currentScale,currentScale,currentScale });
	}
}

void Player::Hit() {
	currentScale += 1.0f;
}

//move one rail up or down sequence of rails
void Player::MoveLeft()
{
	if (currentRail > 0) {
		currentRail--;
		entity->SetPosition(rails->GetRail(currentRail)->GetAttachPoint());
	}
}
void Player::MoveRight()
{
	if (currentRail < RailSet::railCount-1) {
		currentRail++;
		entity->SetPosition(rails->GetRail(currentRail)->GetAttachPoint());
	}
}
//reset to default rail
void Player::MoveDefault()
{
	currentRail = defaultRail;
	entity->SetPosition(rails->GetRail(currentRail)->GetAttachPoint());
}