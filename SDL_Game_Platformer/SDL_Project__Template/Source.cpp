#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>

#include "enemy.h"
#include "button.h"
#include "sound.h"
#include <string>



enum class Direction
{
	Left,
	Right
};



struct block //class not needed for block, rendering method caused too many issues
{
public:

	SDL_Rect pos;
	SDL_Rect source;

	
	block(SDL_Rect _pos, SDL_Rect _source);
	~block();

private:

};

block::block(SDL_Rect _pos, SDL_Rect _source) : pos(_pos), source(_source) {}
block::~block() {};


bool InitEverything(SDL_Window** _window, SDL_Renderer** _renderer);
bool InitSDL();
bool CreateWindow(SDL_Window** _window);
bool CreateRenderer(SDL_Window** _window, SDL_Renderer** _render);
void SetupRenderer(SDL_Renderer** _renderer);

void Render(SDL_Renderer* _renderer, SDL_Rect _playerPos, std::vector<enemy>& _enemy, std::vector<block>& _blocks, SDL_Texture* _blockTexture, SDL_Texture* _enemyTexture, button _scoreKeepingButtons, SDL_Texture* _timerTexture);

//void Render();
void RunGame(SDL_Renderer* _renderer);
void Menu(SDL_Renderer* renderer);


void AddEnemies(std::vector<enemy>& _enemies, int _lastEnemyPos);

void AddBlocks(std::vector<block>& _blocks, int _placementPos);


void movePlayer(const Uint8* _event, SDL_Rect& _playerPos);



bool CheckCollision(const SDL_Rect& rect1, const SDL_Rect& rect2);
bool CheckEnemyCollisions(SDL_Rect _playerPos, std::vector<enemy>& _enemies, std::vector<block>& _blocks);
int CheckBlockCollisions(SDL_Rect& _playerPos, std::vector<block>& _blocks, SDL_Rect& _velocoity, int& _gravity);

const int movementFactor = 2; // Amount of pixels the player move per key press

//Window Size
const int sizeX = 400;
const int sizeY = 600;

// Window pos
const int posX = 300;
const int posY = 200;

int main(int argc, char* args[])
{
	//create window things
	SDL_Window* window;
	SDL_Renderer* renderer;

	
	if (InitEverything(&window, &renderer)) {
		std::cout << "failed to initialise";
		return -1;
	}
	
	if (TTF_Init() < 0) {
		std::cout << "failed to initialise ttf";
		return -1;
	}
	
	Menu(renderer);


	

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


void Menu(SDL_Renderer* renderer) {
	// Create the buttons
	button playButton(renderer, "Play", 100, 100, 100, 50);
	button quitButton(renderer, "Quit", 100, 300, 100, 50);


	
	// menu loop
	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				int x, y;
				SDL_GetMouseState(&x, &y);
				if (playButton.isClicked(x, y)) {
					RunGame(renderer);
				}
				else if (quitButton.isClicked(x, y)) {
					quit = true;
				}
			}
		}

		// Clear the screen
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(renderer);

		// Render the buttons
		playButton.render();
		
		quitButton.render();

		// Update the screen
		SDL_RenderPresent(renderer);
	}
}




