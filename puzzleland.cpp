/*	Puzzleland
 *	Author: Caleb Baker
 *	Date:	August 11, 2017
 *	A simple ASCII art turn-based (but feels like real time) puzzle/exploration game.
 */


#include <cstdio>		// For printf.
#include <cstdint>		// For uint16_t and the like.
#include <termios.h>	// For editing terminal settings.
#include <unistd.h>


// A few global variables.
char board[3200];	// Grid representing  current room.
uint8_t height;		// Height of current room.
uint8_t width;		// Width of current room.
uint8_t flags;		// Used as an array of boolean status flags.
char input;			// User input.


// Print the current room to the screen.
void print() {

	// Clear the screen.
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	// Print the room.
	for (uint8_t i = 0; i < height; i++) {
		for (uint8_t j = 0; j < width; j++) {
			if ((flags & 0x20) == 0x20 && board[width * i + j] == 'X') {
				printf("Y");
			}
			else {
				printf("%c", board[width * i + j]);
			}
		}
		printf("\n");
	}
}


// Change terminal settings to get one character of input at a time.
termios noCanon() {
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);				// Save old settings to a variable.
	newt = oldt;								// Copy setting variable.
	newt.c_lflag &= ~(ICANON | ECHO);			// Change setting variable.
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);	// Apply change.
	return oldt;
}


// Make a horizontal line of c.
void horizontal(uint16_t start, uint8_t length, char c) {
	for (uint8_t i = 0; i < length; i++) {
		board[start + i] = c;
	}
}


// Make a vertical line of c.
void vertical(uint16_t start, uint16_t length, char c) {
	length *= width;
	for (uint16_t i = 0; i < length; i += width) {
		board[start + i] = c;
	}
}


// Make a /diagonal line of c.
void leftDiag(uint16_t start, uint16_t length, char c) {
	length *= width - 1;
	for (uint16_t i = 0; i < length; i += width - 1) {
		board[start + i] = c;
	}	
}


// Make a \diagonal line of c.
void rightDiag(uint16_t start, uint16_t length, char c) {
	length *= width + 1;
	for (uint16_t i = 0; i < length; i += width + 1) {
		board[start + i] = c;
	}	
}


// Make a horizontal wall.
void horizontalWall(uint16_t start, uint8_t length) {
	horizontal(start, length, '-');
}

// Make a vertical wall.
void verticalWall(uint16_t start, uint16_t length) {
	vertical(start, length, '-');
}

// Make a /diagonal wall.
void leftDiagWall(uint16_t start, uint16_t length) {
	leftDiag(start, length, '-');
}

// Make a \diagonal wall.
void rightDiagWall(uint16_t start, uint16_t length) {
	rightDiag(start, length, '-');
}


// Make walls around the edge of the room and empty the middle.
void edgeWalls() {

	horizontalWall(0, width);	// Top wall.

	// Side walls.
	uint8_t i;
	for (i = 1; i < (height - 1); i++) {
		board[i * width] = '-';
		for (uint8_t j = 1; j < width - 1; j++) {
			board[width * i + j] = ' ';
		}
		board[width * (i + 1) - 1] = '-';
	}

	horizontalWall(width * (height - 1), width);	// Bottom wall.
}


// Sets *newPosition to be a knight's move away from position.
void moveKnight(uint16_t position, uint16_t *newPosition) {
	switch(input) {
		case 'y':
			if (position % width > 1) {
				*newPosition -= (2 + width);
			}
			break;
		case 'u':
			if (position > 2 * width) {
				*newPosition -= (1 + 2 * width);
			}
			break;
		case 'i':
			if (position > 2 * width) {
				*newPosition += (1 - 2 * width);
			}
			break;
		case 'o':
			if (position % width < width - 2) {
				*newPosition += (2 - width);
			}
			break;
		case 'h':
			if (position % width > 1) {
				*newPosition += (width - 2);
			}
			break;
		case 'j':
			if (position < width * (height - 2)) {
				*newPosition += (2 * width - 1);
			}
			break;
		case 'k':
			if (position < width * (height - 2)) {
				*newPosition += (2 * width + 1);
			}
			break;
		case 'l':
			if (position % width < width - 2) {
				*newPosition += (2 + width);
			}
	}
}


// Clears the space something was at if it hasn't already been overwritten.
void clear(uint16_t pos, char old) {
	if (board[pos] == old) {
		board[pos] = ' ';
	}
}


