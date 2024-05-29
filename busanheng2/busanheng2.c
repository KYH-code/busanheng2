#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define LEN_MIN 15
#define LEN_MAX 50
#define STM_MIN 0
#define STM_MAX 5
#define PROB_MIN 10
#define PROB_MAX 90
#define AGGRO_MIN 0
#define AGGRO_MAX 5

#define MOVE_LEFT 1
#define MOVE_STAY 0

#define ATK_NONE 0
#define ATK_CITIZEN 1
#define ATK_DONGSEOK 2

#define ACTION_REST 0
#define ACTION_PROVOKE 1
#define ACTION_PULL 2

int train_length,
    percentile_probability,
	turn = 1;

int value;

bool check;

int citizen_random, 
	zombie_random,
	madongseok_direction;

int citizen[10] = {
	0, //시민 위치 값 저장할 배열칸
	0, //시민 이동 전 위치 값 저장할 배열칸
	1, //시민 어그로 값 저장할 배열칸
	1, //시민 어그로 변경 전 값 저장할 배열칸
};

int zombie[10] = {
	0, //좀비 위치 값 저장할 배열칸
	0, //좀비 이동 전 위치 값 저장할 배열칸
};

int madongseok[10] = {
	0, //마동석 위치 값 저장할 배열칸
	0, //마동석 이동 전 위치 값 저장할 배열칸
	1, //마동석 어그로 값 저장할 배열칸
	1, //마동석 어그로 변경 전 값 저장할 배열칸
	0, //마동석 스테미나 저장할 배열칸
	0, //마동석 이전 스테미나 저장할 배열칸
	0, //마동석 행동 값 저장할 배열칸
};

//함수 Prototype 영역
int value_input(const char* message, int DEFINE_MIN, int DEFINE_MAX);
bool value_check(int value, int DEFINE_MIN, int DEFINE_MAX);
void print_train();
void citizen_move();
void zombie_move();
void madongseok_move();
int state_aggro(int unit, int before_unit, int aggro);
void citizen_state();
void zombie_state();
void madongseok_state();
void citizen_action();

int value_input(const char* message, int DEFINE_MIN, int DEFINE_MAX) {

	do {
		printf("%s(%d~%d)>> ", message, DEFINE_MIN, DEFINE_MAX);
		scanf_s("%d", &value);

		check = !value_check(value, DEFINE_MIN, DEFINE_MAX);
	} while (check);

	return value;
}

bool value_check(int value, int DEFINE_MIN, int DEFINE_MAX) {
	return (value >= DEFINE_MIN && value <= DEFINE_MAX);
}

void print_train() {
	printf("\n");

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < train_length; j++) {
			if (i == 0 || i == 2) { printf("#"); }
			else {
				if (j == 0 || j == train_length - 1) { printf("#"); }
				else if (j == citizen[0]) { printf("C"); }
				else if (j == zombie[0]) { printf("Z"); }
				else if (j == madongseok[0]) { printf("M"); }
				else { printf(" "); }
			}
		}
		printf("\n");
	}
	printf("\n");
}

void citizen_move() {
	citizen[1] = citizen[0];
	citizen_random = rand() % 101;
	citizen[0] = (100 - percentile_probability >= citizen_random) ? (citizen[0] - 1) : citizen[0];
	citizen[3] = citizen[2];
	citizen[2] = state_aggro(citizen[0], citizen[1], citizen[2]);
}

void zombie_move() {
	zombie[1] = zombie[0];
	zombie_random = rand() % 101;
	zombie[0] = (turn % 2 != 0) ? ((zombie_random <= percentile_probability) ? (zombie[0] - 1) : zombie[0]) : zombie[0];
}