void RunGame(SDL_Renderer* _renderer)
{
	bool loop = true;   //determines if the game loop should run
	int camPos = 0;  //the position of the virtual camera on the y axis
	int cameraDelay = 120;  //amount of frames that need to run before the camera starts to scroll up
	int jumping = 0;  //amount of frames the player is jumping for
	int jumpCheck = 0;   //number of times the player has jumped without touching the ground
	bool gravity = true;  //whether gravity is applied to the player
	bool screenScrollingUp = false;   //setting for whether the camera should move upwards, just for dev purposes
	
	int score = 0;

	int collision = 0;

	SDL_Rect lastPosition = { 0,0,0,0 };
	SDL_Rect velocity = { 0,0,0,0 };

	// Initialize the player
	SDL_Rect _playerPos{ 180,550,20,20 };
	

	
	sound bkgMusic = sound("music.wav");

	std::vector<block> blocks;
	AddBlocks(blocks, 0);

	std::vector<enemy> enemies;
	AddEnemies(enemies, 0);

	//create texture for blocks
	SDL_Surface* blockImage = SDL_LoadBMP("brick-wall.bmp");

	SDL_Texture* blockTexture = NULL;
	blockTexture = SDL_CreateTextureFromSurface(_renderer, blockImage);

	SDL_FreeSurface(blockImage);

	if (blockTexture == NULL)  //if texture won't load then replace it with a simple black one 
	{
		//std::cout << "Failed to load block texture, error: " << SDL_GetError << std::endl<< _filePath << std::endl;
		SDL_Surface* blockImage = SDL_CreateRGBSurface(0, 50, 50, 32, 0, 0, 0, 0);
		blockTexture = SDL_CreateTextureFromSurface(_renderer, blockImage);
		SDL_FreeSurface(blockImage);

	}

	//create texture for enemies
	SDL_Surface* enemyImage = SDL_LoadBMP("squid2.bmp");

	SDL_Texture* enemyTexture = NULL;
	enemyTexture = SDL_CreateTextureFromSurface(_renderer, enemyImage);

	SDL_FreeSurface(enemyImage);

	if (enemyTexture == NULL)  //if texture won't load then replace it with a simple black one 
	{
		//std::cout << "Failed to load block texture, error: " << SDL_GetError << std::endl<< _filePath << std::endl;
		SDL_Surface* enemyImage = SDL_CreateRGBSurface(0, 50, 50, 32, 0, 0, 0, 0);
		enemyTexture = SDL_CreateTextureFromSurface(_renderer, enemyImage);
		SDL_FreeSurface(enemyImage);

	}

	

	//create a floor for the player to stand on when the game starts 
	for (int i = 0; i < 400; i += 20)
	{
		
		SDL_Rect dest = { i,580,20,20 }; //location
		SDL_Rect source = { 0,20,20,20 };  //size

		blocks.push_back(block(dest, source));

	}

	//save start time for the timer to use 
	Uint32 startTime = SDL_GetTicks();

	//create font for the timer
	TTF_Font* ChrustyRock = TTF_OpenFont("ChrustyRock-ORLA.ttf", 24);

	

	while (loop)
	{
		
		Uint32 currentTime = SDL_GetTicks();

		// Calculate the elapsed time in milliseconds
		Uint32 elapsedTime = currentTime - startTime;

		// Convert elapsed time to seconds and milliseconds
		int seconds = elapsedTime / 1000;
		int milliseconds = elapsedTime % 1000;

		// Format the elapsed time as a string
		char elapsedSeconds[1];
		char elapsedMilliseconds[1];
		
		

		//create a c_string to hold the time so it can be turned into text
		char elapsedTimeString[5]{ '0','0',':','0','0' };
		elapsedTimeString[0] = char(seconds / 10 % 10);
		elapsedTimeString[1] = char(seconds % 10);
		elapsedTimeString[3] = char(milliseconds / 10 % 10);
		elapsedTimeString[4] = char(milliseconds % 10);
		

		SDL_Surface* timerSurface = TTF_RenderText_Solid(ChrustyRock, elapsedTimeString, { 255, 255, 255 });
		SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(_renderer, timerSurface);
		
		//std::cout << elapsedTimeString[0] << elapsedTimeString[1] << elapsedTimeString[3] << elapsedTimeString[4];


		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL); // get current key states 

		//last postion calculated before any movement is applied
		lastPosition.x = _playerPos.x;
		lastPosition.y = _playerPos.y;

/*
		// Check collisions against enemies
		if (CheckEnemyCollisions(_playerPos, _enemies))
			ResetPlayerPos(_playerPos, _bottomBar);

			*/
		if (jumping > 0)  //if the player is jumping
		{
			if (currentKeyStates[SDL_SCANCODE_LEFT])  //check if the player is also trying to move left while jumping
			{
				_playerPos.x -= 2;
			}
			if (currentKeyStates[SDL_SCANCODE_RIGHT]) //check if the player is also trying to move right while jumping
			{
				_playerPos.x += 2;
			}
			_playerPos.y -= 10;
			
			
			jumping--; //decreases each frame to keep track of how long the jump is

		}
		
		if (camPos == 0)
		{

			AddBlocks(blocks, camPos - 600);
			AddEnemies(enemies, camPos - 600);
			camPos = 600;
			
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			
			if (event.type == SDL_QUIT)
				loop = false;
			else if (event.type == SDL_KEYDOWN)
			{

				movePlayer(currentKeyStates, _playerPos);
			}

			//update position of everything on the screen
			//not enough for the player's position to be changed as it means they could eventually climb off the screen
			//instead everything needs to be moved up or down
			

			if (event.key.keysym.sym == SDLK_SPACE && jumping == 0 && jumpCheck < 3) {
				jumping = 25;  //number of frames the player will be jumping for
				jumpCheck += 1;
			}
			

		}		

		//Move every thing on the screen down to give the effect of a screen scrolling upwards

		if (cameraDelay > 0) {   //delay the camera scrolling up to give the player a chance to orient themselves
			cameraDelay -= 1;
			
			screenScrollingUp = false;
		}
		else
		{
			screenScrollingUp = true;
		}

		//apply gravity to the player by increasing the y value (0,0 is top left)
		if (gravity)
		{
			_playerPos.y += 2;

		}

		std::vector<enemy>::iterator currentEnemy;
		std::vector<block>::iterator currentBlock;

		if (screenScrollingUp)
		{
			camPos -= 1;
			score += 1;
			cameraDelay = 3; //controls how fast the camera scrolls upwards 

		//	std::cout << camPos << std::endl;

			for (currentEnemy = enemies.begin(); currentEnemy != enemies.end(); ++currentEnemy)
			{
				

				(*currentEnemy).pos.y = (*currentEnemy).pos.y + 1;
				
			}

			
			for (currentBlock = blocks.begin(); currentBlock != blocks.end(); ++currentBlock)
			{
				(*currentBlock).pos.y = (*currentBlock).pos.y + 1;
				
			}
		}
		
		//Add extra score marker, temp fix until texrt is sorted
		
		char scoreStr[10] = {0};
		std::string scoreText = std::to_string(score);
		//std::sprintf(scoreStr, "%d", score);

		button scoreButton(_renderer, scoreText.c_str(), 100, 00, 200, 50);
		
		//calculate velocity so it can be used in collisions
		velocity.x = _playerPos.x - lastPosition.x;
		velocity.y = _playerPos.y - lastPosition.y;

		
		//check if player is colliding with a block and where that block is in relation to player
		collision = CheckBlockCollisions(_playerPos, blocks, velocity, jumpCheck);


		//loss conditions
		//check if player hits an enemy
		if (CheckEnemyCollisions(_playerPos, enemies, blocks)) { //this also handles the collision between enemies and blocks
			loop = false;
		}

		//check if player falls off the screen
		if (_playerPos.y > 600)
		{
			loop = false;
		}

		Render(_renderer, _playerPos, enemies, blocks, blockTexture, enemyTexture, scoreButton, timerTexture);

		// Add a 16msec delay to make game run at 60fps			
		SDL_Delay(16);
		
	}
}

