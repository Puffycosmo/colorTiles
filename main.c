//#include <ti/getcsc.h>
#include <graphx.h>
#include <libload.h>
#include <keypadc.h>
#include <debug.h>
#include <sys/timers.h>
#include <fileioc.h>

/* Include the converted graphics file */
#include "gfx\gfx.h"
#include "gfx\newFont.h" 
#include "levels\levels.h"
uint8_t selectorA;
uint8_t selectorB;
int page = 0;
bool backSelected;
uint8_t get_single_key_pressed(void);
void drawRectangle(int x, int y, int width, int height, const gfx_sprite_t* tileType);
void drawTitleScreen(void);
void printArrow(bool erase, int selectorIndex);
void drawLevelSelect();
char* convertInt(int num);
void updateSelector();
void PlayScreen(int level);
void moveTiles(int direction);
int currentLevel = 0;
void transitionScreen(int score, int level);
void storeUndo(int allowedMoves, int col, int row, int currentBoard[col][row]);
void loadUndo(int allowedMoves, int col, int row, int currentBoard[col][row]);
void drawTutorialScreen();
void drawCreditsScreen();
void drawVictoryScreen();
int storeAppvar(int level, uint8_t score, bool write);
uint8_t selectedLevel;
int xFloodBuffer[4] = { 99, 99, 99, 99 };
int yFloodBuffer[4] = { 99, 99, 99, 99 };
bool fourMatched = false;
int gameState = 0; // 0 is title screen, 1 is level select

typedef struct {

	int row;
	int col;
	int data[100];

} undoState;

undoState undoBuffer[10];

int main(void)

{
	/* Initialize graphics drawing */
	gfx_Begin();
	//0 = on the title screen 
	/* Set the palette for sprites */
	gfx_SetPalette(global_palette, sizeof_global_palette, 0);
	drawTitleScreen();
	printArrow(false, 0);
	selectorA = 0;
	selectorB = 0;

	while (gameState != 10) { //10 means the game has quit 

		while (gameState == 0) {
			//on title screen 

			int keyCode = get_single_key_pressed();

			if (keyCode != 0) {
				dbg_printf("%d", keyCode);
				dbg_printf("\n");
			}

			if (keyCode == 4) {

				printArrow(true, selectorA);

				selectorA--;

				if (selectorA == 255) { //wraparound for 8-bit unsigned
					selectorA = 3;
				}
				printArrow(false, selectorA);

			}
			if (keyCode == 1) {

				printArrow(true, selectorA);

				selectorA++;

				if (selectorA == 4) { //wraparound for 8-bit unsigned
					selectorA = 0;
				}
				printArrow(false, selectorA);
			}

			if (keyCode == 9) {

				switch (selectorA) {
				case 0:
					drawLevelSelect();
					gameState = 1;
					break;
				case 3:
					gameState = 10;
					break;
				case 1:
					gameState = 3;
					drawTutorialScreen();
					break;
				case 2: 
					gameState = 4; //credits
					drawCreditsScreen();
					break; 
				}

			}


		}
		while (gameState == 1) { //level select state 

			updateSelector();

		}
		while (gameState == 2) { //currently playing state 
			PlayScreen(currentLevel);
		}
		while (gameState == 3 || gameState == 4) { //tutorial

			int keyCode = get_single_key_pressed();

			if (keyCode == 9) {

				gameState = 0;
				drawTitleScreen();
				printArrow(false, 0);
				selectorA = 0;

			}

		}
	}

	gfx_End();
	return 0;
}

void drawCreditsScreen() {
	
	
	drawRectangle(0, 0, 320, 240, grassTile);
	gfx_SetTextScale(3, 3);
	gfx_PrintStringXY("THANKS TO", 60, 10);

	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("THE C TOOLCHAIN", 10, 40);
	gfx_SetTextScale(1, 1);
	gfx_PrintStringXY("GREATEST ACHIEVEMENT OF MANKIND", 10, 55);
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("ADRIWEB", 10, 70);
	gfx_SetTextScale(1, 1);
	gfx_PrintStringXY("FOR CREATING THE LIBRARIES", 10, 85);
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("MY CAT", 10, 100);

	drawRectangle(2, 50, 16, 8, boardTileEmpty); //back button
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("BACK", 10, 210);

	gfx_TransparentSprite(backButtonBorder, 4, 196);

}

