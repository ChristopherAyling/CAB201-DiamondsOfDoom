//###############################//
//                               //
//      -DIAMONDS OF DOOM-       //
//                               //
//###############################//
//  Chris Ayling       n9713581  //
//###############################//

//gcc DoD.c -std=gnu99 -I/home/ZDK -L/home/ZDK -lzdk -lncurses -o DoD

#include <stdlib.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>


//initiating variables
int sx = 0;
int sy = 0;

int lives = 10;
int score = 0;

//srand(stuff)

#define MISSILE_SPEED (-0.2)
#define MAX_MISSILES (100)
sprite_id missiles[MAX_MISSILES];
int tm = 0;

#define BIG_DIAMOND (3)
#define MED_DIAMOND (2)
#define SML_DIAMOND (1)
#define SPACESHIP (-1)

#define DIAMOND_SPEED (0.5)
#define MAX_BIG_DIAMONDS (10)
#define MAX_MED_DIAMONDS (20)
#define MAX_SML_DIAMONDS (40)
sprite_id big_diamonds[MAX_BIG_DIAMONDS];
sprite_id med_diamonds[MAX_MED_DIAMONDS];
sprite_id sml_diamonds[MAX_SML_DIAMONDS];

#define DELAY (10)
#define Ms_in_S (1000)
#define S_in_M (60)
int timeMs = 0;
int timeS = 0;
int timeM = 0;

bool game_over = false;
bool cleared_diamonds = false;

/**
	- Sprites -
**/

//help screen sprite_create
char * help_image =
"#**************************************#"
"*         CAB202 Assignment 1          *"
"*        The Diamonds of Doom          *"
"*         Christopher Ayling           *"
"*              n9713581                *"
"#**************************************#"
"*              Controls                *"
"*      q         : quit                *"
"*      h         : help                *"
"*      a / d     : move left/right     *"
"*      j, k, l   : shoot missiles      *"
"#**************************************#"
"*       Press a key to play...         *"
"#**************************************#";
int help_w = 40;
int help_h = 14;
sprite_id help;


//Spaceship Sprite
char * spaceship_image =
"A     A"
"H .A. H"
"HHHHHHH"
"H[]H[]H"
"\\HHHHH/";
int spaceship_w = 7;
int spaceship_h = 5;
sprite_id spaceship;


//Missile Sprite
char * missile_image =
"^";


//Diamond Sprites
char * Diamond_image =
"  X  "
" XXX "
"XXXXX"
" XXX "
"  X  ";
int diamond_w = 5;
int diamond_h = 5;
sprite_id diamond;
#define FREE_CELLS_BIG (12)
int clearXbig[FREE_CELLS_BIG] = {0, 1, 3, 4, 0 ,4, 0, 4, 0, 1, 3, 4};
int clearYbig[FREE_CELLS_BIG] = {0, 0, 0, 0, 1, 1, 3, 3, 4, 4, 4, 4};

char * med_Diamond_image =
" X "
"XXX"
" X ";
#define FREE_CELLS_MED (4)
int clearXmed[FREE_CELLS_MED] = {0, 2, 0, 2};
int clearYmed[FREE_CELLS_MED] = {0, 0, 2, 2};

char * sml_Diamond_image =
"X";



/**
	- Function Declarations -
**/
void resetGame(void);
void resetSpaceship(void);
void renderObject(sprite_id);


/**
	- Function Definitions -
**/

/*
The border consists of a set of lines occupying the visible edges of the terminal window
-Visible during normal play mode
-Minimum 60x20
*/
void DrawBorder(void){
	//edges
	draw_line(1, 0, sx-2, 0, 220);//Horizontal Top
	draw_line(0, 0, 0, sy-1, 220);//Vertical left
	draw_line(sx-1, sy-1, sx-1, 0, 220);//Vertical Right
	draw_line(1, sy-1, sx-2, sy-1, 220);//Horizontal Bottom
	//status border
	draw_line(1, 2, sx-2, 2, 220);
	show_screen();
	return;
}//end DrawBorder



