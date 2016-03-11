#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include "ResourcePath.h"
#include "Overlap.h"

// Gameplay settings.
const float SHIP_VELOCITY = 10.f;
const float BULLET_VELOCITY = 4.0f;
const int HEALTH = 10;
// Health bar settings.
const int HEALTH_BAR_WIDTH = 100;
const int HEALTH_BAR_HEIGHT = 10;
// Window settings.
const int VIDEO_WIDTH = 1000;
const int VIDEO_HEIGHT = 600;
const int FRAME_LIMIT = 60;
// Title sceen settings.
const float TITLE_BACKGROUND_SCALE_X = .4;
const float TITLE_BACKGROUND_SCALE_Y = .4;
const int INSTRUCTIONS_POS_X = 200;
const int INSTRUCTIONS_POS_Y = 300;
// Player sprite and position settings.
const int START_Y1 = 25;
const int START_Y2 = 440;
const int START_X1 = (VIDEO_WIDTH / 2);
const int START_X2 = START_X1;
const float SHIP_SCALE_X = .1f;
const float SHIP_SCALE_Y = .1f;
const float BULLET_SCALE_X = .10f;
const float BULLET_SCALE_Y = .10f;
// Shot cooldown settings.
const float MAX_SHOT_COOLDOWN = 12.f;
const float MIN_SHOT_COOLDOWN = .3f;
const float SHOT_COOLDOWN_INC = .0005f;	// rate at which bullets slow down (when not moving)
const float SHOT_DECAY_MULTIPLIER = 4;	// how much faster bullets will speed up from moving 
										// compared to slow down while staying still. (when moving)
										// Result scene settings.
const float RESULT_SCREEN_DELAY = 2.5f;
const int RESULT_TEXT_POS_X = 420;
const int RESULT_TEXT_POS_Y = 500;

enum gameScene {
	start,
	gameplay,
	result
};

struct Bullet {
	sf::Sprite sprite;
	bool facingUp;
	bool collided;
};

struct Assets {
	sf::Texture ship;
	sf::Texture bulletUp;
	sf::Texture bulletDown;
	sf::Texture background;
	sf::Texture instructions;
	sf::Texture gameBckground;
	sf::Sprite ocean;
	std::list<Bullet>* bullets;
};

struct Player {
	sf::Sprite sprite;
	sf::Clock shootingClock;
	int health;
	bool playerHit;
	float cooldownRate;	// time between bullets spawned.
	sf::RectangleShape healthBar;
	bool moved;
	bool startTrigger;
};

void initializeTitleScreen(sf::Sprite &titleScreen, sf::Sprite &titleInstructions, Assets &assets);
void loadAssets(Assets &assets, std::list<Bullet> &bullets);
void initializePlayerSettings(Player &player1, Player &player2, Assets &assets);
bool willBeInBounds(sf::Sprite& sprite, sf::Vector2f offset);
void movePlayers(Player &player1, Player &player2, Assets &assets);
void checkCollisions(std::list<Bullet> &bullets, Player &player1, Player &player2);
void removeBullets(std::list<Bullet> &bullets);
void drawBullets(sf::RenderWindow &window, Assets &assets);
void drawPlayers(sf::RenderWindow &window, Player &player1, Player &player2, gameScene &scene);
void showResults(sf::RenderWindow &window, Player &p1, Player &p2);
void changeCooldownRates(Player &player1, Player &player2);

