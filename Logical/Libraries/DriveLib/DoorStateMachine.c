
#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif
#include "DriveLib.h"
#ifdef __cplusplus
	};
#endif
/* TODO: Add your comment here */
void DoorStateMachine(struct DoorStateMachine* inst)
{
	// 
	switch (inst->state){
		/*
			Для согласованности индикации, тумблера, отвечающего за направление, и кнопок,
		отвечающих за датчики, направление ворот, в отличие от методички, выбрано так, что
		открытое состояние, когда ворота слева, и закрытое - справа. Т.о. direction == 0
		обозначает направление влево(открытие), а direction == 1 - вправо(закрытие)
		*/
		case ST_INIT:
			inst->speed = 0;
			inst->state=ST_UNKNOWN;
			break;
		
		case ST_UNKNOWN:
			/*
				Из неизвестного состяния мы можем перейти в одно из двух, открыты или закрыты.
			При этом помимо направления, также должен быть задействован соответствующий датчик,
			иначе остаемся в неизвестном состоянии
			*/
			if (inst->sw1 && inst->direction == 0){
				inst->state = ST_OPEN;
				break;
			} else if (inst->sw4 && inst->direction == 1){
				inst->state = ST_CLOSE;
				break;
			} else {
				inst->speed = 0;
				break;
			}
		
		case ST_OPEN:
			/*
				Если ворота открыты, а направление остаётся тем же (влево), то скорость остаётся нулевой, остаёмся в том же состоянии.
			Иначе, если направление теперь другое (вправо) то переход в состояние ACC_POS (узкорение в сторону закрытия (вправо).
			В случае неизвестной комбинации сигналов с датчиков - отправка в состояние UNKNOWN.
			*/
			// Остаемся в ST_OPEN
			if (inst->direction == 0){
				inst->speed = 0;
				break;
			} 
			// Переход в ST_ACC_POS
			else if (inst->direction == 1){
				inst->state = ST_ACC_POS;
				break;
			} 
			// Переход в UNKNOWN
			else {
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_ACC_POS:
			/*
				Если ворота ускоряются, то они остюаются в том же положении пока не сработал второй датчик 
			или не изменилось направление. Когда сработал второй датчик, то устанавливается максимальная 
			скорость и состояние ST_POS. Когда меняется направление, то устанавливается обратная скорость
			и состояние ST_DEC_NEG
			*/
			// Остаемся в ST_ACC_POS
			if (inst->direction == 1 && !inst->sw2){
				inst->speed = 100;	
				break;
			}
			// Сработал второй датчик, переход в ST_POS
			else if (inst->direction == 1 && inst->sw2){
				inst->state = ST_POS;
				break;
			} 
			// Изменилось направление, переход в ST_DEC_NEG, установка обратной скорости
			else if (inst->direction == 0) {
				inst->state = ST_DEC_NEG;
				break;
			} 
			// Неизвестное состояние
			else {
				inst->speed = 0;
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_POS:
			/*
				Состояние максимальной скорости, движение в сторону закрытия. Остаёмся в том же состоянии пока не сработал третий датчик.
			После срабатывания третьего датчика переход в ST_DEC_POS. Если сменится направление то переход в ST_NEG.
			*/
			// То же состояние
			if (inst->direction == 1 && !inst->sw3){
				inst->speed = 200;
				break;
			}
			// Сработал 3 датчик, замедление
			else if (inst->direction == 1){
				inst->state = ST_DEC_POS;
				break;
			}
			// Смена направления
			else if (inst->direction == 0){
				inst->state = ST_NEG;
				break;
			}
			// Неизвестное состояние
			else{
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_DEC_POS:
			/*
				Состояние замедления перед закрытием. При отстутсвии новых сигналов остаётся в том же состоянии. При наличии сигнала с последнего
			датчика переход в состояние ST_CLOSE. При смене направления, переход в состояние ST_ACC_NEG. Иначе ST_UNKNOWN.
			*/
			// Сохранение состояния
			if (inst->direction == 1 && !inst->sw4){
				inst->speed=100;
				break;
			}
			// Переход в закрытое состояние
			else if (inst->direction == 1 && inst->sw4)
			{
				inst->state = ST_CLOSE;
				break;
			}
			// Смена направления
			else if (inst->direction == 0)
			{
				inst->state = ST_ACC_NEG;
				break;
			}
			// Переход в неизвестное состояние
			else{
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_CLOSE:
			if (inst->direction == 1)
			{
				inst->speed = 0;
				break;
			}
			else if(inst->direction == 0){
				inst->state = ST_ACC_NEG;
				break;
			}
			else
			{
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_ACC_NEG:
			/*
				Состояние открывающихся ворот, сохраняет состояние пока не сработает 3 датчик или не изменится направление
			*/
			if (inst->direction == 0 && !inst->sw3){
				inst->speed = -100;
				break;
			}
			else if (inst->sw3){
				inst->state = ST_NEG;
				break;
			}
			else if (inst->direction == 1){
				inst->state = ST_DEC_POS;
				break;
			}
			else {
				inst->state = ST_UNKNOWN;
				break;
			}
		
		case ST_NEG:
			/*Открытие ворот на макс скорости*/
			if (inst->direction == 0 && !inst->sw2){
				inst->speed = -200;
				break;
			}
			else if (inst->direction == 0) {
				inst->state = ST_DEC_NEG;
				break;
			}
			else if (inst->direction == 1){
				inst->state = ST_POS;
				break;
			}
			else{
				inst->state = ST_UNKNOWN;
				break;
			}
		case ST_DEC_NEG:
			if (inst->direction == 0 && !inst->sw1){
				inst->speed = -100;
				break;
			}
			else if(inst->direction == 0){
				inst->state = ST_OPEN;
				break;
			}
			else if (inst->direction == 1){
				inst->state = ST_ACC_POS;
				break;
			}
			else {
				inst->state = ST_UNKNOWN;
				break;
			}
				
	}
}