/*
The status display appears at the top of the terminal.
-Lives (10)
-Score (0)
-Elapsed game time (00,00)
*/
void DrawStatus(void){
	draw_formatted(4, 1, "lives: %d", lives);
	draw_formatted((sx/2 - 5), 1, "score: %d", score);
	draw_formatted((sx - 13), 1, "time: %d:%d", timeM, timeS);
}//end DrawStatus



/*
Displays name and student number and keyboard commands
-freezes and hides game
-exits when user presses a key
*/
void helpDialog(void){
	while ( get_char() >= 0 ) {}
	clear_screen();
	sprite_show(help);
	sprite_draw(help);
	show_screen();
	wait_char();
	sprite_hide(help);
	clear_screen();	
}//end helpDialog



/*
Waits for (y/n) input

-(y)
close dialog
resets game
-(n)
clean up display
exit cleanly
*/
void gameOverDialog(void){
	while ( get_char() >= 0 ) {} //clear keyboard buffer
	clear_screen();
	draw_string(10, 10, "Game Over\nPlay Again?\n\t(y/n)");
	show_screen();
	
	int key;
	while (true){
		key = wait_char();
		if (key == 'y' || key == 'n') break;
	}//end while
	
	if (key == 'n'){
		game_over = true;		
	} else if (key == 'y'){
		resetGame();
		helpDialog();
	}

	show_screen();
}//end gameOverDialog



/*
Spawns and hides 100 missiles
*/
void spawnMissiles(void){
	for (int i = 0; i < MAX_MISSILES; i++){
		missiles[i] = sprite_create(-5, -5, 1, 1, missile_image);
		sprite_hide(missiles[i]);
	}//end for
}//end spawnMissiles



/*
Shoots a missile
*/
void shootMissile(int station){
	int lx = 0;
	int ly = 0;
	
	//check which station to launch from
	if (station == 'j'){ //shoot from left
		lx = sprite_x(spaceship) + spaceship_w/2 - 3;
		ly = sprite_y(spaceship) - 1;
	}else if (station == 'k'){ //shoot from middle
		lx = sprite_x(spaceship) + spaceship_w/2;
		ly = sprite_y(spaceship);
	}else if (station == 'l'){ //shoot from right
		lx = sprite_x(spaceship) + spaceship_w/2 + 3;
		ly = sprite_y(spaceship) - 1;
	}//end if
	
	//launch
	for (int m = 0; m < MAX_MISSILES; m++){
		if(!sprite_visible(missiles[m])){
			sprite_show(missiles[m]);
			sprite_move_to(missiles[m], lx, ly);
			sprite_turn_to(missiles[m], 0, MISSILE_SPEED);	
			return;
		}//end if		
	}//end for
}//end shootMissile



/*
step missiles
*/
void stepMissiles(void){
	for (int m = 0; m < MAX_MISSILES; m++){
		if (sprite_visible(missiles[m])){
			sprite_step(missiles[m]);
			if (sprite_y(missiles[m]) <= 3){
				sprite_hide(missiles[m]);
				sprite_turn_to(missiles[m], 0, 0);
			}//end if
		}//end if
	}//end for
}//end stepMissiles



/*
reset missiles
*/
void resetMissiles(void){
	for (int m = 0; m < MAX_MISSILES; m++){
		sprite_hide(missiles[m]);
		sprite_turn_to(missiles[m], 0, 0);
		sprite_move_to(missiles[m], -5, -5);
	}//end for	
}//end resetMissiles