// Moves the '!' at *pos one space towards x.
void chase(uint32_t *pos, uint16_t x) {
	if (board[*pos] == '!') {
		board[*pos] = ' ';

		// Calculate the horizontal position of *pos and x.
		uint8_t xHor = x % width;
		uint8_t posHor = (*pos) % width;

		// Figure out where '!' needs to move to.
		uint32_t newPos = *pos;
		if (xHor > posHor) {
			newPos++;
		}
		else if (xHor < posHor) {
			newPos--;
		}
		else if (x > *pos) {
			newPos += width;
		}
		else {
			newPos -= width;
		}

		// Check if there's a wall in the way.
		if (board[newPos] != '-') {
			*pos = newPos;
		}

		board[*pos] = '!';	// Update board.
	}
}


// Causes the '!' at *pos to imitate the players actions.
void copy(uint32_t *pos, uint16_t x) {
	if (board[*pos] == '!') {
		board[*pos] = ' ';

		// Figure out where to move to.
		uint16_t newPosition = (uint16_t) *pos;
		switch (input) {
			case 'w':
				newPosition -= width;
				break;
			case 'a':
				newPosition--;
				break;
			case 's':
				newPosition += width;
				break;
			case 'd':
				newPosition++;
		}
		moveKnight((uint16_t) *pos, &newPosition);

		// Check if new position is clear.
		if (board[newPosition] == ' ' || board[newPosition] == 'X' || board[newPosition] == '!') {
			*pos = newPosition;
		}

		board[*pos] = '!';	// Update board.
	}
}


// Make an '!' waddle back and forth between two positions.
// Used in hall.
void danger(uint32_t *pos, uint16_t x) {
	clear((uint16_t) *pos, '!');
	if(*pos == 81) {
		*pos = 82;
	}
	else {
		*pos = 81;
	}
	board[*pos] = '!';
}


// Makes the '?' blink in and out of existence. If '?' is hit, door appears and flag set.
// Used in prison.
void button(uint32_t *signal, uint16_t x) {

	// If ? is hit.
	if ((*signal & 0x80000000) == 0x80000000) {
		board[14] = 'd';	// Make door.
		flags |= 1;			// Signal for room to change.
	}

	// Blink in and out of existence.
	if (board[6] == ' ') {
		board[6] = '?';
	}
	else {
		board[6] = ' ';
	}
}


// Makes the door appear when the ? in blocks is hit.
void reveal(uint32_t *signal, uint16_t x) {
	if ((*signal >> 31) == 1) {
		board[3] = 'w';
		board[87] = 's';
	}
}


// Makes a giant wall of !'s spawn at the bottom and gradually move up.
void destroy(uint32_t *pos, uint16_t x) {
	if (*pos != 0) {
		*pos -= 80;		// Move up.

		// If still inside the outer wall, replace the next layer of stuff with !'s.
		if ((int) *pos > 80) {
			for (uint16_t i = *pos; i < *pos + 78; i++) {
				board[i] = '!';
			}
		}

		// If wall o death has progressed a bit, let the back of the wall o death fade.
		if ((int) *pos > -160 && (int) *pos < 2870) {
			for (uint16_t i = *pos + 240; i < *pos + 318; i++) {
				board[i] = ' ';
			}
		}

		// If the wall o death has left the room, stop messing with it.
		else if ((int) *pos < -200) {
			*pos = 0;
		}
	}
}


// Makes a wall of !'s travel horizontally.
void killHor(uint32_t *pos, uint16_t x) {
	uint16_t p = (uint16_t) *pos;
	for (uint16_t i = 0; i < 750; i += 80) {
		board[1681 + p + i] = ' ';
	}
	*pos -= p;
	if ((*pos & 0x80000000) == 0x80000000) {
		p++;
	}
	else {
		p += 77;
	}
	p %= 78;
	*pos += p;
	for (uint16_t i = 0; i < 750; i += 80) {
		board[1681 + p + i] = '!';
	}
}


// Makes a wall of !'s travel vertically.
void killVert(uint32_t *pos, uint16_t x) {
	uint16_t p = (uint16_t) *pos;
	for (uint16_t i = 0; i < 78; i++) {
		board[1681 + (80 * p) + i] = ' ';
	}
	*pos -= p;
	if ((*pos & 0x80000000) == 0x80000000) {
		p++;
	}
	else {
		p += 9;
	}
	p %= 10;
	*pos += p;
	for (uint16_t i = 0; i < 78; i++) {
		board[1681 + (80 * p) + i] = '!';
	}
}


// Blocks the door the player entered from and creates a new one that's obnoxious to get to.
// Used in knightsMove.
void knight(uint32_t *signal, uint16_t x) {
	if ((*signal >> 31) == 1) {
		board[1970] = '-';
		board[1850] = 'a';
	}
}

// Spawns a line of B's that overlaps with one of the walls.
// Used in final room.
void change(uint32_t *signal, uint16_t x) {
	if ((*signal >> 31) == 1) {
		vertical(123, 5, 'B');
		*signal = 0;
	}
}


