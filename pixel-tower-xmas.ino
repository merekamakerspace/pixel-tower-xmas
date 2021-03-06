#include "FastLED.h"
#include "digits.h"
#include "track.h"


// How many leds in your strip?
#define NUM_LEDS_1 150
#define NUM_LEDS_2 98

#define NUM_LEDS NUM_LEDS_1 + NUM_LEDS_2

#define LEDS_PER_ROW 8
#define NUM_ROWS 31


// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN_1 7
#define DATA_PIN_2 8

#define BIKE1 A3
#define BIKE2 A0

#define PLAYER_ONE_START 0
#define PLAYER_TWO_START 62

#define NUM_STEPS 62

#define PLAYER_ONE_FINISH 61
#define PLAYER_TWO_FINISH 123


//#define PLAYER_START_LEN 2


enum {RED_PLAYER, BLUE_PLAYER};

enum {WAITING, COUNT_DOWN, START_GAME, PLAYING, WIN, LOSE, END_GAME };

int state = WAITING;


CRGB leds[NUM_LEDS];

CRGB player_colour[] = {CRGB::Blue, CRGB::Red};

int player_start[] = {PLAYER_ONE_START, PLAYER_TWO_START};
int player_end[] = {PLAYER_ONE_START, PLAYER_TWO_START};
int player_finish[] = {PLAYER_ONE_FINISH, PLAYER_TWO_FINISH};


unsigned long last_twinkle = millis();
unsigned long last_move = millis();

unsigned char hue = 0;
unsigned int delay_time = 100;
unsigned char fade_rate = 100;
unsigned long last_grow = millis();

int player_energy[] = {0, 0};
int player_adc[] = {BIKE1, BIKE2};

int max_energy[] = {110, 110};




void fadeAll()  {
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i].nscale8(fade_rate);

	}
	FastLED.show();
}

void light_row(int row, CRGB colour) {

	if (row > NUM_ROWS) {
		return;
	}

	for (int i = 0; i <  LEDS_PER_ROW; i++) {
		int pixel = (LEDS_PER_ROW * row) + i;
		leds[NUM_LEDS - pixel - 1] = colour;
	}

}


void show_digit(int digit, int start_row) {
	//FastLED.clear();

	for (int row = 0; row < 8; row++) {
		if (start_row + row >= 0 ) {
			byte line = DIGITS[digit][row];

			//Serial.println(line);
			int col = 0;
			for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
				//int pixel = ((start_row + row) * 8) +  col;
				int pixel  = ((start_row + row) * 8) + (7 - col);

				if ((start_row + row) % 2) {
					//pixel  = ((start_row + row) * 8) + (7 - col);
					pixel = ((start_row + row) * 8) +  col;//pixel += 1;
				}

				if (line & mask) {
					leds[pixel] = CRGB::Red;
					//Serial.print("*");
				} else {
					//Serial.print(" ");
					leds[pixel] = CRGB::Black;
				}
				//Serial.println(pixel);

				col++;
			}
		}
		//Serial.println();
	}
	FastLED.show();
}


void drawPlayer(int pid) {

	// Serial.print(pid);
	// Serial.print(" ");
	// Serial.print(player_start[pid]);
	// Serial.print(" ");
	// Serial.print(player_end[pid]);
	// Serial.print(" ");
	// Serial.println(TRACK[player_end[pid]]);


	for (int i = player_start[pid]; i <= player_end[pid]; i++) {
		// Serial.print(pid);
		// Serial.print(" ");
		// Serial.println(TRACK[i]);
		leds[TRACK[i]] = player_colour[pid];
	}


}


void movePlayer(int player_id) {
	//Increment player token
	//player_start[player_id]++;
	player_end[player_id]++;

	// if (player_end[player_id] >= player_finish[player_id] ) {
	// 	player_end[player_id] = player_finish[player_id];
	// }

	last_move = millis();
	Serial.print(last_move);
	Serial.print(" ");
	Serial.print(player_id);
	Serial.print(" ");
	Serial.print(player_end[player_id]);
	Serial.print(" ");
	Serial.println(TRACK[player_end[player_id]]);


}



void processSerial() {
	while (Serial.available()) {
		char inChar = (char) Serial.read();
		unsigned int val;
		switch (inChar) {
		case 's':
			val = (unsigned int) Serial.parseInt();

			Serial.print("s");
			if (val > 1) {
				delay_time = val;
				Serial.print("delay: ");
				Serial.println(val);
			}

			// get the new byte:
			break;
		case 'f':
			Serial.print("f");
			val = (unsigned int) Serial.parseInt();

			if (val < 256 && val > 1) {
				fade_rate = val;
				Serial.print("fade: ");
				Serial.println(val);
			}

			break;
		case 'd':
			FastLED.clear();
			val = (unsigned int) Serial.parseInt();

			if (val == 10) {
				Serial.println(0);
				FastLED.clear();
				show_digit(0, 8);
			}

			if (val > 0 && val < 10) {
				Serial.println(val);
				show_digit(val, 8);
			}

			break;

		case 'x':
			state = COUNT_DOWN;
			break;
		case 'q':
			movePlayer(RED_PLAYER);
			drawPlayer(BLUE_PLAYER);
			drawPlayer(RED_PLAYER);
			FastLED.show();

			break;
		case 'p':
			movePlayer(BLUE_PLAYER);
			drawPlayer(RED_PLAYER);
			drawPlayer(BLUE_PLAYER);
			FastLED.show();

			break;

		}


	}

}