void drawTutorialScreen() {

	drawRectangle(0, 0, 320, 240, grassTile);
	gfx_SetTextScale(3, 3);
	gfx_PrintStringXY("TUTORIAL", 60, 10);
	gfx_SetTextScale(1, 1);
	gfx_PrintStringXY("- USE ARROW KEYS TO SHIFT THE TILES", 10, 40);
	gfx_PrintStringXY("- THE MORE MOVES YOU CONSERVE, THE HIGHER", 10, 50);
	gfx_PrintStringXY(" YOUR SCORE, EACH ADDITIONAL STAR IS A MOVE", 10, 60);
	gfx_PrintStringXY(" NOT USED", 10, 70);
	gfx_PrintStringXY("- USE CLEAR KEY TO EXIT A LEVEL", 10, 80);
	gfx_PrintStringXY("- USE SUBTRACT KEY TO UNDO A MOVE", 10, 90);
	gfx_PrintStringXY("- YOU WILL ALWAYS HAVE AT LEAST TWO MOVES", 10, 100);
	gfx_PrintStringXY(" MORE THAN NECESSARY", 10, 110);
	gfx_PrintStringXY("- MATCH 4 TILES OF THE SAME COLOR TO CLEAR", 10, 120);
	gfx_PrintStringXY(" THEM, CLEAR THE WHOLE BOARD TO SOLVE", 10, 130);
	gfx_PrintStringXY("- USE OBSTACLES TO MATCH TILES", 10, 140); 

	drawRectangle(2, 50, 16, 8, boardTileEmpty); //back button
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("BACK", 10, 210);

	gfx_TransparentSprite(backButtonBorder, 4, 196);

}

void drawTitleScreen(void) {
	//fill the background with grass 
	drawRectangle(0, 0, 80, 60, grassTile); //draws grass
	drawRectangle(48, 18, 24, 24, boardTileEmpty); //draws board 
	drawRectangle(48, 17, 24, 1, borderTileUp); //draws upper border 
	drawRectangle(48, 42, 24, 1, borderTileDown); //lower border 
	drawRectangle(47, 18, 1, 24, borderTileLeft); //left border
	drawRectangle(72, 18, 1, 24, borderTileRight); //right border 
	drawRectangle(47, 17, 1, 1, borderTopLeft);
	drawRectangle(72, 17, 1, 1, borderTopRight);
	drawRectangle(47, 42, 1, 1, borderBottomLeft);
	drawRectangle(72, 42, 1, 1, borderBottomRight);

	gfx_TransparentSprite(titleText, 33, 8);

	gfx_SetFontData(font);
	gfx_SetTextScale(2, 2);
	gfx_SetTextFGColor(0);

	gfx_PrintStringXY("PLAY", 40, 70);
	gfx_PrintStringXY("TUTORIAL", 40, 94);
	gfx_PrintStringXY("CREDITS", 40, 115);
	gfx_PrintStringXY("EXIT", 40, 136);

}
void drawLevelSelect(void) {

	drawRectangle(0, 0, 80, 60, grassTile);

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			//j is the y, i is the x 
			int num = (page * 24) + (i + (j * 6) + 1);
			int value = storeAppvar(num, 0, false);
			if (value < 4) {
				switch (value) {
				case 0:
					if (storeAppvar(num - 1, 0, false) != 0 || num == 1) {
						drawRectangle(11 + (i * 10), 10 + (j * 10), 8, 8, boardTileEmpty);
					}
					else {
						gfx_Sprite(lockedButton, 44 + (i * 40), 40 + (j * 40));
					}
					break;
				case 1:
					gfx_Sprite(bronzeButton, 44 + (i * 40), 40 + (j * 40));
					break;
				case 2:
					gfx_Sprite(silverButton, 44 + (i * 40), 40 + (j * 40));
					break;
				case 3:
					gfx_Sprite(goldButton, 44 + (i * 40), 40 + (j * 40));
					break;

				}
			}
			else {
				gfx_Sprite(goldButton, 44 + (i * 40), 40 + (j * 40));

			}
			
			
			gfx_SetTextScale(1, 1);
			gfx_PrintStringXY(convertInt(num), 52 + (i * 40), 43 + (j * 40));
		}
	}
	drawRectangle(2, 50, 16, 8, boardTileEmpty); //back button
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("BACK", 10, 210);

	gfx_PrintStringXY("PAGE ", 120, 220); 
	gfx_PrintString(convertInt(page + 1));


}

