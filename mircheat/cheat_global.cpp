#include "cheat_global.h"

float acceleration = 2.47f;
bool general_speed_change_checkbox = FALSE;

uint32_t original_general_move_speed = 0;
bool general_speed_checkbox = FALSE;
uint32_t general_move_speed = 73;
uint32_t general_move_speed_type = 2;
// 攻击值默认50,越小速度越快
uint32_t original_general_attack_speed = 20;
uint32_t general_attack_speed = 49;
uint32_t original_general_magic_speed = 0;

uint32_t general_magic_speed = 44;
uint32_t original_general_move_interval = 190;
uint32_t general_move_interval = 20;

uint32_t general_attack_interval_type = 2;
uint32_t original_general_attack_interval = 190;
uint32_t general_attack_interval = 20;

uint32_t general_magic_interval_type = 2;
uint32_t original_general_magic_interval = 190;
uint32_t general_magic_interval = 20;
bool fight_fadao_phantom_magic_checkbox = FALSE;

uint32_t *general_super_smooth_type = nullptr;
bool general_super_smooth_checkbox = FALSE;
bool fight_warrior_attack_smooth_checkbox = FALSE;
bool fight_fadao_magic_smooth_checkbox = FALSE;