/********************************************* Main Function *********************************************/
int main()
{
	// INITIALIZAION
	gameScene scene = start;
	sf::RenderWindow window(sf::VideoMode(VIDEO_WIDTH, VIDEO_HEIGHT), "Toasty Duels!");
	window.setFramerateLimit(FRAME_LIMIT);

	// declare an STL linked list of Bullets
	std::list<Bullet> bullets;

	// Load assets
	Assets assets;
	loadAssets(assets, bullets);

	// Title screen settings
	/************************************* TITLE SCREEN TEXTURE HERE*************************************/
	sf::Sprite titleScreen;
	sf::Sprite titleInstructions;
	initializeTitleScreen(titleScreen, titleInstructions, assets);

	// Initialize Player settings
	Player player1, player2;
	initializePlayerSettings(player1, player2, assets);

	// GAME LOOP
	while (window.isOpen())
	{
		sf::Event event;

		if (scene == gameplay)
		{
			movePlayers(player1, player2, assets);

		}

		while (window.pollEvent(event))
		{
			// HANDLE EVENTS
			switch (scene)
			{
			case start:
				// Trigger gameplay when "Enter" is pressed
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
					scene = gameplay;
				break;
			case gameplay:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
					initializePlayerSettings(player1, player2, assets);
					scene = start;
				}
				break;
			case result:
				// Restart game when "Enter is pressed
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
					initializePlayerSettings(player1, player2, assets);
					scene = start;
				}
				break;
			}

			if (event.type == sf::Event::Closed)
				window.close();
		}

		// SIMULATE WORLD
		if (scene == gameplay)
			checkCollisions(bullets, player1, player2);
		// clear window
		window.clear();

		// Draw game
		switch (scene)
		{
		case start:
			window.draw(titleScreen);
			window.draw(titleInstructions);
			break;
		case gameplay:
			drawBullets(window, assets);
			removeBullets(bullets);
			drawPlayers(window, player1, player2, scene);
			break;
		case result:
			sf::Clock resultClock;
			while (resultClock.getElapsedTime().asSeconds() < RESULT_SCREEN_DELAY)
			{
				//std::cout << resultClock.getElapsedTime().asSeconds() << std::endl;
				window.clear();
				showResults(window, player1, player2);
				window.display();
			}
			resultClock.restart();
			initializePlayerSettings(player1, player2, assets);
			scene = start;
			break;
		}
		window.display();
	}

	return 0;
}

/******************************************** Game Functions ********************************************/

/*This function draws the result screen*/
void showResults(sf::RenderWindow &window, Player &p1, Player &p2)
{
	/**************************************** RESULT SCREEN TEXTURE *************************************/
	sf::Texture shipExplosion;
	shipExplosion.loadFromFile(resourcePath() + "assets/resultScene.jpg");
	sf::Sprite resultScene;
	resultScene.setTexture(shipExplosion);
	/**************************************** RESULT SCREEN TEXTURE *************************************/

	window.draw(resultScene);
	/**************************************** FONTS HERE *************************************/
	sf::Font myFont;
	myFont.loadFromFile(resourcePath() + "/assets/arial.ttf");
	sf::Text resultText;
	resultText.setFont(myFont);

	if (p1.health == 0) {
		resultText.setString("Player 2 Wins!");
	}
	else
		resultText.setString("Player 1 Wins!");

	resultText.setColor(sf::Color::Red);
	resultText.setPosition(RESULT_TEXT_POS_X, RESULT_TEXT_POS_Y);
	/**************************************** FONTS HERE *************************************/

	window.draw(resultText);

}

/*This function sets up the title screen.*/
void initializeTitleScreen(sf::Sprite &titleScreen, sf::Sprite &titleInstructions, Assets &assets)
{
	/**************************************** TITLE SCREEN TEXTURE HERE *************************************/
	// Create background image
	titleScreen.setTexture(assets.background);
	titleScreen.setScale(TITLE_BACKGROUND_SCALE_X, TITLE_BACKGROUND_SCALE_Y);
	// Create instruction textbox
	assets.instructions.loadFromFile(resourcePath() + "assets/instructions.png");
	titleInstructions.setTexture(assets.instructions);
	titleInstructions.setPosition(INSTRUCTIONS_POS_X, INSTRUCTIONS_POS_Y);
}