void updateSelector(void) {

	int keyCode = get_single_key_pressed();

	if (keyCode == 1 || keyCode == 2 || keyCode == 3 || keyCode == 4 || keyCode == 9) {

		if (backSelected == false) { //erase normal border 
			drawRectangle(10 + (selectorA * 10), 9 + (selectorB * 10), 10, 1, grassTile); //erases top horizontal
			drawRectangle(10 + (selectorA * 10), 9 + (selectorB * 10), 1, 10, grassTile); //erases left vertical
			drawRectangle(19 + (selectorA * 10), 9 + (selectorB * 10), 1, 10, grassTile); //erases right vertical
			drawRectangle(10 + (selectorA * 10), 18 + (selectorB * 10), 10, 1, grassTile); //erases bottom horizontal
		}


		switch (keyCode) {

		case 1: //down
			if (backSelected == false) {
				if (selectorB <= 2) {
					selectorB++; //selectorB is vertical, selector A is horizontal 
				}
				else {
					backSelected = true;
					selectorA = 0;
					selectorB = 0;
				}
			}
			else {
				drawRectangle(1, 49, 18, 1, grassTile); //erases top horizontal 
				drawRectangle(1, 49, 1, 10, grassTile); //erases left vertical 
				drawRectangle(1, 58, 18, 1, grassTile); //erases bottom horizontal 
				drawRectangle(18, 49, 1, 10, grassTile); //erases right vertical (does not work) 

				backSelected = false;
			}
			break;
		case 2: //left button
			if (backSelected == false) {
				if (selectorA >= 1) {
					selectorA--;
				}
				else if (page == 1) {
					page--;
					drawLevelSelect();
				}
			}
			else {
				drawRectangle(1, 49, 18, 1, grassTile); //erases top horizontal 
				drawRectangle(1, 49, 1, 10, grassTile); //erases left vertical 
				drawRectangle(1, 58, 18, 1, grassTile);
				drawRectangle(18, 49, 1, 10, grassTile);

				backSelected = false;
			}
			break;
		case 3: //moves right 
			if (backSelected == false) {
				if (selectorA <= 4) {
					selectorA++;
				}
				else if (page == 0) {
					page++;
					selectorA = 0; 
					drawLevelSelect();
				}
			}
			else {
				drawRectangle(1, 49, 18, 1, grassTile); //erases top horizontal 
				drawRectangle(1, 49, 1, 10, grassTile); //erases left vertical 
				drawRectangle(1, 58, 18, 1, grassTile);
				drawRectangle(18, 49, 1, 10, grassTile);

				backSelected = false;
			}
			break;
		case 4:
			if (backSelected == false) {
				if (selectorB >= 1) {
					selectorB--;
				}
				else {
					backSelected = true;
					selectorA = 0;
					selectorB = 0;
				}
			}
			else {
				drawRectangle(1, 49, 18, 1, grassTile); //erases top horizontal 
				drawRectangle(1, 49, 1, 10, grassTile); //erases left vertical 
				drawRectangle(1, 58, 18, 1, grassTile);
				drawRectangle(18, 49, 1, 10, grassTile);

				backSelected = false;
			}
			break;
		case 9:

			if (backSelected == true) {
				drawRectangle(1, 49, 18, 1, grassTile); //erases top horizontal 
				drawRectangle(1, 49, 1, 10, grassTile); //erases left vertical 
				drawRectangle(1, 58, 18, 1, grassTile);
				drawRectangle(18, 49, 1, 10, grassTile);
				drawTitleScreen();
				printArrow(false, 0);
				dbg_printf("gameState changed \n");
				gameState = 0;
			}
			else {
				currentLevel = ((selectorA + (selectorB * 6)) + 1) + (page * 24);
				if (storeAppvar((currentLevel - 1), 1, false) > 0 || currentLevel == 1) {
					drawRectangle(0, 0, 320, 240, grassTile);
					gameState = 2;
				}

			}
			break;

		}
	}
	if (gameState == 1) {
		if (backSelected == false && gameState == 1) {
			gfx_TransparentSprite(selectorBorder, 40 + (selectorA * 40), 36 + (selectorB * 40));
		}
		else if (backSelected == true) {
			gfx_TransparentSprite(backButtonBorder, 4, 196);
		}
	}


}
void PlayScreen(int level) {

	//copy data from levels.c to this function in currentboard
	bool boardClear = false;
	int blockSize;
	int y = levels[level - 1].rows;
	int x = levels[level - 1].cols;
	int currentBoard[y][x];
	int allowedMoves = 2;

	for (int t = 1; t < 9; t++) {
		for (int i = 0; i < levels[level - 1].rows * levels[level - 1].cols; i++) {
			if (levels[level - 1].data[i] == t) {
				allowedMoves++;
				break;
			}
		}
	}
	int maxMoves = allowedMoves;
	dbg_printf("%d", allowedMoves);
	dbg_printf("\n");

	for (int i = 0; i < x; i++) {
		for (int j = 0; j < y; j++) {
			currentBoard[j][i] = levels[level - 1].data[(i * x) + j];
		}
	}

	int dimension = levels[level - 1].rows;

	if (dimension == 3 || dimension == 4 || dimension == 6 || dimension == 8) {
		//draw 48*48 
		drawRectangle(16, 11, 48, 1, borderTileUp);
		drawRectangle(15, 12, 1, 49, borderTileLeft);
		drawRectangle(64, 12, 1, 48, borderTileRight);
		drawRectangle(15, 11, 1, 1, borderTopLeft);
		drawRectangle(64, 11, 1, 1, borderTopRight);

		drawRectangle(16, 12, 48, 48, boardTileEmpty);
		blockSize = (48 / dimension);
		dbg_printf("dimension = 3 \n");

	}
	if (dimension == 5 || dimension == 9) {
		//YES I KNOW THIS ISN'T NECESSARY AND WE CAN JUST CALCULATE THE X AND Y POSITIONS OF THESE BORDERS WITH DIMENSION BUT THAT IS REALLY BUGGY AND COPY PASTING IS ACTUALLY FASTER
		drawRectangle(18, 14, 45, 1, borderTileUp);
		drawRectangle(17, 15, 1, 46, borderTileLeft);
		drawRectangle(63, 15, 1, 46, borderTileRight);
		drawRectangle(17, 14, 1, 1, borderTopLeft);
		drawRectangle(63, 14, 1, 1, borderTopRight);

		drawRectangle(18, 15, 45, 45, boardTileEmpty);
		blockSize = (45 / dimension);
		dbg_printf("dimension = 5 \n");

	}
	if (dimension == 7) {

		drawRectangle(19, 17, 42, 1, borderTileUp);
		drawRectangle(18, 18, 1, 42, borderTileLeft);
		drawRectangle(61, 18, 1, 42, borderTileRight);
		drawRectangle(18, 17, 1, 1, borderTopLeft);
		drawRectangle(61, 17, 1, 1, borderTopRight);

		drawRectangle(19, 18, 42, 42, boardTileEmpty);
		blockSize = 6;
		dbg_printf("dimension = 7 \n");
	}
	if (dimension == 10) {
		drawRectangle(15, 10, 50, 50, boardTileEmpty);
		blockSize = (50 / dimension);
	}
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("LEVEL:", 110, 20);
	gfx_PrintStringXY(convertInt(level), 190, 20);

	gfx_SetTextScale(1, 1);
	gfx_PrintStringXY("MOVES:", 10, 60);
	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);

	while (boardClear == false && gameState == 2) {
		boardClear = true;
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {

				gfx_sprite_t* tilePtr;
				switch (currentBoard[i][j]) {
				case 1:
					tilePtr = redTile;
					break;
				case 2:
					tilePtr = blueTile;
					break;
				case 3:
					tilePtr = yellowTile;
					break;
				case 4:
					tilePtr = orangeTile;
					break;
				case 5:
					tilePtr = whiteTile;
					break;
				case 6:
					tilePtr = cyanTile;
					break;
				case 7:
					tilePtr = purpleTile;
					break;
				case 8:
					tilePtr = grayTile;
					break;
				case 9:
					tilePtr = grassTile;
					break;
				case 0:
					tilePtr = boardTileEmpty;
					break;


				}
				if (dimension == 3 || dimension == 4 || dimension == 6 || dimension == 8) {
					drawRectangle((i * blockSize) + 16, (j * blockSize) + 12, blockSize, blockSize, tilePtr);
				}
				if (dimension == 5 || dimension == 9) {
					drawRectangle((i * blockSize) + 18, (j * blockSize) + 15, blockSize, blockSize, tilePtr);

				}
				if (dimension == 7) {
					drawRectangle((i * blockSize) + 19, (j * blockSize) + 18, blockSize, blockSize, tilePtr);

				}
			}
		}
		int keyCode = get_single_key_pressed();


		switch (keyCode) {
		case 1: //moves down
			if (allowedMoves > 0) {
				storeUndo(allowedMoves, x, y, currentBoard);
				for (int i = 0; i < x; i++) {

					for (int j = (y - 1); j >= 0; j--) {

						if (currentBoard[i][j] == 0 && j > 0 && currentBoard[i][j - 1] != 9) {
							currentBoard[i][j] = currentBoard[i][j - 1];
							currentBoard[i][j - 1] = 0;
						}


					}

				}

				allowedMoves--;
				drawRectangle(3, 17, 10, 6, grassTile);
				gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);
			}


			break;
		case 2: //moves left
			if (allowedMoves > 0) {
				storeUndo(allowedMoves, x, y, currentBoard);
				for (int j = 0; j < y; j++) {

					for (int i = 0; i < x; i++) {
						if (currentBoard[i][j] == 0 && i < (x - 1) && currentBoard[i + 1][j] != 9) {
							currentBoard[i][j] = currentBoard[i + 1][j];
							currentBoard[i + 1][j] = 0;
						}
					}
				}

				allowedMoves--;
				drawRectangle(3, 17, 10, 6, grassTile);
				gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);
			}
			break;
		case 3: //moves right 
			if (allowedMoves > 0) {
				storeUndo(allowedMoves, x, y, currentBoard);
				for (int j = 0; j < y; j++) {

					for (int i = (x - 1); i >= 0; i--) {
						if (currentBoard[i][j] == 0 && i > 0 && currentBoard[i - 1][j] != 9) {
							currentBoard[i][j] = currentBoard[i - 1][j];
							currentBoard[i - 1][j] = 0;
						}
					}
				}

				allowedMoves--;
				drawRectangle(3, 17, 10, 6, grassTile);
				gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);
			}
			break;

		case 4: //moves up
			if (allowedMoves > 0) {
				storeUndo(allowedMoves, x, y, currentBoard);
				for (int i = 0; i < x; i++) {

					for (int j = 0; j < y; j++) {
						if (currentBoard[i][j] == 0 && j < (y - 1) && currentBoard[i][j + 1] != 9) {
							currentBoard[i][j] = currentBoard[i][j + 1];
							currentBoard[i][j + 1] = 0;
						}
					}
				}

				allowedMoves--;
				drawRectangle(3, 17, 10, 6, grassTile);
				gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);
			}
			break;

		case 15:

			gameState = 1;
			drawRectangle(0, 0, 320, 240, grassTile);
			drawLevelSelect();

			break;

		case 11:
			if (allowedMoves != maxMoves) {
				allowedMoves++; //load undostate back in
				drawRectangle(3, 17, 10, 6, grassTile);
				gfx_PrintStringXY(convertInt(allowedMoves), 10, 70);
				for (int i = 0; i < x; i++) {
					for (int j = 0; j < y; j++) {
						dbg_printf("%d", undoBuffer[9 - allowedMoves].data[(i * y) + j]);
						dbg_printf("\n");
						currentBoard[i][j] = undoBuffer[9 - allowedMoves].data[(i * y) + j];

					}
				}
			}
			break;
		}




		for (int t = 1; t < 9; t++) {

			//clear out the buffers 

			for (int i = 0; i < 4; i++) {
				xFloodBuffer[i] = 0;
				yFloodBuffer[i] = 0;
			}

			int index = 0;
			for (int i = 0; i < x; i++) { //t here stands for tile, we're going to be checking every single tile type for matches 
				for (int j = 0; j < y; j++) {

					if (currentBoard[i][j] == t) { //matching tile type found 
						boardClear = false;
						xFloodBuffer[index] = i;
						yFloodBuffer[index] = j;
						index++;
					}

				}
			}

			//compare the values in the arrays against each other 

			int connections = 0;
			for (int i = 0; i < 4; i++) {

				for (int j = 0; j < 4; j++) {

					if ((((xFloodBuffer[i] - xFloodBuffer[j]) * (xFloodBuffer[i] - xFloodBuffer[j])) + ((yFloodBuffer[i] - yFloodBuffer[j]) * (yFloodBuffer[i] - yFloodBuffer[j]))) == 1) {

						connections++;

					}

				}

			}
			if (connections >= 6) {
				//found a matching quadrouplet, clear them out 

				for (int i = 0; i < 4; i++) {
					currentBoard[xFloodBuffer[i]][yFloodBuffer[i]] = 0; //remove the matching tiles 
					dbg_printf("Clear");
				}

			}

		}



	}

	if (boardClear == true) {

		storeAppvar(currentLevel, allowedMoves + 1, true);
		if (currentLevel != 48) {
			transitionScreen(allowedMoves + 1, level);
			currentLevel++;
		}
		else {
			drawVictoryScreen();
			gameState = 1;
			page = 0;
			selectorA = 0;
			selectorB = 0;
			drawLevelSelect();
		}

		

	}

}

