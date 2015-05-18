// Game.cpp implements the Game class found in game.h

// Game.cpp implements the Game class found in game.h

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "game.h"

/*
* Makes the calls to initialise allegro and sets up the game components.
*/
JetMan::Game::Game() {
	initGame();
}

/*
* Frees up memory allocated.
*/
JetMan::Game::~Game() {
	al_destroy_display(gameWindow);
	al_destroy_event_queue(eventQueue);
	al_destroy_event_queue(timerQueue);
	al_destroy_timer(timer);
	al_destroy_font(bigFont);
	al_destroy_font(normalFont);
	delete title;
	delete play;
	delete demo;
	delete quit;
	delete info;
	delete jetMan;
	delete wall1;
	delete wall2;
}

/*
* Initialises the game components.
*/
void JetMan::Game::initGame() {
	gameWindow = al_create_display(800, 600);
	eventQueue = al_create_event_queue();
	timerQueue = al_create_event_queue();
	al_register_event_source(eventQueue, al_get_display_event_source(gameWindow));
	al_set_window_title(gameWindow, "JetMan");
	timer = al_create_timer(1 / ((double)120));
	al_register_event_source(timerQueue, al_get_timer_event_source(timer));
	al_register_event_source(eventQueue, al_get_mouse_event_source());
	al_register_event_source(eventQueue, al_get_keyboard_event_source());

	
	bigFont = al_load_ttf_font("assets/fonts/arial.ttf", 72, NULL);
	normalFont = al_load_ttf_font("assets/fonts/arial.ttf", 20, NULL);

	mainMenu.setBounds(JetMan::Utils::Rectangle(0, 0, 800, 600));
	title = new JetMan::Graphics::Label("JetMan", bigFont);
	title->setPosition(260, 100);
	title->setColour(al_map_rgb(7, 70, 70));
	mainMenu.addWidget(title);

	play = new JetMan::Game::PlayButton(this);
	play->setPosition(360, 250);
	mainMenu.addWidget(play);
	demo = new JetMan::Game::DemoButton(this);
	demo->setPosition(360, 300);
	mainMenu.addWidget(demo);
	quit = new JetMan::Game::QuitButton(this);
	quit->setPosition(360, 350);
	mainMenu.addWidget(quit);

	gameScreen.setBounds(JetMan::Utils::Rectangle(0, 0, 800, 600));
	info = new JetMan::Graphics::InformationBox(800, 100, normalFont);
	gameScreen.addWidget(info);
	gameCanvas.setBounds(JetMan::Utils::Rectangle(0, 100, 800, 500));
	
	jetMan = new JetMan::Graphics::JetManSprite(imageManager.getImage(JetMan::Utils::ImageManager::JETMAN));
	jetMan->setPosition(50, 250);
	gameCanvas.addWidget(jetMan);
	wall1 = new JetMan::Graphics::Wall(imageManager.getImage(JetMan::Utils::ImageManager::WALL), 1);
	wall1->setPosition(480, 100);
	wall1->setVelocityX(-130);
	gameScreen.addWidget(wall1);
	wall2 = new JetMan::Graphics::Wall(imageManager.getImage(JetMan::Utils::ImageManager::WALL), 2);
	wall2->setPosition(960, 100);
	wall2->setVelocityX(-130);
	gameScreen.addWidget(wall2);
	wall3 = new JetMan::Graphics::Wall(imageManager.getImage(JetMan::Utils::ImageManager::WALL), 3);
	wall3->setPosition(1440, 100);
	wall3->setVelocityX(-130);
	gameScreen.addWidget(wall3);
	gameScreen.addWidget(&gameCanvas);

	front = wall1;
	back = wall3;

	soundManager.playSound(JetMan::Utils::SoundManager::SAD_PIANO, ALLEGRO_PLAYMODE_BIDIR, 0.6);
	state = JetMan::Graphics::InformationBox::OVER;

	score = 0;
	lastHover = nullptr;
	shouldRun = true;
	currDisplay = &mainMenu;
	al_start_timer(timer);
}