void madongseok_move() {
	madongseok[1] = madongseok[0];
	do {
		if (madongseok[0] == zombie[0] + 1) {
			printf("madongseok move(%d:stay)>> ", MOVE_STAY);
			scanf_s("%d", &madongseok_direction);
			check = !value_check(madongseok_direction, MOVE_STAY, MOVE_STAY);
		}
		else {
			printf("madongseok move(%d:stay, %d:left)>> ", MOVE_STAY, MOVE_LEFT);
			scanf_s("%d", &madongseok_direction);
			check = !value_check(madongseok_direction, MOVE_STAY, MOVE_LEFT);
		}

	} while (check);

	if (madongseok_direction == 0) { madongseok[0]; }
	else { madongseok[0] = madongseok[0] - 1; }

	madongseok[3] = madongseok[2];
	madongseok[2] = state_aggro(madongseok[0], madongseok[1], madongseok[2]);
}

int state_aggro(int unit, int before_unit, int aggro) {
	if (aggro > AGGRO_MIN && aggro < AGGRO_MAX) {
		if (unit != before_unit) { aggro++; }
		else { aggro--; }
	}

	return aggro;
}

void citizen_state() {
	if (citizen[0] == citizen[1]) {
		printf("citizen: stay %d (aggro: %d -> %d)\n", citizen[0], citizen[3], citizen[2]);
	}
	else {
		printf("citizen: %d -> %d (aggro: %d -> %d)\n", citizen[1], citizen[0], citizen[3], citizen[2]);
	}
}

void zombie_state() {

	if (turn % 2 != 0) {

		if (zombie[0] == zombie[1]) {
			printf("zombie: stay %d\n", zombie[0]);
		}
		else {
			printf("zombie: %d -> %d\n", zombie[1], zombie[0]);
		}
	}
	else {
		printf("zombie: stay %d (cannot move)\n", zombie[0]);
	}
	printf("\n");

}

void madongseok_state() {
	if (madongseok[0] == madongseok[1]) {
		printf("madongseok: stay %d (aggro: %d -> %d, stamina: %d)\n", madongseok[0], madongseok[3], madongseok[2], madongseok[4]);
	}
	else {
		printf("madongseok: %d -> %d (aggro: %d -> %d, stamina: %d)\n", madongseok[1], madongseok[0], madongseok[3], madongseok[2], madongseok[4]);
	}
}

void citizen_action() {
	if (citizen[0] == 1) {
		printf("You WIN!\n");
		exit(0);
	}
	else {
		printf("citizen does nothing.\n");
	}
}

void zombie_action() {

	madongseok[5] = madongseok[4];

	if (citizen[0] + 1 == zombie[0]) {
		printf("GAME OVER! citizen dead...\n");
		exit(0);
	}
	else if (zombie[0] + 1 == madongseok[0]) {
		print("Zombie attacked madongseok (madongseok stamina: %d -> %d)\n");
	}
	else if ((citizen[0] + 1 == zombie[0]) && (zombie[0] + 1 == madongseok[0])) {
		if (citizen[2] <= madongseok[2]) {
			madongseok[4] = madongseok[4] - 1;
			if (madongseok[4] - 1 == STM_MIN) {
				printf("GAME OVER! madongseok dead...\n");
				exit(0);
			}
			else {
				printf("Zombie attacked madongseok (aggro: %d vs. %d, madongseok stamina: %d -> %d)\n", citizen[2], madongseok[2], madongseok[5], madongseok[4]);
			}
		}
	}
	else {
		printf("zombie attacked nobody.\n");
	}
}

void madongseok_action() {

}

int main() {

	srand((unsigned int)time(NULL));

	train_length = value_input("train_length", LEN_MIN, LEN_MAX);
	madongseok[4] = value_input("madongseok stamina", STM_MIN, STM_MAX);
	percentile_probability = value_input("percentile probability 'p'", PROB_MIN, PROB_MAX);

	citizen[0] = train_length - 6;
	zombie[0] = train_length - 3;
	madongseok[0] = train_length - 2;

	print_train();

	while (1) {

		citizen_move();

		zombie_move();
		
		print_train();

		citizen_state();

		zombie_state();

		madongseok_move();

		print_train();

		madongseok_state();

		citizen_action();

		zombie_action();

		madongseok_action();

		turn++;
	}


	return 0;
}