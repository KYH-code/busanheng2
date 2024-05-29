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
	turn = 1,
	value,
	stage = 1,
	escape = 0;

bool check;

int citizen_random, 
	zombie_random,
	madongseok_random,
	madongseok_direction;

int citizen[10] = {
	0, //시민 위치 값 저장할 배열칸
	0, //시민 이동 전 위치 값 저장할 배열칸
	1, //시민 어그로 값 저장할 배열칸
	1, //시민 어그로 변경 전 값 저장할 배열칸
};

int villain[10] = {
	0, //빌런 위치 값 저장할 배열칸
	0, //빌런 이동 전 위치 값 저장할 배열칸
	1, //빌런 어그로 값 저장할 배열칸
	1, //빌런 어그로 변경 전 값 저장할 배열칸
	30, //빌런 발암 확률 저장할 배열칸
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
	0, //마동석 ACTION_PULL 성공 여부 저장 배열칸
};

//함수 Prototype 영역
int value_input(const char* message, int DEFINE_MIN, int DEFINE_MAX);
bool value_check(int value, int DEFINE_MIN, int DEFINE_MAX);
void print_train();
void citizen_move();
void villain_move();
void zombie_move();
void madongseok_move();
int state_aggro(int unit, int before_unit, int aggro);
void citizen_state();
void villain_state();
void zombie_state();
void madongseok_state();
void citizen_action();
void villain_action();
void zombie_action();
void madongseok_action();
void pull_probability();
int stat_management(int stat, int operand, int DEFINE_MIN, int DEFINE_MAX);
void stamina_check();

void stage_one();
void stage_two();
void stage_three();
void stage_four();

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
				else if (j == villain[0] && stage == 2) { printf("V"); }
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

void villain_move() {
	villain[1] = villain[0];
	villain[0] = (citizen[0] != citizen[1]) ? (villain[0] - 1) : villain[0];
	villain[3] = villain[2];
	villain[2] = state_aggro(villain[0], villain[1], villain[2]);
}