void drawVictoryScreen() {

	gfx_FillScreen(1);
	gfx_SetTextFGColor(2);
	gfx_SetTextScale(3, 3);
	sleep(1);
	gfx_PrintStringXY("YOU DID IT!!!", 30, 20);
	gfx_SetTextScale(2, 2);
	sleep(1);
	gfx_PrintStringXY("THANKS FOR PLAYING", 30, 50);
	gfx_PrintStringXY("ANY KEY TO CONTINUE", 25, 215); 

	gfx_ScaledTransparentSprite_NoClip(endScreen, 60, 40, 2, 2);

	int keyCode = 0; 

	while (keyCode == 0) {

		keyCode = get_single_key_pressed();
	}

	gfx_SetTextFGColor(0);


}

int storeAppvar(int level, uint8_t score, bool write) {

	int index = level - 1;
	uint8_t handle = ti_Open("CTLEVEL", "r+");
	ti_SetArchiveStatus(false, handle);
	if (handle == 0) { //appvar does not exist 
		dbg_printf("Does not exist");
		handle = ti_Open("CTLEVEL", "w+"); //creates appvar 
		//use saveStates later, this is unused right now 
		uint8_t saveStates[48];

		for (int i = 0; i < 48; i++) {
			saveStates[i] = 0;
		}
		ti_Write(saveStates, sizeof(uint8_t), 48, handle);

	}

	//handle does exist 
	uint8_t value;

	ti_Seek(index, SEEK_SET, handle);
	ti_Read(&value, sizeof(uint8_t), 1, handle);

	if ((value < score || value > 3) && write == true) {
		ti_Seek(index * sizeof(uint8_t), SEEK_SET, handle); //testing only 
		ti_Write(&score, sizeof(uint8_t), 1, handle);

	}

	ti_Seek(index, SEEK_SET, handle);
	ti_Read(&value, sizeof(uint8_t), 1, handle);

	ti_SetArchiveStatus(true, handle);

	ti_Close(handle);
	dbg_printf("\nvalue: ");
	dbg_printf("%d", value);

	return value; 

}