void serialEvent() {
	processSerial();

}

void readADC(int pid) {

	int val = analogRead(player_adc[pid]) / 6;
	if (val > 30) {
		player_energy[pid] += val;

		Serial.print(pid);
		Serial.print(" ");
		Serial.print(val);
		Serial.print(" ");
		Serial.println(player_energy[pid]);

	}

	if (player_energy[pid] > max_energy[pid]) {
		Serial.print(pid);
		Serial.print(" moved ");
		Serial.println(player_end[pid]);
		player_energy[pid] = 0;
		movePlayer(pid);
		//drawPlayer(pid);

	}

}


void testScreen() {
	for (int i = 0; i < NUM_ROWS; i++) {
		hue = map(i, 0, NUM_ROWS, 0, 160);
		light_row(i, CHSV(hue, 255, 255));

	}
	FastLED.show();

	//unsigned long start_time = millis();
	while (leds[NUM_LEDS - 3].r > 0 ) {
		fadeAll();
		delay(30);
	}

}


void drawPixel(int x, int y, CRGB color) {

	int pixel = ((LEDS_PER_ROW) * (y) ) + x;

	if (y % 2) {
		pixel  = ((LEDS_PER_ROW) * (y) ) + (LEDS_PER_ROW - 1 - x);
		//pixel += 1;
	}

	leds[pixel] = color;

}






void resetGame() {
	//Reset player positions
	player_start[0] = PLAYER_ONE_START;
	player_end[0] = PLAYER_ONE_START;

	player_start[1] = PLAYER_TWO_START;
	player_end[1] = PLAYER_TWO_START;

	player_energy[0] = 0;
	player_energy[1] = 0;

	FastLED.clear();
	FastLED.show();
	fade_rate = 100;
	last_grow = millis();
	last_move = millis();
	Serial.println("Reset Game");
}


int check_winner () {
	if (player_end[0] >= player_finish[0] ) {
		Serial.println("Blue Wins");
		return 1;
	}
	if (player_end[1] >= player_finish[1] ) {
		Serial.println("Red Wins");
		return 2;
	}
	return 0;
}


int matrix_row = NUM_ROWS;
int matrix_col = random(LEDS_PER_ROW);

CRGB matrix_color = player_colour[0];

void matrix() {

	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i].nscale8(230);
	}

	if (matrix_row <= 0) {
		matrix_row = NUM_ROWS;
		matrix_col = random(LEDS_PER_ROW);
		if (matrix_color == player_colour[0]) {
			matrix_color  = player_colour[1];
		} else {
			matrix_color = player_colour[0];
		}
		//hue += 128;
	}

	if (millis() - last_twinkle > 300) {
		last_twinkle = millis();
		matrix_row--;
		int pixel = ((LEDS_PER_ROW) * (matrix_row) ) + matrix_col;

		if (matrix_row % 2) {
			pixel  = ((LEDS_PER_ROW) * (matrix_row) ) + (LEDS_PER_ROW - 1 - matrix_col);
			//pixel += 1;
		}
		leds[NUM_LEDS - pixel - 1] =  matrix_color; //CHSV(hue, 255, 255);

		FastLED.show();

	}


}




void setup() {


	Serial.begin(115200);
	Serial.println("ON");

	//FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
	FastLED.addLeds<WS2812, DATA_PIN_1, RGB>(leds, 0, NUM_LEDS_1);
	FastLED.addLeds<WS2812, DATA_PIN_2, RGB>(leds, NUM_LEDS_1, NUM_LEDS_2);

	testScreen();
	FastLED.clear();
	FastLED.show();


}






void colours() {
	hue += 1;
	CRGB col = CHSV(hue, 255, 255);
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = col;
	}
	FastLED.show();
}

void wait(unsigned int dt) {
	unsigned long start_time = millis();
	while (millis() - start_time <  dt) {
		processSerial();
		delay(1);
	}
}

//Show 3 to 1 count_down
void count_down() {
	FastLED.clear();
	// FastLED.show();
	show_digit(3, 8);
	delay(1000);
	show_digit(2, 8);
	delay(1000);
	show_digit(1, 8);
	delay(1000);
	FastLED.clear();
	FastLED.show();


}


int count = 0;