void zombie_move() {
	zombie[1] = zombie[0];
	zombie_random = rand() % 101;
	zombie[0] = (madongseok[7] == 0 && turn % 2 != 0) ? ((zombie_random <= percentile_probability) ? (citizen[2] >= madongseok[2]) ? (citizen[0] != zombie[0] - 1) ? zombie[0] - 1 : zombie[0] : (zombie[0] + 1 != madongseok[0]) ? zombie[0] + 1 : zombie[0] : zombie[0]) : zombie[0];
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

void villain_state() {
	if (villain[0] == villain[1]) {
		printf("villain: stay %d (aggro: %d -> %d)\n", villain[0], villain[3], villain[2]);
	}
	else {
		printf("villain: %d -> %d (aggro: %d -> %d)\n", villain[1], villain[0], villain[3], villain[2]);
	}
}

void zombie_state() {

	if (turn % 2 != 0) {
		if (madongseok[7] == 1) {
			printf("zombie: stay %d (Caught by madongseok)\n", zombie[0]);
		}
		else {
			if (zombie[0] == zombie[1]) {
				printf("zombie: stay %d\n", zombie[0]);
			}
			else {
				printf("zombie: %d -> %d\n", zombie[1], zombie[0]);
			}
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
	printf("\n");
}

void citizen_action() {
	if (citizen[0] == 1) {
		printf("You WIN!\n");
		escape = 1;
	}
	else {
		printf("citizen does nothing.\n");
	}
}

void villain_action() {}

void zombie_action() {

	madongseok[5] = madongseok[4];

	if (citizen[0] + 1 == zombie[0]) {
		printf("GAME OVER! citizen dead...\n");
		exit(0);
	}
	else if (zombie[0] + 1 == madongseok[0]) {
		madongseok[4] = madongseok[4] - 1;
		printf("Zombie attacked madongseok (madongseok stamina: %d -> %d)\n", madongseok[5], madongseok[4]);
	}
	else if ((citizen[0] + 1 == zombie[0]) && (zombie[0] + 1 == madongseok[0])) {
		if (citizen[2] <= madongseok[2]) {
			madongseok[4] = madongseok[4] - 1;
			printf("Zombie attacked madongseok (aggro: %d vs. %d, madongseok stamina: %d -> %d)\n", citizen[2], madongseok[2], madongseok[5], madongseok[4]);
		}
		else {
			printf("GAME OVER! citizen dead...\n");
			exit(0);
		}
	}
	else {
		printf("zombie attacked nobody.\n");
	}

	stamina_check();
}

void madongseok_action() {
	if(madongseok[0] - 1 != zombie[0]) {
		do {
			printf("madongseok action(%d.rest, %d.provoke)>> ", ACTION_REST, ACTION_PROVOKE);
			scanf_s("%d", &madongseok[6]);
			check = !value_check(madongseok[6], ACTION_REST, ACTION_PROVOKE);
		} while (check);
	}
	else {
		do {
			printf("madongseok action(%d.rest, %d.provoke, %d.pull)>> ", ACTION_REST, ACTION_PROVOKE, ACTION_PULL);
			scanf_s("%d", &madongseok[6]);
			check = !value_check(madongseok[6], ACTION_REST, ACTION_PULL);
		} while (check);
	} printf("\n");

	madongseok[3] = madongseok[2];
	madongseok[5] = madongseok[4];

	switch (madongseok[6]) {
		case 0:
			printf("madongseok rests...\n");
			madongseok[2] = stat_management(madongseok[2], -1, AGGRO_MIN, AGGRO_MAX);
			madongseok[4] = stat_management(madongseok[4], 1, STM_MIN, STM_MAX);
			printf("madongseok: %d (aggro: %d -> %d, stamina: %d -> %d)\n", madongseok[0], madongseok[3], madongseok[2], madongseok[5], madongseok[4]);
			break;

		case 1:
			printf("madongseok provoked zombie...\n");
			madongseok[2] = AGGRO_MAX;
			printf("madongseok: %d (aggro: %d -> %d, stamina: %d)\n", madongseok[0], madongseok[3], madongseok[2], madongseok[4]);
			break;

		case 2:
			pull_probability();
			break;

		default:
			break;
	}
	
	stamina_check();

}

void pull_probability() {
	madongseok_random = rand() % 101;
	madongseok[2] = stat_management(madongseok[2], 2, AGGRO_MIN, AGGRO_MAX);
	madongseok[4] = stat_management(madongseok[4], -1, STM_MIN, STM_MAX);

	if (100 - percentile_probability >= madongseok_random) { 
		madongseok[7] = 1;
		printf("madongseok pulled zombie... Next turn, it can't move\n");
	}
	else { 
		madongseok[7] = 0;
		printf("madongseok failed to pull zombie\n");
	}

	printf("madongseok: %d (aggro: %d -> %d, stamina: %d -> %d)\n", madongseok[0], madongseok[3], madongseok[2], madongseok[5], madongseok[4]);
}

int stat_management(int stat, int operand, int DEFINE_MIN, int DEFINE_MAX) {

	if (stat + operand >= DEFINE_MIN && stat + operand <= DEFINE_MAX) {
		stat = stat + operand;
	}
	else if (stat + operand > DEFINE_MAX) {
		stat = DEFINE_MAX;
	}
	else if (stat + operand < DEFINE_MIN) {
		stat = DEFINE_MIN;
	}

	return stat;
}

void stamina_check() {
	if (madongseok[4] == 0) {
		printf("GAME OVER! madongseok dead...\n");
		exit(0);
	}
}

void stage_one() {
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

		if (escape == 1) { return; }

		zombie_action();

		madongseok_action();

		turn++;
	}
}

void stage_two() {
	stage = 2;
	escape = 0;

	citizen[0] = train_length - 6;
	villain[0] = train_length - 5;
	zombie[0] = train_length - 3;
	madongseok[0] = train_length - 2;

	print_train();

	while (1) {

		citizen_move();

		villain_move();

		zombie_move();

		print_train();

		citizen_state();

		villain_state();

		zombie_state();

		madongseok_move();

		print_train();

		madongseok_state();

		citizen_action();

		if (escape == 1) { return; }

		villain_action();

		zombie_action();

		madongseok_action();

		turn++;

	}
}

void stage_three() {}
void stage_four() {}

int main() {

	srand((unsigned int)time(NULL));

	train_length = value_input("train_length", LEN_MIN, LEN_MAX);
	madongseok[4] = value_input("madongseok stamina", STM_MIN, STM_MAX);
	percentile_probability = value_input("percentile probability 'p'", PROB_MIN, PROB_MAX);

	stage_one();
	stage_two();
	stage_three();
	stage_four();

	/*if (escape == 1) { stage_two(); }
	if (escape == 1) { stage_three(); }
	if (escape == 1) { stage_four(); }*/

	return 0;
}