void storeUndo(int allowedMoves, int col, int row, int currentBoard[col][row]) {
	//store the undo moves in the buffer 

	undoBuffer[9 - allowedMoves].row = row;
	undoBuffer[9 - allowedMoves].col = col;

	for (int i = 0; i < col; i++) {
		for (int j = 0; j < row; j++) {

			undoBuffer[9 - allowedMoves].data[(i * row) + j] = currentBoard[i][j];
			dbg_printf("%d", undoBuffer[9 - allowedMoves].data[(i * row) + j]);
			dbg_printf("\n");
		}
	}




}

void drawRectangle(int x, int y, int width, int height, const gfx_sprite_t* tileType) {
	//j is x, i is y, prints a rectangle of the appropriate tile type 
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {

			gfx_Sprite(tileType, (x * 4) + (j * 4), (y * 4) + (i * 4));

		}
	}


}

uint8_t get_single_key_pressed(void) {
	static uint8_t last_key;
	uint8_t only_key = 0;
	kb_Scan();
	for (uint8_t key = 1, group = 7; group; --group) {
		for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
			if (kb_Data[group] & mask) {
				if (only_key) {
					last_key = 0;
					return 0;
				}
				else {
					only_key = key;
				}
			}
		}
	}
	if (only_key == last_key) {
		return 0;
	}
	last_key = only_key;
	return only_key;
}