/*This function loads the textures from assets folder.*/
void loadAssets(Assets &assets, std::list<Bullet> &bullets)
{
	// Title screen assets
	assets.background.loadFromFile(resourcePath() + "assets/battleshipTitle.jpg");
	// Game assets
	assets.ship.loadFromFile(resourcePath() + "assets/battleship.png");
	assets.bulletDown.loadFromFile(resourcePath() + "assets/bulletDown.png");
	assets.bulletUp.loadFromFile(resourcePath() + "assets/bulletUp.png");
	assets.gameBckground.loadFromFile(resourcePath() + "assets/gameBackground.jpg");
	assets.ocean.setTexture(assets.gameBckground);
	// Assign bullets linked list to assets
	assets.bullets = &bullets;
}

/*This function will initialize player settings, such as sprite, cooldownRate, health etc,
for both player1 and player2*/
void initializePlayerSettings(Player &player1, Player &player2, Assets &assets)
{
	// Initialize player1
	player1.sprite.setTexture(assets.ship);
	player1.sprite.setScale(sf::Vector2f(SHIP_SCALE_X, SHIP_SCALE_Y));
	player1.sprite.setPosition(sf::Vector2f(START_X1, START_Y1));
	player1.playerHit = false;
	player1.health = HEALTH;
	player1.cooldownRate = MIN_SHOT_COOLDOWN;
	player1.healthBar.setFillColor(sf::Color::Red);
	player1.startTrigger = false;

	// Initialize player2
	player2.sprite.setTexture(assets.ship);
	player2.sprite.setScale(sf::Vector2f(SHIP_SCALE_X, SHIP_SCALE_Y));
	player2.sprite.setPosition(sf::Vector2f(START_X2, START_Y2));
	player2.playerHit = false;
	player2.health = HEALTH;
	player2.cooldownRate = MIN_SHOT_COOLDOWN;
	player2.healthBar.setPosition(sf::Vector2f(0, VIDEO_HEIGHT - HEALTH_BAR_HEIGHT));
	player2.healthBar.setFillColor(sf::Color::Yellow);
	player2.startTrigger = false;

	// Clear bullets on screen
	assets.bullets->clear();
}

/*This function check if the sprites are within the screen boundaries*/
bool willBeInBounds(sf::Sprite& sprite, sf::Vector2f offset) {
	sf::FloatRect bounds = sprite.getGlobalBounds();

	if ((bounds.top + offset.y) < 0) {
		return false;
	}
	if ((bounds.left + offset.x) < 0) {
		return false;
	}
	if ((bounds.left + bounds.width + offset.x) > VIDEO_WIDTH) {
		return false;
	}
	if ((bounds.top + bounds.height + offset.y) > VIDEO_HEIGHT) {
		return false;
	}
	return true;
}