void show_logo() {
	count = random(23) * 8;
	for (int j = 0; j < 8; j++) {
		byte line = M_LOGO[j];

		//Serial.println(line);

		for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask

			if (line & mask) {
				leds[count] = CRGB::Red;
				//Serial.print("*");
			} else {
				//Serial.print(" ");
				leds[count] = CRGB::Black;
			}
			count++;
		}
		///Serial.println();
	}
	FastLED.show();
}



unsigned long setia_delay = millis();
void loop() {


	switch (state) {

	case WAITING:
		fade_rate = 240;
		fadeAll();
		if (millis() - setia_delay > 100) {
			//show_logo();
			matrix();
			//showSetia();
			setia_delay = millis();
		}
		//matrix();
		//noisyFire();
		readADC(RED_PLAYER);
		readADC(BLUE_PLAYER);

		if (player_energy[0] > 5 || player_energy[1] > 5) {
			state = COUNT_DOWN;
		}
		wait(100);
		break;
	case COUNT_DOWN:
		count_down();
		state = START_GAME;
		break;

	case START_GAME:
		resetGame();
		state = PLAYING;
		drawPlayer(RED_PLAYER);
		drawPlayer(BLUE_PLAYER);
		last_move = millis();
		break;

	case PLAYING:
		FastLED.clear();
		readADC(RED_PLAYER);
		readADC(BLUE_PLAYER);

		//FastLED.show();
		//drawTrack();

		drawPlayer(RED_PLAYER);
		drawPlayer(BLUE_PLAYER);
		FastLED.show();
		if (check_winner()) {
			state = WIN;
			break;
		}

		// Timeout game after 30s
		if (millis() - last_move > 30000) {
			Serial.println("Timeout");
			resetGame();
			//Show winner
			state = WAITING;
			fade_rate = 150;
			FastLED.clear();
			//show_logo();
			for (int i = 0 ; i < 20; i++) {
				fadeAll();
				delay(100);
			}

		}

		wait(delay_time);

		break;

	case WIN:
		for (int i = 0 ; i < 10; i++) {
			fadeAll();
			delay(10);
		}
		//RED Wins
		if (check_winner() ==  1) {
			for (int i = 0; i < TRACK_LEN; i++) {
				leds[TRACK[i]] = player_colour[0];
			}

		}
		// Blue Wins
		if (check_winner() ==  2) {
			for (int i = 0; i < TRACK_LEN; i++) {
				leds[TRACK[i]] = player_colour[1];
			}

		}
		FastLED.show();

		for (int i = 0 ; i < 50; i++) {
			fade_rate = 240;
			fadeAll();
			delay(100);
		}

		resetGame();
		//Show winner
		state = WAITING;
		fade_rate = 150;
		FastLED.clear();
		//show_logo();
		for (int i = 0 ; i < 20; i++) {
			fadeAll();
			delay(100);
		}

		break;
	case END_GAME:

		break;
	}


}


// void show_letter(int letter, int start_row) {
// 	//FastLED.clear();

// 	for (int row = 0; row < 8; row++) {
// 		if (start_row + row >= 0 ) {
// 			byte line = SETIA[letter][row];

// 			//Serial.println(line);
// 			int col = 0;
// 			for (byte mask = 00000001; mask > 0; mask <<= 1) { //iterate through bit mask
// 				//int pixel = ((start_row + row) * 8) +  col;
// 				int pixel  = ((start_row + row) * 8) + (7 - col);

// 				if ((start_row + row) % 2) {
// 					//pixel  = ((start_row + row) * 8) + (7 - col);
// 					pixel = ((start_row + row) * 8) +  col;//pixel += 1;
// 				}

// 				if (pixel > 0 && pixel < NUM_LEDS) {
// 					if (line & mask) {
// 						leds[pixel] = CRGB::Blue;
// 						//Serial.print("*");
// 					} else {
// 						//Serial.print(" ");
// 						leds[pixel] = CRGB::Black;
// 					}

// 				}

// 				//Serial.println(pixel);

// 				col++;
// 			}
// 		}
// 		//Serial.println();
// 	}
// 	//FastLED.show();
// }
// int setia_line = 32;

// void showSetia() {
// 	FastLED.clear();
// 	show_letter(4, setia_line + 32);
// 	show_letter(3, setia_line + 24);
// 	show_letter(2, setia_line + 16);
// 	show_letter(1, setia_line + 8);
// 	show_letter(0, setia_line);


// 	FastLED.show();
// 	setia_line--;
// 	if (setia_line < -32) {
// 		setia_line = 32;
// 	}
// }
// //check that doesn't wrap around 0
// if (player_end[pid] > player_start[pid]) {

// 	for (int i = TRACK_LEN - 1; i >= player_end[pid]; i--) {
// 		leds[TRACK[i]] = player_colour[pid];
// 	}

// 	for (int i = 0; i <= player_start[pid]; i++ ) {
// 		leds[TRACK[i]] = player_colour[pid];
// 	}

// }
// void drawTrack() {
// 	for (int i = 0; i < TRACK_LEN; i++) {
// 		leds[TRACK[i]] = CRGB(10, 10, 10);
// 	}
// }