/*
Spawn Diamonds
*/
void spawnDiamonds (void){
	//big
	for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
		big_diamonds[d] = sprite_create(9 + rand() % sx - 8, 8, 5, 5, Diamond_image);
		sprite_turn_to(big_diamonds[d], 0.2, 0);
		sprite_turn(big_diamonds[d], rand() % 360);
		sprite_draw(big_diamonds[d]);
	}//end for
	
	//med
	for (int d = 0; d < MAX_MED_DIAMONDS; d++){
		med_diamonds[d] = sprite_create(-5, -5, 3, 3, med_Diamond_image);
		sprite_hide(med_diamonds[d]);
	}//end for	
	
	//sml
	for (int d = 0; d < MAX_SML_DIAMONDS; d++){
		sml_diamonds[d] = sprite_create(-5, -5, 1, 1, sml_Diamond_image);
		sprite_hide(sml_diamonds[d]);
	}//end for	
	
	
}//end spawnDiamonds



/*
step Diamonds
*/
void stepDiamonds(void){
	//big
	for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
		if (sprite_visible(big_diamonds[d])){
			sprite_step(big_diamonds[d]);
			if (sprite_y(big_diamonds[d]) <= 3) sprite_turn_to(big_diamonds[d], sprite_dx(big_diamonds[d]), -sprite_dy(big_diamonds[d])); //top
			if (sprite_x(big_diamonds[d]) <= 1) sprite_turn_to(big_diamonds[d], -sprite_dx(big_diamonds[d]), sprite_dy(big_diamonds[d])); //left
			if (sprite_y(big_diamonds[d]) > sy - 6) sprite_turn_to(big_diamonds[d], sprite_dx(big_diamonds[d]), -sprite_dy(big_diamonds[d])); //bottom
			if (sprite_x(big_diamonds[d]) > sx - 6) sprite_turn_to(big_diamonds[d], -sprite_dx(big_diamonds[d]), sprite_dy(big_diamonds[d])); //left
		}//end if
	}//end for
	
	//med
	for (int d = 0; d < MAX_MED_DIAMONDS; d++){
		if (sprite_visible(med_diamonds[d])){
			sprite_step(med_diamonds[d]);
			if (sprite_y(med_diamonds[d]) <= 3) sprite_turn_to(med_diamonds[d], sprite_dx(med_diamonds[d]), -sprite_dy(med_diamonds[d])); //top
			if (sprite_x(med_diamonds[d]) <= 1) sprite_turn_to(med_diamonds[d], -sprite_dx(med_diamonds[d]), sprite_dy(med_diamonds[d])); //left
			if (sprite_y(med_diamonds[d]) > sy - 4) sprite_turn_to(med_diamonds[d], sprite_dx(med_diamonds[d]), -sprite_dy(med_diamonds[d])); //bottom
			if (sprite_x(med_diamonds[d]) > sx - 4) sprite_turn_to(med_diamonds[d], -sprite_dx(med_diamonds[d]), sprite_dy(med_diamonds[d])); //left
		}//end if
	}//end for
	
	//sml
	for (int d = 0; d < MAX_SML_DIAMONDS; d++){
		if (sprite_visible(sml_diamonds[d])){
			sprite_step(sml_diamonds[d]);
			if (sprite_y(sml_diamonds[d]) <= 3) sprite_turn_to(sml_diamonds[d], sprite_dx(sml_diamonds[d]), -sprite_dy(sml_diamonds[d])); //top
			if (sprite_x(sml_diamonds[d]) <= 1) sprite_turn_to(sml_diamonds[d], -sprite_dx(sml_diamonds[d]), sprite_dy(sml_diamonds[d])); //left
			if (sprite_y(sml_diamonds[d]) > sy - 1) sprite_turn_to(sml_diamonds[d], sprite_dx(sml_diamonds[d]), -sprite_dy(sml_diamonds[d])); //bottom
			if (sprite_x(sml_diamonds[d]) > sx - 1) sprite_turn_to(sml_diamonds[d], -sprite_dx(sml_diamonds[d]), sprite_dy(sml_diamonds[d])); //left
		}//end if
	}//end for
	
}//end stepDiamonds