/*This function handles controls, move the player ship sprites and spawn bullets.*/
void movePlayers(Player &player1, Player &player2, Assets &assets)
{
	// HANDLE CONTROLS
	sf::Keyboard::Key left = sf::Keyboard::Left;
	sf::Keyboard::Key right = sf::Keyboard::Right;
	sf::Keyboard::Key A = sf::Keyboard::A;
	sf::Keyboard::Key D = sf::Keyboard::D;
	sf::Keyboard::Key space = sf::Keyboard::Space;
	sf::Keyboard::Key shift = sf::Keyboard::RShift;
	sf::Vector2f rightBoundary = sf::Vector2f(-SHIP_VELOCITY, 0);
	sf::Vector2f leftBoundary = sf::Vector2f(SHIP_VELOCITY, 0);

	// Initialize move settings

	player1.moved = false;
	player2.moved = false;

	// Move player1
	if (sf::Keyboard::isKeyPressed(left) && willBeInBounds(player1.sprite, rightBoundary))
	{
		player1.sprite.move(-SHIP_VELOCITY, 0);
		player1.moved = true;
	}
	if (sf::Keyboard::isKeyPressed(right) && willBeInBounds(player1.sprite, leftBoundary))
	{
		player1.sprite.move(SHIP_VELOCITY, 0);
		player1.moved = true;
	}
	if (sf::Keyboard::isKeyPressed(left) && sf::Keyboard::isKeyPressed(right))
	{
		player1.moved = false;
	}

	// Spawn bullets for player1
	if (sf::Keyboard::isKeyPressed(shift) && player1.shootingClock.getElapsedTime().asSeconds() > player1.cooldownRate)
	{
		int x = (player1.sprite.getPosition().x) + ((player1.sprite.getGlobalBounds().width) / 2);	// get x-coordinate of player1

		Bullet bltDown;
		bltDown.sprite.setTexture(assets.bulletDown);
		bltDown.sprite.setScale(sf::Vector2f(BULLET_SCALE_X, BULLET_SCALE_Y));
		bltDown.facingUp = false;
		bltDown.collided = false;
		bltDown.sprite.setPosition(x, (player1.sprite.getGlobalBounds().top + player1.sprite.getGlobalBounds().height));
		assets.bullets->push_back(bltDown);
		player1.shootingClock.restart();
	}
	// Move player2
	if (sf::Keyboard::isKeyPressed(A) && willBeInBounds(player2.sprite, rightBoundary))
	{
		player2.sprite.move(-SHIP_VELOCITY, 0);
		player2.moved = true;
	}
	if (sf::Keyboard::isKeyPressed(D) && willBeInBounds(player2.sprite, leftBoundary))
	{
		player2.sprite.move(SHIP_VELOCITY, 0);
		player2.moved = true;
	}
	if (sf::Keyboard::isKeyPressed(A) && sf::Keyboard::isKeyPressed(D))
	{
		player2.moved = false;
	}

	// Spawn bullets for player2
	if (sf::Keyboard::isKeyPressed(space) && player2.shootingClock.getElapsedTime().asSeconds() > player2.cooldownRate)
	{
		int x = (player2.sprite.getPosition().x) + ((player2.sprite.getGlobalBounds().width) / 2);	// get x-coordinate of player2

		Bullet bltUp;
		bltUp.sprite.setTexture(assets.bulletUp);
		bltUp.sprite.setScale(sf::Vector2f(BULLET_SCALE_X, BULLET_SCALE_Y));
		bltUp.facingUp = true;
		bltUp.collided = false;
		bltUp.sprite.setPosition(x, START_Y2);
		assets.bullets->push_back(bltUp);
		player2.shootingClock.restart();
	}

	// Trigger startTrigger when player is moved initially
	//	if (!player1.startTrigger)
	//		if (player1.moved)
	//			player1.startTrigger = true; they stay true forever after a person moves once
	//	if (!player2.startTrigger)
	//		if (player2.moved)
	//			player2.startTrigger = true;

	changeCooldownRates(player1, player2);
}