void transitionScreen(int score, int level) {

	gfx_FillScreen(1);
	gfx_SetTextFGColor(2);
	gfx_SetTextScale(3, 3);
	gfx_PrintStringXY("LEVEL", 30, 20);
	gfx_PrintStringXY(convertInt(level), 150, 20);

	sleep(1);

	gfx_PrintStringXY("COMPLETE", 30, 50);

	for (int i = 0; i < score; i++) {

		gfx_TransparentSprite(star, 10 + (i * 100), 70);
		sleep(1);

	}

	gfx_SetTextScale(2, 2);
	gfx_PrintStringXY("CLEAR KEY TO EXIT", 30, 180);
	gfx_SetTextScale(1, 1);
	gfx_PrintStringXY("RIGHT ARROW TO CONTINUE", 30, 200);


	int keyPress = 0;

	while (keyPress != 3 && keyPress != 15) {

		keyPress = get_single_key_pressed();

	}

	if (keyPress == 15) {
		gfx_SetTextFGColor(0);
		drawLevelSelect();
		gameState = 1;

	}
	else {
		gfx_SetTextFGColor(0);
		drawRectangle(0, 0, 320, 240, grassTile);
	}



}

void printArrow(bool erase, int selectorIndex) {



	if (erase == false) {

		gfx_Sprite(pointer, 30, (selectorIndex * 21) + 75);

	}
	if (erase == true) {
		drawRectangle(7, 17, 4, 30, grassTile);
	}

}
char* convertInt(int num) {
	static char convertedString[2];
	int tensNumber = 0;

	for (int i = 1; num > 9; i++) {

		num -= 10;
		tensNumber = i;

	}

	convertedString[0] = tensNumber + '0';
	convertedString[1] = num + '0';

	return convertedString;
}