/*
reset Diamonds
*/
void resetDiamonds(void){
	//big
	for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
		sprite_show(big_diamonds[d]);
		sprite_move_to(big_diamonds[d], 9 + rand() % sx - 8, 8);
		sprite_turn_to(big_diamonds[d], 0.2, 0);
		sprite_turn(big_diamonds[d], rand() % 360);
		sprite_draw(big_diamonds[d]);
	}//end for
	
	//med
	for (int d = 0; d < MAX_MED_DIAMONDS; d++){
		sprite_hide(med_diamonds[d]);
		sprite_move_to(med_diamonds[d], -5, -5);
	}//end for
	
	//sml
	for (int d = 0; d < MAX_SML_DIAMONDS; d++){
		sprite_hide(sml_diamonds[d]);
		sprite_move_to(sml_diamonds[d], -5, -5);
	}//end for
	
}//end resetMissiles



/*
Checks if objectA and objectB have collided
*/
bool collisionBetween(sprite_id objectA, sprite_id objectB, int size){
	bool collision = true;
	
	int objectA_top = round(sprite_y(objectA));
	int objectA_left = round(sprite_x(objectA));
	int objectA_bottom = round(sprite_y(objectA)) + sprite_height(objectA) - 1;
	int objectA_right = round(sprite_x(objectA)) + sprite_width(objectA) - 1;
	
	int objectB_top = round(sprite_y(objectB));
	int objectB_left = round(sprite_x(objectB));
	int objectB_bottom = round(sprite_y(objectB)) + sprite_height(objectB) - 1;
	int objectB_right = round(sprite_x(objectB)) + sprite_width(objectB) - 1;
	
	if(objectA_top > objectB_bottom) collision = false;
	else if (objectA_bottom < objectB_top) collision = false;
	else if (objectA_left > objectB_right) collision = false;
	else if (objectA_right < objectB_left) collision = false;
	
	if (collision && size == BIG_DIAMOND){
		for (int i = 0; i < FREE_CELLS_BIG; i++){
			if (sprite_x(objectA) - sprite_x(objectB) == clearXbig[i] && sprite_y(objectA) - sprite_y(objectB) == clearYbig[i]){
				collision = false;
			}//end if
		}//end for
	}else if (collision && size == MED_DIAMOND){
		for (int i = 0; i < FREE_CELLS_MED; i++){
			if (sprite_x(objectA) - sprite_x(objectB) == clearXmed[i] && sprite_y(objectA) - sprite_y(objectB) == clearYmed[i]){
				collision = false;
			}//end if
		}//end for
	}//end if
	
	return collision;
}//end collisionBetween



/*
checks if a diamond has collided with the spaceship
*/
bool spaceshipHit(void){
	//big
	for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
		if (collisionBetween(spaceship, big_diamonds[d], SPACESHIP)){
			return true;
		}//end if
	}//end for
	
	//med
	for (int d = 0; d < MAX_MED_DIAMONDS; d++){
		if (collisionBetween(spaceship, med_diamonds[d], SPACESHIP)){
			return true;
		}//end if
	}//end for	
	
	//sml
	for (int d = 0; d < MAX_SML_DIAMONDS; d++){
		if (collisionBetween(spaceship, sml_diamonds[d], SPACESHIP)){
			return true;
		}//end if
	}//end for	
	
	return false;
}//end spaceshipHit