void Render(SDL_Renderer* _renderer, SDL_Rect _playerPos, std::vector<enemy>& _enemies, std::vector<block>& _blocks, SDL_Texture* _blockTexture, SDL_Texture* _enemyTexture, button _scoreKeepingButtons, SDL_Texture* _timerTexture)
{
	// Clear the window and make it all red
	SDL_RenderClear(_renderer);

	// Change color to blue!
	SDL_SetRenderDrawColor(_renderer, 0, 0, 255, 255);

	// Render player
	SDL_RenderFillRect(_renderer, &_playerPos);

	

	//render enemies
	std::vector<enemy>::iterator currentEnemy;
	for (currentEnemy = _enemies.begin(); currentEnemy != _enemies.end(); ++currentEnemy)
	{
		
		SDL_RenderCopy(_renderer, _enemyTexture, &(*currentEnemy).source, &(*currentEnemy).pos);
	}

	// Change color to green!
	SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255);
	

	
	//render blocks
	std::vector<block>::iterator currentBlock;

	for (currentBlock = _blocks.begin(); currentBlock != _blocks.end(); ++currentBlock)
	{
		
		//currentBlock->renderBlock("bkg.bmp", _renderer, texture);
		SDL_RenderCopy(_renderer, _blockTexture, &(*currentBlock).source, &(*currentBlock).pos);

	}

	_scoreKeepingButtons.render();

	//timer
	SDL_Rect timerSource{0,80,80,0};
	SDL_Rect timerPos{ 0,80,80,0 };
	
	SDL_RenderCopy(_renderer, _timerTexture, &timerSource, &timerPos);



	// Render the changes above
	SDL_RenderPresent(_renderer);
}