/*This function removes collided or out of bounds bullets from the bullets linked list.*/
void removeBullets(std::list<Bullet> &bullets)
{
	// Remove collided bullets from list.
	for (std::list<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
	{
		if (it->collided) {
			bullets.erase(it);
		}
		break;
	}

	// Remove old bullets from list.
	for (std::list<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
	{
		sf::Vector2f bulletBoundary = it->facingUp ? sf::Vector2f(0, -BULLET_VELOCITY) : sf::Vector2f(0, BULLET_VELOCITY);
		if (!willBeInBounds(it->sprite, bulletBoundary))
		{
			bullets.erase(it);
			break;
		}
	}
}

/*This function checks for three types of collisions in the game: bullet-bullet collision,
bullet-player1 collision and bullet-player2 collision.*/
void checkCollisions(std::list<Bullet> &bullets, Player &player1, Player &player2)
{
	// Create an iterator to traverse the list 
	for (std::list<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
	{
		if (it->facingUp)
			it->sprite.move(0, -BULLET_VELOCITY);
		if (!(it->facingUp))
			it->sprite.move(0, BULLET_VELOCITY);
	}

	// Compare bullets to check for collision
	for (std::list<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ++it)
	{
		// Check for bullet-ship1 collision
		if (it->facingUp && overlap(it->sprite, player1.sprite)) {
			it->collided = true;
			player1.playerHit = true;
			player1.health--;
			continue;
		}
		// Check for bullet-ship2 collision
		else if (!(it->facingUp) && overlap(it->sprite, player2.sprite)) {
			it->collided = true;
			player2.playerHit = true;
			player2.health--;
			continue;
		}

		// Check for bullet-bullet collision
		for (std::list<Bullet>::iterator iterator = bullets.begin(); iterator != bullets.end(); ++iterator) {
			// Avoid comparing the same bullet
			if (it == iterator)
				break;
			// Ignore comparing bullets spawned by same ship
			if (it->facingUp == iterator->facingUp)
				break;
			if (overlap(it->sprite, iterator->sprite)) {
				it->collided = true;
				iterator->collided = true;
			}
		}
	}
}

/*This function draws the bullet to the window.*/
void drawBullets(sf::RenderWindow &window, Assets &assets)
{
	window.draw(assets.ocean);

	// Draw each sprite from list
	for (std::list<Bullet>::iterator it = assets.bullets->begin(); it != assets.bullets->end(); ++it)
	{
		if (!it->collided)
			window.draw(it->sprite);
		// Move collided bullets out of the screen and delete from list later (fix with better solution later).
		if (it->collided)
			it->sprite.setPosition(1200, 1200);
	}
}

/*This function draw each player sprites on the window if the players are alive, and change the scene if game is over.*/
void drawPlayers(sf::RenderWindow &window, Player &player1, Player &player2, gameScene &scene)
{
	// Draw player1 ship
	if (player1.health > 0) {
		window.draw(player1.sprite);
	}
	// Draw player1 health bar
	player1.healthBar.setSize(sf::Vector2f(player1.health * HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
	window.draw(player1.healthBar);

	// Draw player2 ship
	if (player2.health > 0) {
		window.draw(player2.sprite);
	}
	// Draw player2 health bar
	player2.healthBar.setSize(sf::Vector2f(player2.health * HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT));
	window.draw(player2.healthBar);

	// Change scene if player died
	if (player1.health == 0 || player2.health == 0)
		scene = result;
}

void changeCooldownRates(Player &player1, Player &player2)
{
	/*For Player 1*/
	if (player1.moved) {
		if (player1.cooldownRate > MIN_SHOT_COOLDOWN) {
			player1.cooldownRate = player1.cooldownRate - (SHOT_COOLDOWN_INC * SHOT_DECAY_MULTIPLIER);
		}
	}
	//	if (!player1.moved && player1.startTrigger) { considering the bool is always true, i see no point for it. I also reason that decay should always occur- even ecspecially when the player is not moving. right now it happens when they are moving.
	if (player1.cooldownRate < MAX_SHOT_COOLDOWN) {
		player1.cooldownRate += SHOT_COOLDOWN_INC;
		//std::cout << "Shoot cooldown is now: " << player1.cooldownRate << std::endl;
	}

	//	}
	// reset moved value
	//	player1.moved = false; already happens during movement function

	/*For Player 2*/
	if (player2.moved) {
		if (player2.cooldownRate > MIN_SHOT_COOLDOWN) {
			player2.cooldownRate = player2.cooldownRate - (SHOT_COOLDOWN_INC * SHOT_DECAY_MULTIPLIER);
		}
	}
	//	if (!player2.moved && player2.startTrigger) {
	if (player2.cooldownRate < MAX_SHOT_COOLDOWN) {
		player2.cooldownRate += SHOT_COOLDOWN_INC;
		//std::cout << "Shoot cooldown is now: " << player1.cooldownRate << std::endl;
	}

	//	}
	// reset moved value
	//	player2.moved = false;

}