/*
actions to take when a diamond is hit with a missile
	-splinter if big or med
	-destroy if sml
	-check if all diamonds are destroyed
*/
void diamondHit(sprite_id diamond, int size){
	
	int spawned = 0;
	
	int cAngle = atan(sprite_dx(diamond)/sprite_dy(diamond)) * (180/31.14159265);
	
	if (size == BIG_DIAMOND){//splinter big to 2 med
		for (int d = 0; d < MAX_MED_DIAMONDS; d++){
			if ( ! sprite_visible(med_diamonds[d])){
				sprite_show(med_diamonds[d]);
				
				sprite_move_to(med_diamonds[d], sprite_x(diamond), sprite_y(diamond));
				sprite_turn_to(med_diamonds[d], sprite_dx(diamond), sprite_dy(diamond));
				if (spawned == 0) sprite_turn(med_diamonds[d], cAngle + 45);
				if (spawned == 1) sprite_turn(med_diamonds[d], cAngle - 45);

				spawned = spawned + 1;
				if (spawned > 1) break;
			}//end if
		}//end for	
		
		
	} else if (size == MED_DIAMOND){//splinter med to 2 sml
		for (int d = 0; d < MAX_SML_DIAMONDS; d++){
			if ( ! sprite_visible(sml_diamonds[d])){
				sprite_show(sml_diamonds[d]);
				
				sprite_move_to(sml_diamonds[d], sprite_x(diamond), sprite_y(diamond));
				sprite_turn_to(sml_diamonds[d], sprite_dx(diamond), sprite_dy(diamond));
				if (spawned == 0) sprite_turn(sml_diamonds[d], cAngle + 45);
				if (spawned == 1) sprite_turn(sml_diamonds[d], cAngle - 45);

				spawned = spawned + 1;
				if (spawned > 1) break;
			}//end if
		}//end for
		
		
	}else {//check if last
		for (int d = 0; d < MAX_SML_DIAMONDS; d++){
			if (sprite_visible(sml_diamonds[d])){
				cleared_diamonds = false;
			}//end if
		}//end for
		
		for (int d = 0; d < MAX_MED_DIAMONDS; d++){
			if (sprite_visible(med_diamonds[d])){
				cleared_diamonds = false;
			}//end if
		}//end for
		
		for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
			if (sprite_visible(big_diamonds[d])){
				cleared_diamonds = false;
			}//end if
		}//end for
		
	}//end else
	
	//destroy hit diamond
	sprite_hide(diamond);
	sprite_turn_to(diamond, 0, 0);
	
}//end diamondHit



/*
actions to take when a missile collides with a diamond
*/
void missileHit(sprite_id missile, sprite_id diamond, int size){
	score = score + 1;
	
	sprite_hide(missile);
	
	diamondHit(diamond, size);	
}//end missileHit



/*
checks if a missile has collided with a diamond
*/
void missileHitCheck(void){
	for (int m = 0; m < MAX_MISSILES; m++){
		if (sprite_visible(missiles[m])){
			//big
			for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
				if (sprite_visible(big_diamonds[d])){
					if (collisionBetween(missiles[m], big_diamonds[d], BIG_DIAMOND)){
						missileHit(missiles[m], big_diamonds[d], BIG_DIAMOND);
					}//end if
				}//end if
			}//end for
			
			//med
			for (int d = 0; d < MAX_MED_DIAMONDS; d++){
				if (sprite_visible(med_diamonds[d])){
					if (collisionBetween(missiles[m], med_diamonds[d], MED_DIAMOND)){
						missileHit(missiles[m], med_diamonds[d], MED_DIAMOND);
					}//end if
				}//end if
			}//end for
			
			//sml
			for (int d = 0; d < MAX_SML_DIAMONDS; d++){
				if (sprite_visible(sml_diamonds[d])){
					if (collisionBetween(missiles[m], sml_diamonds[d], SML_DIAMOND)){
						missileHit(missiles[m], sml_diamonds[d], SML_DIAMOND);
					}//end if
				}//end if
			}//end for
			
			
		}//end if
	}//end for
}//end missileHit



/*
Tracks elapsed time
*/
void stepTime(void){
	timeMs = timeMs + 1;
	if (timeMs/(Ms_in_S/6) == 1){
		timeS = timeS + 1;
		timeMs = 0;
	}//end if
	if (timeS/S_in_M == 1){
		timeM = timeM + 1;
		timeS = 0;
	}
}//end step_time



/*
resets timer
*/
void resetTime(void){
	timeMs = 0;
	timeS = 0;
	timeM = 0;
}//end resetTime



