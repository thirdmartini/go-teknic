#ifdef __cplusplus
extern "C" {
#endif // __cplusplus}

#include <stdbool.h>
#include <stdint.h>

typedef void * Motor_t;
typedef void * Manager_t;


Manager_t NewManager();
void mgrClose(Manager_t m);
Motor_t mgrGetMotor(Manager_t m, unsigned motor);

typedef struct MoveOptions {
    double AccLimit;
    double VelLimit;
    double TrqLimit;
}MoveOptions_t;

void motorMove(Motor_t m, int32_t units, bool absolute, MoveOptions_t opts);
bool motorMoveIsDone(Motor_t m);
void motorHalt(Motor_t m);
void motorClear(Motor_t m);
void motorDisable(Motor_t m);
void motorEnable(Motor_t m);
bool motorReady(Motor_t m);
double motorPosition(Motor_t m);


typedef struct MotorInfo {

}MotorInfo_t;



#ifdef __cplusplus
}
#endif // __cplusplus}