// A room with an enemy that chases you and a bunch of pointless m's that disapear when you step on them.
uint16_t mudRoom(char c, void (** action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 10;
	height = 10;
	horizontalWall(0, 10);
	uint8_t i;
	for (i = 1; i < 9; i++) {
		board[i * 10] = '-';
		for (uint8_t j = 1; j < 9; j++) {
			board[10 * i + j] = 'm';
		}
		board[10 * (i + 1) - 1] = '-';
	}
	horizontalWall(90, 10);
	board[5] = 'w';
	board[95] = 's';
	board[45] = '!';
	action[0] = chase;
	data[0] = 45;
	action[1] = nullptr;
	if (c == 'w') {
		return 85;
	}
	else {
		return 15;
	}
}

uint16_t blocks(char c, void (** action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 7;
	height = 13;
	edgeWalls();
	horizontal(29, 2, 'B');
	horizontal(32, 2, 'B');
	horizontal(57, 2, 'B');
	horizontal(60, 2, 'B');
	horizontal(43, 2, '-');
	horizontal(46, 2, '-');
	board[9] = 'B';
	board[11] = 'B';
	board[17] = 'B';
	board[38] = 'B';
	board[52] = 'B';
	board[73] = 'B';
	board[79] = 'B';
	board[81] = 'B';
	board[45] = '?';
	// board[35] = 'a';
	action[0] = reveal;
	data[0] = 0;
	action[1] = nullptr;
	if (c == 'w') {
		board[87] = 's';
		return 80;
	}
	else {
		board[3] = 'w';
		return 10;
	}
}

uint16_t hall(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 5;
	height = 40;
	edgeWalls();
	board[2] = 'w';
	board[197] = 's';
	board[81] = '!';
	action[0] = danger;
	data[0] = 82;
	action[1] = nullptr;
	if (c == 'w') {
		return 192;
	}
	else {
		return 7;
	}
}

uint16_t tele(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 7;
	height = 11;
	edgeWalls();
	horizontal(36, 5, '-');
	board[24] = '*';
	board[52] = '*';
	board[3] = 'w';
	board[73] = 's';
	action[0] = nullptr;
	data[1] = 24;
	data[2] = 52;
	if (c == 'w') {
		return 66;
	}
	else {
		return 10;
	}
}

uint16_t prison(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 5;
	height = 5;
	edgeWalls();
	action[0] = button;
	data[0] = 0;
	action[1] = nullptr;
	return 12;
}

uint16_t secret(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 7;
	height = 10;
	edgeWalls();
	action[0] = nullptr;
	board[16] = '-';
	board[18] = '-';
	board[37] = '-';
	board[39] = '-';
	board[51] = '-';
	board[53] = '-';
	board[3] = 'w';
	board[66] = 's';
	switch (c) {
		case 'w':
			return 59;
		case 's':
			return 10;
		default:
			return 31;
	}
}

uint16_t bigRoom(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 80;
	height = 40;
	edgeWalls();
	board[40] = 'w';
	board[1600] = 'a';
	board[3160] = 's';
	board[1679] = 'd';
	action[0] = nullptr;
	switch(c) {
		case 'w':
			return 3080;
		case 'a':
			return 1678;
		case 's':
			return 120;
		default:
			return 1601;
	}
}

uint16_t labyrinth(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 80;
	height = 30;
	edgeWalls();
	horizontalWall(402, 59);
	horizontalWall(724, 57);
	horizontalWall(883, 57);
	horizontalWall(1021, 17);
	horizontalWall(1121, 60);
	horizontalWall(1183, 16);
	horizontalWall(1246, 14);
	horizontalWall(1284, 21);
	horizontalWall(1309, 7);
	horizontalWall(1400, 38);
	horizontalWall(1444, 20);
	horizontalWall(1556, 43);
	horizontalWall(1714, 44);
	horizontalWall(1764, 15);
	horizontalWall(1872, 47);
	horizontalWall(1923, 8);
	horizontalWall(1932, 9);
	horizontalWall(2030, 48);
	horizontalWall(2085, 16);
	horizontalWall(2188, 51);
	verticalWall(91, 3);
	verticalWall(111, 3);
	verticalWall(118, 3);
	verticalWall(131, 3);
	verticalWall(141, 11);
	verticalWall(164, 3);
	verticalWall(179, 3);
	verticalWall(193, 3);
	verticalWall(205, 3);
	verticalWall(218, 3);
	verticalWall(482, 8);
	verticalWall(1202, 13);
	verticalWall(1202, 2);
	verticalWall(1305, 13);
	verticalWall(1307, 10);
	verticalWall(1526, 2);
	verticalWall(1530, 2);
	verticalWall(1534, 2);
	verticalWall(1538, 2);
	verticalWall(1541, 8);
	verticalWall(1608, 2);
	verticalWall(1612, 2);
	verticalWall(1616, 2);
	verticalWall(1624, 9);
	verticalWall(1629, 3); 
	leftDiag(1239, 13, '-');
	leftDiagWall(1262, 2);
	rightDiagWall(1389, 4);
	board[150] = '-';
	board[235] = '-';
	board[304] = '-';
	board[470] = '-';
	board[544] = '-';
	board[634] = '-';
	board[709] = '-';
	board[793] = '-';
	board[866] = '-';
	board[1100] = '-';
	board[1325] = '-';
	board[1473] = '-';
	board[1488] = '-';
	board[1498] = '-';
	board[1523] = '-';
	board[1660] = '-';
	board[1818] = '-';
	board[1979] = '-';
	board[301] = ' ';
	board[701] = ' ';
	board[1409] = ' ';
	board[1422] = ' ';
	board[1561] = ' ';
	board[1575] = ' ';
	board[1589] = ' ';
	board[1739] = ' ';
	board[1874] = ' ';
	board[1888] = ' ';
	board[1913] = ' ';
	board[2044] = ' ';
	board[2223] = ' ';
	board[1271] = 'w';
	board[1200] = 'a';
	board[1043] = 's';
	board[1041] = 'd';
	action[0] = nullptr;
	switch(c) {
		case 'w':
			return 963;
		case 'a':
			return 961;
		case 's':
			return 1351;
		default:
			return 1201;
	}
}

uint16_t blockRoom(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 11;
	height = 10;
	edgeWalls();
	horizontal(27, 4, '-');
	horizontal(49, 2, '-');
	horizontal(96, 2, 'B');
	vertical(14, 4, '-');
	vertical(41, 2, '-');
	vertical(74, 2, '-');
	vertical(58, 4, 'B');
	vertical(68, 3, 'B');
	board[62] = 'B';
	board[70] = 'B';
	board[64] = '!';
	board[1] = 'w';
	board[65] = 'd';
	action[0] = nullptr;
	return 12;
}

uint16_t frontRoom(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 10;
	height = 10;
	edgeWalls();
	board[5] = 'w';
	board[95] = 's';
	action[0] = nullptr;
	if (c == 's') {
		return 15;
	}
	board[45] = 'X';
	return 45;
}

uint16_t knightsMove(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 50;
	height = 40;
	edgeWalls();
	horizontal(1853, 34, '-');
	for(uint16_t i = 52; i < 88; i++) {
		vertical(i, 37, '-');
	}
	horizontal(904, 14, ' ');
	vertical(89, 2, '-');
	board[82] = ' ';
	board[123] = ' ';
	board[130] = ' ';
	board[136] = ' ';
	board[171] = ' ';
	board[178] = ' ';
	board[183] = ' ';
	board[219] = ' ';
	board[222] = ' ';
	board[226] = ' ';
	board[235] = ' ';
	board[267] = ' ';
	board[274] = ' ';
	board[315] = ' ';
	board[320] = ' ';
	board[334] = ' ';
	board[363] = ' ';
	board[367] = ' ';
	board[372] = ' ';
	board[382] = ' ';
	board[411] = ' ';
	board[420] = ' ';
	board[434] = ' ';
	board[459] = ' ';
	board[481] = ' ';
	board[486] = ' ';
	board[507] = ' ';
	board[519] = ' ';
	board[534] = ' ';
	board[555] = ' ';
	board[582] = ' ';
	board[618] = ' ';
	board[654] = ' ';
	board[719] = ' ';
	board[820] = ' ';
	board[869] = ' ';
	board[873] = ' ';
	board[921] = ' ';
	board[925] = ' ';
	board[1024] = ' ';
	board[1072] = ' ';
	board[1120] = ' ';
	board[1168] = ' ';
	board[1165] = ' ';
	board[1216] = ' ';
	board[1264] = ' ';
	board[1312] = ' ';
	board[1360] = ' ';
	board[1408] = ' ';
	board[1456] = ' ';	
	board[1504] = ' ';
	board[1603] = ' ';
	board[1903] = '-';
	board[88] = '+';
	board[138] = '?';
	board[1970] = 's';
	action[0] = knight;
	data[0] = 0;
	action[1] = nullptr;
	return 1920;
}

uint16_t powerGrip(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 15;
	height = 15;
	edgeWalls();
	horizontal(69, 4, '-');
	horizontal(115, 4, '-');
	horizontal(159, 4, '-');
	vertical(22, 9, '-');
	vertical(24, 3, '-');
	horizontal(46, 5, 'B');
	horizontal(61, 5, 'B');
	horizontal(99, 2, 'B');
	horizontal(130, 4, 'B');
	vertical(21, 8, 'B');
	board[143] = '-';
	board[84] = 'B';
	board[86] = 'B';
	board[128] = 'B';
	board[42] = '+';
	board[15] = 'a';
	action[0] = nullptr;
	return 202;
}

uint16_t finalRoom(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 80;
	height = 40;
	edgeWalls();
	horizontal(401, 78, '-');
	horizontal(481, 78, '-');
	horizontal(965, 74, '-');
	horizontal(1045, 74, '-');
	horizontal(1521, 77, '-');
	horizontal(1601, 78, '-');
	horizontal(2482, 77, '-');
	horizontal(2561, 78, '-');
	horizontal(1841, 78, 'B');
	horizontal(1921, 78, 'B');
	vertical(122, 5, '-');
	rightDiag(2840, 4, 'B');
	rightDiag(2920, 3, '-');
	rightDiag(3000, 2, '!');
	leftDiag(2919, 3, 'B');
	leftDiag(2999, 2, '-');
	board[3079] = '!';
	board[696] = 'B';
	board[1598] = 'B';
	board[2481] = 'B';
	board[754] = '!';
	board[781] = '!';
	board[1242] = '!';
	board[1295] = '!';
	board[321] = '?';
	board[40] = 'w';
	action[0] = change;
	data[0] = 0;
	action[1] = chase;
	data[1] = 1242;
	action[2] = chase;
	data[2] = 754;
	action[3] = copy;
	data[3] = 781;
	action[4] = copy;
	data[4] = 1295;
	action[5] = killHor;
	data[5] = 0x80000000;
	action[6] = killHor;
	data[6] = 77;
	action[7] = killVert;
	data[7] = 0x80000000;
	action[8] = killVert;
	data[8] = 9;
	action[9] = nullptr;
	board[3080] = 'o';
	return 120;
}

uint16_t room(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 15;
	height = 15;
	edgeWalls();
	board[7] = 'w';
	board[105] = 'a';
	board[217] = 's';
	board[119] = 'd';
	action[0] = nullptr;
	switch(c) {
		case 'w':
		case 'W':
			return 202;
		case 'a':
		case 'A':
			return 118;
		case 's':
		case 'S':
			return 22;
		default:
			return 106;
	}
}

uint16_t left(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	board[105] = '-';
	return pos;
}

uint16_t right(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	board[119] = '-';
	return pos;
}

uint16_t top(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	board[7] = '-';
	return pos;
}

uint16_t bottom(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	board[217] = '-';
	return pos;
}

uint16_t topLeft(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = top(c, action, data);
	board[105] = '-';
	return pos;
}

uint16_t topRight(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = top(c, action, data);
	board[119] = '-';
	return pos;
}

uint16_t bottomLeft(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = bottom(c, action, data);
	board[105] = '-';
	return pos;
}

uint16_t bottomRight(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = bottom(c, action, data);
	board[119] = '-';
	return pos;
}

uint16_t cheese(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	if ((flags & 0x10) == 0) {
		board[112] = 'c';
	}
	return pos;
}

uint16_t down(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = topLeft(c, action, data);
	board[119] = '-';
	return pos;
}

uint16_t up(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = bottomRight(c, action, data);
	board[105] = 'A';
	return pos;
}

uint16_t checkers(char c, void (** action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = left(c, action, data);
	board[119] = '-';
	for (uint8_t y = 16; y < 200; y += 15) {
		uint8_t x = y;
		if ((x & 1) == 0) {
			x++;
		}
		while (x < y + 13) {
			board[x] = 'B';
			x += 2;
		}
	}
	return pos;
}

uint16_t warpy(char c, void (** action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 11;
	height = 15;
	edgeWalls();
	horizontal(25, 7, '-');
	horizontal(48, 3, '-');
	horizontal(61, 4, '-');
	horizontal(105, 4, '-');
	horizontal(126, 4, '-');
	vertical(13, 12, '-');
	vertical(59, 8, '-');
	vertical(83, 2, '-');
	board[1] = 'w';
	board[5] = 'W';
	board[159] = 's';
	board[41] = '*';
	board[85] = '*';
	action[0] = nullptr;
	data[1] = 41;
	data[2] = 85;
	if (c == 'w') {
		return 148;
	}
	if (c == 's') {
		return 12;
	}
	return 16;
}


// Empty room with doors at the top and bottom.
uint16_t vert(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = left(c, action, data);
	board[119] = '-';
	return pos;
}


// Room between checkers and tele.
uint16_t postWarp(char c, void (** action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = vert(c, action, data);
	board[218] = 'S';
	if (c == 'W') {
		pos++;
	}
	return pos;
}


uint16_t toKnight(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = room(c, action, data);
	board[134] = 'D';
	return pos;
}

uint16_t warpPoint(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = topRight(c, action, data);
	board[217] = '-';
	board[112] = '+';
	return pos;
}

uint16_t wallOdeath(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 80;
	height = 40;
	edgeWalls();
	board[3160] = 's';
	horizontal(994, 14, 'B');
	horizontal(1440, 4, '-');
	horizontal(1765, 74, '-');
	vertical(108, 12, '-');
	vertical(132, 12, '-');
	vertical(1444, 5, '-');
	vertical(2918, 3, '-');
	vertical(2919, 3, '-');
	vertical(2921, 3, '-');
	vertical(2922, 3, '-');
	board[1602] = '*';
	board[1677] = '*';
	data[1] = 1602;
	data[2] = 1677;
	if (c == 'a') {
		action[0] = destroy;
		action[1] = nullptr;
		data[0] = 3121;
		return 158;
	}
	action[0] = nullptr;
	return 3080;
}


// This room has two enemies which copy your movements.
uint16_t copyCats(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 18;
	height = 12;
	edgeWalls();
	horizontal(38, 15, '-');
	horizontal(56, 15, '-');
	horizontal(91, 14, '-');
	horizontal(109, 14, '-');
	board[16] = 'w';
	board[126] = 'a';
	board[206] = 's';
	board[134] = '!';
	action[0] = copy;
	data[0] = 134;
	if (c == 's') {
		action[1] = nullptr;
		return 34;
	}
	board[34] = '!';
	action[1] = copy;
	data[1] = 34;
	action[2] = nullptr;
	if (c == 'w') {
		return 188;
	}
	return 127;
}


// This room is below the labyrinth room and has an optional puzzle.
uint16_t underLab(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 10;
	height = 13;
	edgeWalls();
	board[23] = 'B';
	board[74] = 'B';
	board[76] = 'B';
	board[5] = 'w';
	board[121] = 's';
	board[128] = 'S';
	horizontal(63, 2, '-');
	horizontal(66, 3, '-');
	horizontal(84, 3, '-');
	horizontal(107, 2, 'B');
	horizontal(33, 2, 'B');
	horizontal(36, 3, 'B');
	horizontal(44, 5, 'B');
	vertical(22, 10, '-');
	action[0] = nullptr;
	if (c == 'w') {
		return 111;
	}
	if (c == 'W') {
		return 118;
	}
	return 15;
}


// This room comes right before the room where you get the sticky.
uint16_t toSticky(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 17;
	height = 17;
	edgeWalls();
	board[95] = '-';
	board[8] = 'w';
	board[136] = 'a';
	board[280] = 's';
	horizontal(18, 7, 'B');
	horizontal(26, 7, 'B');
	action[0] = nullptr;
	switch(c) {
		case 'w':
			return 263;
		case 's':
			return 25;
		default:
			return 137;
	}
}


// This room is near the bottom of the map and has w, W, and d doors.
uint16_t corner(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	uint16_t pos = bottomLeft(c, action, data);
	if (c == 'S') {
		pos++;
	}
	board[8] = 'W';
	return pos;
}


// This room requires you to shove a block through kill thingies.
uint16_t shield(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 9;
	height = 15;
	edgeWalls();
	board[24] = 'B';
	board[113] = 'B';
	board[4] = 'w';
	board[130] = 's';
	for(uint16_t i = 37; i < 44; i++) {
		vertical(i, 7, '!');
	}
	if (c == 'w') {
		return 121;
	}
	return 13;
}


// This room looks like a penguin.
uint16_t logo(char c, void (**action)(uint32_t*, uint16_t), uint32_t *data) {
	width = 45;
	height = 31;	
	edgeWalls();
	board[514] = 'B';
	board[608] = 'B';
	board[748] = 'B';
	board[779] = 'B';
	board[963] = 'B';
	board[973] = 'B';
	board[1001] = 'B';
	board[1090] = 'B';
	board[1113] = 'B';
	board[1144] = 'B';
	board[1149] = 'B';
	board[21] = 'w';
	board[1371] = 's';
	horizontal(289, 4, 'B');
	horizontal(333, 6, 'B');
	horizontal(378, 7, 'B');
	horizontal(425, 2, 'B');
	horizontal(428, 2, 'B');
	horizontal(470, 2, 'B');
	horizontal(650, 3, 'B');
	horizontal(1009, 2, 'B');
	horizontal(1100, 4, 'B');
	horizontal(1136, 3, 'B');
	horizontal(1155, 3, 'B');
	horizontal(1183, 6, 'B');
	horizontal(1195, 6, 'B');
	vertical(423, 4, 'B');
	vertical(778, 4, 'B');
	vertical(794, 4, 'B');
	vertical(822, 3, 'B');
	vertical(840, 3, 'B');
	vertical(841, 4, 'B');
	vertical(887, 4, 'B');
	leftDiag(557, 5, 'B');
	leftDiag(646, 3, 'B');
	leftDiag(956, 3, 'B');
	leftDiag(972, 5, 'B');
	rightDiag(474, 7, 'B');
	rightDiag(518, 2, 'B');
	rightDiag(519, 7, 'B');
	rightDiag(558, 2, 'B');
	rightDiag(914, 2, 'B');
	rightDiag(915, 6, 'B');
	rightDiag(916, 3, 'B');
	rightDiag(976, 2, 'B');
	rightDiag(977, 3, 'B');
	action[0] = nullptr;
	if (c == 'w') {
		return 1326;
	}
	return 66;
}


int main() {
	const uint8_t mapWidth = 11;
	const uint8_t start = 90;
	const uint8_t cage = 78;
	const uint8_t warpRoom = 24;
	const uint8_t knight = 98;
	const uint8_t grab = 45;
	uint32_t moves = 0;
	printf("\n\n\nWelcome to puzzle-land.\nYour objective is to find a circular item (It looks like the letter 'o').\nIt shouldn't be far from your starting location.\nwasd - move\nt - close\nx - reset room\nAny other key - wait\nPress enter to continue.\n");
	flags = 0;
	input = getchar();
	if (input == 'C') {
		flags = 14;
	}
	uint16_t (*initialize[121])(char, void (**)(uint32_t*, uint16_t), uint32_t*) =  {	nullptr,	nullptr,	nullptr,	topLeft,	top,	top,	top,	top,	top,		topRight,		nullptr,
																						nullptr,	nullptr,	nullptr,	vert,		left,	room,	room,	room,	room,		right,			nullptr,
																						nullptr,	wallOdeath,	warpPoint,	vert,		left,	room,	room,	room,	room,		right,			nullptr,
																						nullptr,	bottomLeft,	copyCats,	tele,		left,	room,	room,	cheese,	room,		right,			nullptr,
																						down,		powerGrip,	logo,		postWarp,	left,	room,	room,	room,	room,		right,			nullptr,
																						bottomLeft,	toSticky,	mudRoom,	checkers,	left,	room,	room,	room,	room,		right,			nullptr,
																						nullptr,	blocks,		hall,		warpy,		left,	room,	room,	room,	room,		right,			nullptr,
																						nullptr,	prison,		bigRoom,	labyrinth,	room,	room,	room,	room,	room,		right,			nullptr,
																						nullptr,	blockRoom,	frontRoom,	underLab,	left,	room,	room,	room,	room,		right,			knightsMove,
																						nullptr,	nullptr,	finalRoom,	shield,		left,	room,	room,	room,	toKnight,	right,			up,
																						nullptr,	nullptr,	nullptr,	corner,		bottom,	bottom,	bottom,	bottom,	bottom,		bottomRight,	nullptr};
	void (*action[10])(uint32_t*, uint16_t);
	uint32_t data[10];
	char entrance = 'w';
	uint8_t roomNum = start;
	input = ' ';
	uint16_t warp = 0;
	uint16_t position = (initialize[roomNum])(' ', action, data);
	termios oldt = noCanon();
	while (input != 't') {
		uint16_t block = 0;
		if ((flags & 1) == 1) {
			initialize[cage] = secret;
			flags &= 0xFE;
		}
		print();
		input = (char) getchar();
		moves++;
		uint16_t newPosition = position;
		switch (input) {
			case 'w':
				block = position + width;
				newPosition -= width;
				break;
			case 'a':
				block = position + 1;
				newPosition--;
				break;
			case 's':
				block = position - width;
				newPosition += width;
				break;
			case 'd':
				block = position - 1;
				newPosition++;
				break;
			case 'r':
				clear(warp, '@');
				warp = position;
				break;
			case 'f':
				if ((flags & 2) == 2 && warp != 0) {
					newPosition = warp;
				}
				break;
			case 'e':
				if ((flags & 0x28) == 0x28) {
					flags &= 0xDF;
				}
				else if ((flags & 8) == 8) {
					flags |= 0x20;
				}
				break;
			case 'x':
				newPosition = initialize[roomNum](entrance, action, data);
		}
		if ((flags & 4) == 4) {
			moveKnight(position, &newPosition);
		}
		for (uint8_t i = 0; action[i] != nullptr; i++) {
			(action[i])(data + i, position);
		}
		if (board[newPosition] == '-') {
			newPosition = position;
		}
		else if (board[newPosition] == 'B') {
			uint16_t blockPos;
			switch (input) {
				case 'w':
					blockPos = newPosition - width;
					break;
				case 'a':
					blockPos = newPosition - 1;
					break;
				case 's':
					blockPos = newPosition + width;
					break;
				case 'd':
					blockPos = newPosition + 1;
					break;
				default:
					blockPos = newPosition;
					newPosition = position;
			}
			if (board[blockPos] == '-' || board[blockPos] == 'B' || board[blockPos] == 'w' || board[blockPos] == 'a'|| board[blockPos] == 's'|| board[blockPos] == 'd' || board[blockPos] == 'W' || board[blockPos] == 'A'|| board[blockPos] == 'S'|| board[blockPos] == 'D') {
				newPosition = position;
			}
			else {
				board[blockPos] = 'B';
			}
		}
		switch (board[newPosition]) {
			case 'w':
				roomNum -= mapWidth;
				newPosition = (initialize[roomNum])('w', action, data);
				entrance = 'w';
				warp = 0;
				break;
			case 'W':
				roomNum -= 2 * mapWidth;
				newPosition = (initialize[roomNum])('W', action, data);
				entrance = 'W';
				warp = 0;
				break;
			case 'a':
				newPosition = (initialize[--roomNum])('a', action, data);
				entrance = 'a';
				warp = 0;
				break;									
			case 'A':
				roomNum -= 2;
				newPosition = (initialize[roomNum])('A', action, data);
				entrance = 'A';
				warp = 0;
				break;									
			case 's':
				roomNum += mapWidth;
				newPosition = (initialize[roomNum])('s', action, data);
				entrance = 's';
				warp = 0;
				break;
			case 'S':
				roomNum += 2 * mapWidth;
				newPosition = (initialize[roomNum])('S', action, data);
				entrance = 'S';
				warp = 0;
				break;
			case 'd':
				newPosition = (initialize[++roomNum])('d', action, data);
				entrance = 'd';
				warp = 0;
				break;
			case 'D':
				roomNum += 2;
				newPosition = (initialize[roomNum])('D', action, data);
				entrance = 'D';
				warp = 0;
				break;									
			case '!':
				tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
				print();
				printf("\n\nGame over.\n\n");
				return 1;				
			case '?':
				data[0] |= 0x80000000;
				break;
			case '*':
				if (data[1] == newPosition) {
					newPosition = data[2];
				}
				else {
					newPosition = data[1];
				}
				switch (input) {
					case 'w':
						newPosition -=  width;
						break;
					case 'a':
						newPosition--;
						break;
					case 's':
						newPosition += width;
						break;
					case 'd':
						newPosition++;
						break;
					default:
						newPosition = position;
				}
				break;
			case '+':
				if (roomNum == warpRoom) {
					flags |= 2;
					warp = 0;
					flags &= 0xDF;
					printf("You found the warp point! Good for you!\nPress 'r' and 'f' to use it.\nPress any key to continue.\n");
					getchar();
				}
				else if (roomNum == knight) {
					flags |= 4;
					clear(warp, '@');
					warp = 0;
					flags &= 0xDF;
					printf("You found the knight's move! Nice.\nUse it with y, u, i, o, h, j, k, and l.\nPress any key to continue.\n");
					getchar();
				}
				else {
					flags |= 8;
					flags &= 0xDF;
					clear(warp, '@');
					warp = 0;
					printf("You found the sticky.\nPress e to use it.\nPress any key to continue.\n");
					getchar();
				}
				break;
			case 'c':
				flags |= 0x10;
				printf("You found some moldy cream cheese.\nMaybe if you cut the moldy parts off it might still be useful for something.\nPress any key to continue.\n");
				getchar();
				break;
			case 'o':
				tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
				print();
				printf("\n\nCongradulations!!! You found your bagel!\nUnfortunately, it's stale. :(\n");
				if ((flags & 0x10) == 0x10) {
					printf("With a bit of cream cheese, though, it isn't too bad.\n");
				}
				printf("Well, you won. I hope you had fun.\n\nMoves taken: %u\n\n", moves);
				return 2;	
		}
		if ((flags & 0x20) == 0x20 && position != newPosition && board[block] == 'B') {
			board[block] = ' ';
			board[position] = 'B';
		}
		else {
			clear(position, 'X');
		}
		position = newPosition;
		if ((flags & 2) == 2 && board[warp] == ' ') {
			board[warp] = '@';
		}
		board[position] = 'X';
	}
	printf("\n\nExiting...\n\n");
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return 0;
}