bool InitEverything(SDL_Window** _window, SDL_Renderer** _renderer)
{
	if (!InitSDL()) {
		std::cout << "failed to init sdl";
		return false;
	}

	if (!CreateWindow(_window))
	{
		std::cout << "failed to create window";
		return false;
	}
	if (!CreateRenderer(_window, _renderer))
		std::cout << "failed to create renderer";
		return false;
	
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		std::cout << "failed to initialise audio";
		return false;
	}
	

	SetupRenderer(_renderer);

	return true;

}


bool InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << " Failed to initialize SDL : " << SDL_GetError() << std::endl;
		return false;
	}

	return true;
}

bool CreateWindow(SDL_Window** _window)
{
	*_window = SDL_CreateWindow("test", posX, posY, sizeX, sizeY, 0);

	if (*_window == nullptr)
	{
		std::cout << "Failed to create window : " << SDL_GetError();
		return false;
	}

	return true;
}

bool CreateRenderer(SDL_Window** _window, SDL_Renderer** _renderer)
{
	*_renderer = SDL_CreateRenderer(*_window, -1, 0);

	if (*_renderer == nullptr)
	{
		std::cout << "Failed to create renderer : " << SDL_GetError();
		return false;
	}

	return true;
}

void SetupRenderer(SDL_Renderer** _renderer)
{
	// Set size of renderer to the same as window
	SDL_RenderSetLogicalSize(*_renderer, sizeX, sizeY);

	// Set color of renderer to red
	SDL_SetRenderDrawColor(*_renderer, 255, 0, 0, 255);
}



bool CheckCollision(const SDL_Rect& rect1, const SDL_Rect& rect2)
{
	// Find edges of rect1
	int left1 = rect1.x;
	int right1 = rect1.x + rect1.w;
	int top1 = rect1.y;
	int bottom1 = rect1.y + rect1.h;

	// Find edges of rect2
	int left2 = rect2.x;
	int right2 = rect2.x + rect2.w;
	int top2 = rect2.y;
	int bottom2 = rect2.y + rect2.h;

	// Check edges
	if (left1 > right2)// Left 1 is right of right 2
		return false; // No collision

	if (right1 < left2) // Right 1 is left of left 2
		return false; // No collision

	if (top1 > bottom2) // Top 1 is below bottom 2
		return false; // No collision

	if (bottom1 < top2) // Bottom 1 is above top 2 
		return false; // No collision

	return true;
}

bool CheckEnemyCollisions(SDL_Rect _playerPos, std::vector<enemy>& _enemies, std::vector<block>& _blocks)
{

	std::vector<enemy>::iterator EnemyItr;
	std::vector<block>::iterator BlockItr;
	for (EnemyItr = _enemies.begin(); EnemyItr != _enemies.end(); ++EnemyItr)
	{
		(*EnemyItr).patrol();

		for (BlockItr = _blocks.begin(); BlockItr != _blocks.end(); ++BlockItr)  //check if it collides with a block, if so send it in the other direction
		{
			if (CheckCollision((*BlockItr).pos, (*EnemyItr).pos))
			{
				if ((*EnemyItr).dirSwitch)
				{
					(*EnemyItr).dirSwitch = false;
				}
				else
				{
					(*EnemyItr).dirSwitch = true;
				}
				//std::cout << "enemy block collide" << std::endl;
			}
		}

		if (CheckCollision((*EnemyItr).pos, _playerPos)) {  //check if it collides with a player
			return true;
		}
	}

	

	return false;
}

