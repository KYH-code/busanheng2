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

//커스텀 상수
#define MAX_CITIZEN 5

int train_length,
percentile_probability,
turn = 1,
value,
stage = 1,
escape = 0,
temp = 0;

bool check;

int citizen_random,
zombie_random,
madongseok_random,
madongseok_direction,
villain_random,
citizen_count = 0,
random_location,
LOCATION_MAX,
living_citizen;

int citizen[5][5] = {
	{
		0, //시민 위치 값 저장할 배열칸
		0, //시민 이동 전 위치 값 저장할 배열칸
		1, //시민 어그로 값 저장할 배열칸
		1, //시민 어그로 변경 전 값 저장할 배열칸
		1, //시민 생존 여부 저장할 배열칸
	},
	{0, 0, 1, 1, 1},
	{0, 0, 1, 1, 1},
	{0, 0, 1, 1, 1},
	{0, 0, 1, 1, 1},
};

int villain[5] = {
	0, //빌런 위치 값 저장할 배열칸
	0, //빌런 이동 전 위치 값 저장할 배열칸
	1, //빌런 어그로 값 저장할 배열칸
	1, //빌런 어그로 변경 전 값 저장할 배열칸
	30, //빌런 발암 확률 저장할 배열칸
};

int zombie[2] = {
	0, //좀비 위치 값 저장할 배열칸
	0, //좀비 이동 전 위치 값 저장할 배열칸
};

int madongseok[8] = {
	0, //마동석 위치 값 저장할 배열칸
	0, //마동석 이동 전 위치 값 저장할 배열칸
	1, //마동석 어그로 값 저장할 배열칸
	1, //마동석 어그로 변경 전 값 저장할 배열칸
	0, //마동석 스테미나 저장할 배열칸
	0, //마동석 이전 스테미나 저장할 배열칸
	0, //마동석 행동 값 저장할 배열칸
	0, //마동석 ACTION_PULL 성공 여부 저장 배열칸
};

int valid_location[MAX_CITIZEN] = { 0 };

//함수 Prototype 영역
int value_input(const char* message, int DEFINE_MIN, int DEFINE_MAX);
bool value_check(int value, int DEFINE_MIN, int DEFINE_MAX);
void print_train();
void citizen_move(int citizen_num);
void villain_move(int citizen_num);
void zombie_move(int citizen_num);
void madongseok_move();
int state_aggro(int unit, int before_unit, int aggro);
void citizen_state(int citizen_num);
void villain_state();
void zombie_state();
void madongseok_state();
void citizen_action(int citizen_num);
void villain_action(int citizen_num);
void zombie_action(int citizen_num);
void madongseok_action();
void pull_probability();
int stat_management(int stat, int operand, int DEFINE_MIN, int DEFINE_MAX);
void stamina_check();
void citizen_counter();
void citizen_setting();

void stage_one(int citizen_num);
void stage_two(int citizen_num);
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
				else if (j == citizen[0][0]) { printf("C"); }
				else if (j == citizen[1][0] && citizen[1][0] != 0) { printf("C"); }
				else if (j == citizen[2][0] && citizen[2][0] != 0) { printf("C"); }
				else if (j == citizen[3][0] && citizen[3][0] != 0) { printf("C"); }
				else if (j == citizen[4][0] && citizen[4][0] != 0) { printf("C"); }
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

void citizen_move(int citizen_num) {
	citizen[citizen_num][1] = citizen[citizen_num][0];
	citizen_random = rand() % 101;
	citizen[citizen_num][0] = (100 - percentile_probability >= citizen_random) ? (citizen[citizen_num][0] - 1) : citizen[citizen_num][0];
	citizen[citizen_num][3] = citizen[citizen_num][2];
	citizen[citizen_num][2] = state_aggro(citizen[citizen_num][0], citizen[citizen_num][1], citizen[citizen_num][2]);
}

