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
    int JrkLimit;
}MoveOptions_t;

typedef struct MotorInfo {
    // TODO
}MotorInfo_t;

typedef struct {
    uint64_t Code;
    char *Msg;
}Error;


Error* motorMove(Motor_t m, int32_t units, bool absolute, MoveOptions_t opts);
bool motorMoveIsDone(Motor_t m);
Error* motorMoveCancel(Motor_t m);
Error* motorHalt(Motor_t m);
Error* motorClear(Motor_t m);
Error* motorDisable(Motor_t m);
Error* motorEnable(Motor_t m);
bool motorReady(Motor_t m);
double motorPosition(Motor_t m);



// Error handling
/*
int motorLastErrorCode(Motor_t m);
const char * motorLastErrorMsg(Motor_t m);
void motorClearLastError(Motor_t m);
*/

void motorFreeError(Error *error);


#ifdef __cplusplus
}
#endif // __cplusplus}