void AddEnemies(std::vector<enemy>& _enemies, int _lastEnemyPos)
{
	bool patrolling = true;
	int maxEnemies = 5; //sets number of enemies to show up per generation 
	SDL_Rect source{ 0,20,20,20 };  


	for (int i = 0; i < maxEnemies; i++)
	{
		_enemies.push_back(enemy({ rand() % 300, _lastEnemyPos, 20, 20}, source, 1, patrolling));
		_lastEnemyPos += 200; //set this so the next one will be 200 units higher than the last
	}

	
}


int CheckBlockCollisions(SDL_Rect& _playerPos, std::vector<block>& _blocks, SDL_Rect& _velocity, int& _jumpCheck)
{
	SDL_Rect collisionResult{0,0,0,0};
	int resultY = 0;
	int resultX = 0;
	bool xCollision = false;
	bool yCollision = false;

	std::vector<block>::iterator itr;
	for (itr = _blocks.begin(); itr != _blocks.end(); ++itr)
	{
		if (CheckCollision((*itr).pos, _playerPos))
		{

			if (
				(_playerPos.y < (*itr).pos.y + 20 && _playerPos.y + 20 > (*itr).pos.y))
			{
				
				yCollision = true;

				
				

			}
			// Collision from above or below
			if (
				(_playerPos.x < (*itr).pos.x + 20 && _playerPos.x + 20 > (*itr).pos.x)) {
				
				xCollision = true;
				if (_playerPos.x < (*itr).pos.x + 20)
				{
					_jumpCheck = 0; //if the player is stood on top of a block set the jumpcheck back to 0
				}
				
				
			}
				
			




			
		}

		
	}
	if (xCollision)
	{

		_playerPos.y -= _velocity.y;
		//_velocoity.y = 0;
		

	}
	if (yCollision)
	{
		//
		_playerPos.x -= _velocity.x;

		//_velocoity.x = 0;
	}

	 

	return 0;
}

void AddBlocks(std::vector<block>& _blocks, int _placementPos)
{
	SDL_Rect dest{ 0,0,20,20 }; //location
	SDL_Rect source{ 0,20,20,20 };  //size

	int randGap;
	int randPlatformLen;
	int maxPlatforms = 20;
	int randY;
	int randX;
	int lastPlacementPos = _placementPos;

	//window size is 400 x 600

	//initialise random variables
	srand(time(0));
	randGap = rand() % 31 + 30;
	randPlatformLen = 1 + (rand() % 5);
	randX = rand() % 401;



	//fill walls (so player can't fall out of level)

	for (int i = 0; i < 600; i+=20)
	{
		//left side
		 dest = { 0,_placementPos + i,20,20 }; //location
		_blocks.push_back(block(dest, source));


		//right side
		dest = { 380,_placementPos + i,20,20 }; //location
		_blocks.push_back(block(dest, source));
		
	}

	//place platforms
	for (int j = 0; j < maxPlatforms; j++)
	{
		dest = { randX, lastPlacementPos + randGap, 20, 20 }; //location
		_blocks.push_back(block(dest, source));
	//	std::cout << randX << " this is x" << std::endl;
		//std::cout << lastPlacementPos + randGap << std::endl;
		

		for (int k = 0; k < randPlatformLen * 20; k+=20)
		{
			dest = { randX + k,lastPlacementPos + randGap,20,20 }; //location
			_blocks.push_back(block(dest, source));
			//std::cout << lastPlacementPos << std::endl;

		}

		lastPlacementPos += randGap;

		randPlatformLen = 1 + (rand() % 5);
		randGap = rand() % 31 + 30;
		randX = rand() % 401;

	}
	

}



void movePlayer(const Uint8* _event, SDL_Rect& _playerPos) {
	
	if (_event[SDL_SCANCODE_LEFT])
	{
		_playerPos.x -= movementFactor;
	}
	if (_event[SDL_SCANCODE_RIGHT])
	{
		_playerPos.x += movementFactor;
	}

/*
	switch (_event)
	{
		if (true)
		{

		} SDLK_RIGHT:
		_playerPos.x += movementFactor;
		break;
	case SDLK_LEFT:
		
		break;
		// Remeber 0,0 in SDL is left-top
	
	default:
		break;
	}
*/
}
 