/*
* The main game loop.
*/
int JetMan::Game::loop() {
	ALLEGRO_EVENT nextEvent;
	int nUpdates = 0;
	bool hasNext;

	float spaceStartHold = 0;
	float spaceLengthHeld = 0;

	while (shouldRun) {
		hasNext = al_get_next_event(eventQueue, &nextEvent);
		if (hasNext) {
			if (nextEvent.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				shouldRun = false;
			}
			else if (nextEvent.type == ALLEGRO_EVENT_MOUSE_AXES) {
				JetMan::Utils::Rectangle mouse(nextEvent.mouse.x, nextEvent.mouse.y, 2, 2);
				if (lastHover != nullptr) {
					if (!lastHover->getBounds().intersects(mouse)) {
						lastHover->onMouseOut();
						lastHover = currDisplay->onMouseOver(mouse);
					}
					else {
						lastHover = lastHover->onMouseOver(mouse);
					}
				}
				else {
					lastHover = currDisplay->onMouseOver(mouse);
				}
			}
			else if (nextEvent.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
				JetMan::Utils::Rectangle mouse(nextEvent.mouse.x, nextEvent.mouse.y, 2, 2);
				currDisplay->onMouseClick(mouse);
			}
			else if (nextEvent.type == ALLEGRO_EVENT_KEY_DOWN) {
				if (nextEvent.keyboard.keycode == ALLEGRO_KEY_SPACE) {
					if (currDisplay == &(gameScreen)) {
						if (state == JetMan::Graphics::InformationBox::ACTIVE) {
							spaceStartHold = al_current_time();
						}
					}
				}
			}
			else if (nextEvent.type == ALLEGRO_EVENT_KEY_UP) {
				if (currDisplay == &(gameScreen)) {
					if (nextEvent.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
						if (state == JetMan::Graphics::InformationBox::ACTIVE) {
							// Pause the game
							state = JetMan::Graphics::InformationBox::PAUSED;
							info->setState(state);
							soundManager.stopSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE);
						}
						else {
							// return to main menu
							soundManager.playSound(JetMan::Utils::SoundManager::SAD_PIANO, ALLEGRO_PLAYMODE_BIDIR, 0.6);
							state = JetMan::Graphics::InformationBox::OVER;
							info->setState(state);
							currDisplay = &(mainMenu);
						}
					}
					else if (nextEvent.keyboard.keycode == ALLEGRO_KEY_ENTER) {
						if (state == JetMan::Graphics::InformationBox::PAUSED) {
							soundManager.playSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE, ALLEGRO_PLAYMODE_BIDIR, 0.6);
							state = JetMan::Graphics::InformationBox::ACTIVE;
							info->setState(state);
						}
						else if (state == JetMan::Graphics::InformationBox::OVER) {
							soundManager.playSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE, ALLEGRO_PLAYMODE_BIDIR, 0.6);
							state = JetMan::Graphics::InformationBox::ACTIVE;
							info->setState(state);
							reset();
						}
					}
					else if (nextEvent.keyboard.keycode == ALLEGRO_KEY_SPACE) {
						if (currDisplay == &(gameScreen)) {
							if (state == JetMan::Graphics::InformationBox::ACTIVE) {
								spaceLengthHeld = al_current_time() - spaceStartHold;
								if (spaceLengthHeld > 0.2f) {
									jetMan->setVelocityY(-150);
								}
								else {
									jetMan->setVelocityY(-70);
								}
							}
						}
					}
				}
			}
		}

		hasNext = al_get_next_event(timerQueue, &nextEvent);
		if (hasNext) {
			// Timer event - Update game here
			if (state != JetMan::Graphics::InformationBox::PAUSED && state != JetMan::Graphics::InformationBox::OVER) {
				jetMan->update(FPSIncrement);
				wall1->update(FPSIncrement);
				wall2->update(FPSIncrement);
				wall3->update(FPSIncrement);

				JetMan::Utils::Rectangle jetManBounds = jetMan->getBounds();
				if (jetManBounds.getY() < 100) {
					jetManBounds.setY(100);
					jetMan->setBounds(jetManBounds);
					jetMan->setVelocityY(0);
				}

				if (jetManBounds.getY() > 600 - jetManBounds.getHeight()) {
					// Crashed down.
					state = JetMan::Graphics::InformationBox::OVER;
					info->setState(state);
					soundManager.stopSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE);
					soundManager.playSound(JetMan::Utils::SoundManager::CRASH, ALLEGRO_PLAYMODE_ONCE, 0.6);
				}
				else if ((wall1->collides(jetManBounds)) || (wall2->collides(jetManBounds)) || (wall3->collides(jetManBounds))) {
					// Collided with a wall.
					state = JetMan::Graphics::InformationBox::OVER;
					info->setState(state);
					soundManager.stopSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE);
					soundManager.playSound(JetMan::Utils::SoundManager::CRASH, ALLEGRO_PLAYMODE_ONCE, 0.6);
				}
				else {
					JetMan::Utils::Rectangle* w1 = &front->getBounds();
					if (w1->getX() < -w1->getWidth()) {
						score++;
						info->updateScore(score);
						w1->setX(back->getBounds().getX() + 3 * w1->getWidth());
						front->setBounds(*w1);
						front->updateGap();
						back = front;
						if (front == wall1) {
							front = wall2;
						}
						else if (front == wall2) {
							front = wall3;
						}
						else {
							front = wall1;
						}
					}
				}
			}
			nUpdates++;
		}

		if (nUpdates == 10) {
			// Don't display all the time.
			nUpdates = 0;
			display();
		}
	}
	return 0;
}

/*
 * Display the graphics.
 */
void JetMan::Game::display() {
	al_draw_bitmap(imageManager.getImage(JetMan::Utils::ImageManager::BACKGROUND), 0, 0, NULL);
	currDisplay->draw();
	al_flip_display();
}

/*
 * Implements the play button being clicked.
 */
void JetMan::Game::PlayButton::onClick() {
	game->currDisplay = &game->gameScreen;
	game->soundManager.stopSound(JetMan::Utils::SoundManager::SAD_PIANO);
	game->soundManager.playSound(JetMan::Utils::SoundManager::MISSION_IMPOSSIBLE, ALLEGRO_PLAYMODE_BIDIR, 0.6);
	game->reset();
}

/*
 * Implements the demo button being clicked.
 */
void JetMan::Game::DemoButton::onClick() {
	
}

/*
 * Implements the quit button being clicked.
 */
void JetMan::Game::QuitButton::onClick() {
	game->shouldRun = false;
}

/*
 * Resets the game.
 */
void JetMan::Game::reset() {
	state = JetMan::Graphics::InformationBox::ACTIVE;
	info->setState(state);
	jetMan->setPosition(50, 250);
	jetMan->setVelocityY(0);
	score = 0;
	info->updateScore(score);
	wall1->setPosition(480, 100);
	wall2->setPosition(960, 100);
	wall3->setPosition(1440, 100);
	front = wall1;
	back = wall3;
}