/*
render missiles
*/
void renderMissiles(void){
	for (int m = 0; m < MAX_MISSILES; m++){
		renderObject(missiles[m]);
	}//end for
}//end renderMissiles



/*
render diamonds
*/
void renderDiamonds(void){
	//big
	for (int d = 0; d < MAX_BIG_DIAMONDS; d++){
		renderObject(big_diamonds[d]);
	}//end for
	
	//med
	for (int d = 0; d < MAX_MED_DIAMONDS; d++){
		renderObject(med_diamonds[d]);
	}//end for
	
	//sml
	for (int d = 0; d < MAX_SML_DIAMONDS; d++){
		renderObject(sml_diamonds[d]);
	}//end for
	
}//end renderDiamonds



/*
if the object is visible, render it
*/
void renderObject(sprite_id object){
	if (sprite_visible(object)){
		sprite_draw(object);
	}//end if
}//end renderObject



/*
Updates graphics of the playing field
*/
void updateGraphics(void){
	clear_screen();
	
	renderDiamonds();
	sprite_draw(spaceship);
	DrawBorder();
	DrawStatus();
	renderMissiles();
	
	show_screen();
}//end updateGraphics



/*
Set up Game
*/
void setup(void){
	sx = screen_width();
	sy = screen_height();
	
	//setup help screen
	help = sprite_create(sx/2 - help_w/2, sy/2 - help_h/2, help_w, help_h, help_image);
	sprite_hide(help);
	
	//show initial help screen
	helpDialog();

	//setup game screen
	DrawBorder();
	DrawStatus();

	//setup spaceship
	spaceship = sprite_create(sx/2, sy - 6, 7, 5, spaceship_image);
	sprite_draw(spaceship);

	//setup missile
	spawnMissiles();
	
	//setup diamond
	spawnDiamonds();
	
	show_screen();
}//end setup



/*
Play one turn of the game
*/
void process(void){
	
	
	//get input
	int key =get_char();

	//test for help (h)
	if (key == 'h') helpDialog();
	
	
	//test for quit (q)
	if (key == 'q') {
		gameOverDialog();
		return;
	}//end if
	
	//get player co-ordinates
	int px = round(sprite_x(spaceship));

	//move left (a)
	if (key == 'a' && px > 1) sprite_move(spaceship, -1, 0);

	//move right(d)
	if (key == 'd' && px < sx - sprite_width(spaceship) - 1) sprite_move(spaceship, 1, 0);

	//shoot missile (m)
	if ((key == 'j' || key == 'k' || key == 'l') && tm < 100) shootMissile(key);
	
	//diamond physics
	stepDiamonds();

	//missile physics
	stepMissiles();

	//update timer
	stepTime();	
	
	//check collisions
	if(spaceshipHit()){
		lives = lives - 1;
		score = 0;
		resetMissiles();
		resetDiamonds();
	}//end if
	
	missileHitCheck();
	
	//check if death
	if (lives <= 0) gameOverDialog();
	
	//check if all diamonds are destroyed
	if (cleared_diamonds){
		resetDiamonds();
	}//end if
	
	//update graphics
	updateGraphics();	
	
}//end process



/*
resets spaceship's location
*/
void resetSpaceship(void){
	sprite_move_to(spaceship, sx/2 - spaceship_w/2, sy - spaceship_h - 1);
}//end resetSpaceship



/*
Reset Game
*/
void resetGame(void){
	score = 0;
	lives = 10;
	resetTime();
	resetSpaceship();
	//destroy diamonds
	//destroy missiles
	resetMissiles();
	//respawn diamonds
	resetDiamonds();
}//end resetGame



/**
	- Program Entry Point-
**/
int main(void){
	setup_screen();
	setup();

	
	while (!game_over){
	process();
	timer_pause(DELAY);
	}

	cleanup_screen();
	return 0;
}//end main