void villain_move(int citizen_num) {
	villain[1] = villain[0];
	villain[0] = (citizen[citizen_num][0] != citizen[citizen_num][1]) ? (villain[0] - 1) : villain[0];
	villain[3] = villain[2];
	villain[2] = state_aggro(villain[0], villain[1], villain[2]);
}

void zombie_move(int citizen_num) {
	zombie[1] = zombie[0];
	zombie_random = rand() % 101;
	zombie[0] = (madongseok[7] == 0 && turn % 2 != 0) ? ((zombie_random <= percentile_probability) ? (citizen[citizen_num][2] >= madongseok[2]) ? (citizen[citizen_num][0] != zombie[0] - 1) ? zombie[0] - 1 : zombie[0] : (zombie[0] + 1 != madongseok[0]) ? zombie[0] + 1 : zombie[0] : zombie[0]) : zombie[0];
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

void citizen_state(int citizen_num) {
	if (citizen[citizen_num][0] == citizen[citizen_num][1]) {
		printf("citizen: stay %d (aggro: %d -> %d)\n", citizen[citizen_num][0], citizen[citizen_num][3], citizen[citizen_num][2]);
	}
	else {
		printf("citizen: %d -> %d (aggro: %d -> %d)\n", citizen[citizen_num][1], citizen[citizen_num][0], citizen[citizen_num][3], citizen[citizen_num][2]);
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

void citizen_action(int citizen_num) {
	if (citizen[citizen_num][0] == 1) {
		printf("You WIN!\n");
		escape = 1;
	}
	else if (citizen[citizen_num][0] + 1 == zombie[0] && (stage == 3 || stage == 4)) {
		printf("citizen%d has been attacked by zombie.\n", citizen_num);
	}
	else {
		if (stage == 3 || stage == 4) {
			printf("citizen%d does nothing.\n", citizen_num);
		}
		else {
			printf("citizen does nothing.\n");
		}
	}
}

void villain_action(int citizen_num) {
	villain_random = rand() % 101;
	if (villain[0] - 1 == citizen[citizen_num][0]) {
		if (villain_random <= villain[4]) {
			temp = villain[0];
			villain[0] = citizen[citizen_num][0];
			citizen[citizen_num][0] = temp;
		}
	}
}

void zombie_action(int citizen_num) {

	madongseok[5] = madongseok[4];

	if (citizen[citizen_num][0] + 1 == zombie[0]) {
		printf("GAME OVER! citizen dead...\n");
		exit(0);
	}
	else if ((stage == 2) && (villain[0] + 1 == zombie[0])) {
		printf("Zombie attacked villain, villain dead...\n");
		stage = 0;
	}
	else if (zombie[0] + 1 == madongseok[0]) {
		madongseok[4] = madongseok[4] - 1;
		printf("Zombie attacked madongseok (madongseok stamina: %d -> %d)\n", madongseok[5], madongseok[4]);
	}
	else if ((citizen[citizen_num][0] + 1 == zombie[0]) && (zombie[0] + 1 == madongseok[0])) {
		if (citizen[citizen_num][2] <= madongseok[2]) {
			madongseok[4] = madongseok[4] - 1;
			printf("Zombie attacked madongseok (aggro: %d vs. %d, madongseok stamina: %d -> %d)\n", citizen[citizen_num][2], madongseok[2], madongseok[5], madongseok[4]);
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

	printf("\n");

}

void madongseok_action() {
	if (madongseok[0] - 1 != zombie[0]) {
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

void citizen_counter() {
	if (train_length >= 15 && train_length <= 20) { citizen_count = 2; }
	else if (train_length >= 21 && train_length <= 30) { citizen_count = 3; }
	else if (train_length >= 31 && train_length <= 40) { citizen_count = 4; }
	else if (train_length >= 41 && train_length <= 50) { citizen_count = 5; }

	living_citizen = citizen_count;

	citizen_setting();
}

void citizen_setting() {
	LOCATION_MAX = citizen[0][0] - 1;
	for (int i = 1; i < citizen_count; i++) {

		// 이미 선택된 위치와 겹치지 않도록 무작위로 위치 선택
		do {
			random_location = rand() % LOCATION_MAX + 1;
			check = false; // 기본값은 false로 설정

			// 선택된 위치가 이미 다른 시민에 의해 선택된 경우
			for (int j = 1; j < i; j++) {
				if (valid_location[j] == random_location) {
					check = true;
					break;
				}
			}
		} while (check); // 이미 선택된 위치일 경우 다시 선택

		// 선택된 위치를 기록
		valid_location[i] = random_location;
		// 시민의 위치 설정
		citizen[i][0] = random_location;
	}
}

void stage_one(int citizen_num) {
	citizen[citizen_num][0] = train_length - 6;
	zombie[0] = train_length - 3;
	madongseok[0] = train_length - 2;

	print_train();

	while (1) {

		citizen_move(0);

		zombie_move(0);

		print_train();

		citizen_state(0);

		zombie_state();

		madongseok_move();

		print_train();

		madongseok_state();

		citizen_action(0);

		if (escape == 1) { return; }

		zombie_action(0);

		madongseok_action();

		turn++;
	}
}

void stage_two(int citizen_num) {
	stage = 2;
	escape = 0;

	citizen[citizen_num][0] = train_length - 6;
	villain[0] = train_length - 5;
	zombie[0] = train_length - 3;
	madongseok[0] = train_length - 2;

	print_train();

	while (1) {

		citizen_move(0);

		villain_move(0);

		zombie_move(0);

		print_train();

		citizen_state(0);

		villain_state();

		zombie_state();

		madongseok_move();

		print_train();

		madongseok_state();

		citizen_action(0);

		if (escape == 1) { return; }

		villain_action(0);

		zombie_action(0);

		madongseok_action();

		turn++;

	}
}

void stage_three() {
	stage = 3;
	escape = 0;

	citizen[0][0] = train_length - 6;
	citizen_counter();
	villain[0] = train_length - 5;
	zombie[0] = train_length - 3;
	madongseok[0] = train_length - 2;

	print_train();

	while (1) {

		for (int i = 0; i < citizen_count; i++) {
			citizen_move(i);
		}

		int max_index = -1;
		int max_value = -1;

		for (int i = 0; i < citizen_count; i++) {
			if (citizen[i][0] > max_value) {
				max_value = citizen[i][0];
				max_index = i;
			}
		}

		// max_index가 유효한지 확인 후 zombie_move 호출
		if (max_index != -1) {
			zombie_move(max_index);
		}

		print_train();

		for (int i = 0; i < citizen_count; i++) {
			citizen_state(i);
		}

		zombie_state();

		madongseok_move();

		print_train();

		madongseok_state();

		for (int i = 0; i < citizen_count; i++) {
			citizen_action(i);
		}

		if (escape == 1) { return; }

		max_index = -1;
		max_value = -1;

		for (int i = 0; i < citizen_count; i++) {
			if (citizen[i][0] > max_value) {
				max_value = citizen[i][0];
				max_index = i;
			}
		}

		// max_index가 유효한지 확인 후 zombie_action 호출
		if (max_index != -1) {
			zombie_action(max_index);
		}

		madongseok_action();

		turn++;

	}
}

void stage_four() {
	stage = 3;
	escape = 0;
}

int main() {

	srand((unsigned int)time(NULL));

	train_length = value_input("train_length", LEN_MIN, LEN_MAX);
	madongseok[4] = value_input("madongseok stamina", STM_MIN, STM_MAX);
	percentile_probability = value_input("percentile probability 'p'", PROB_MIN, PROB_MAX);

	/*stage_one(0);
	stage_two(0);*/
	stage_three();
	stage_four();

	return 0;
}