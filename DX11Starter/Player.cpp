#include "Player.h"


#pragma once

Player::Player(Entity* e, Rail** r)
{
	entity = e;
	rails = r;
}


Player::~Player()
{
	for (int i = 0; i < railCount; i++) {
		delete rails[i];
	}
	delete rails;
}

void Player::Update() {
	int move = 0;
	if (GetAsyncKeyState('A') && 0x8000) {
		MoveLeft();
		move--;
	}
	if (GetAsyncKeyState('D') && 0x8000) {
		MoveRight();
		move++;
	}

	if (defaultReset && move == 0) {
		MoveDefault();
	}
	prevMove = move; 
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