#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include "pubSysCls.h"
#include "go_teknic.h"

using namespace sFnd;

#define ACC_LIM_RPM_PER_SEC     100000
#define VEL_LIM_RPM             10
#define NUM_MOVES               5
#define TIME_TILL_TIMEOUT       10000   //The timeout used for homing(ms)

class Motor {
public:
    INode *node;

    Motor(INode *node) {
        this->node = node;
    }

/*
    MotorInfo_t Info() {

    }*/



    void Move(int32_t units, bool absolute, MoveOptions_t opts) {
        //printf("Moving: %d %b\n", units, absolute);
        this->node->Motion.MoveWentDone();                                          //Clear the rising edge Move done register
        this->node->AccUnit(INode::RPM_PER_SEC);                            //Set the units for Acceleration to RPM/SEC
        this->node->VelUnit(INode::RPM);                                            //Set the units for Velocity to RPM
        this->node->TrqUnit(INode::PCT_MAX);

        this->node->Motion.AccLimit = opts.AccLimit;                   //Set Acceleration Limit (RPM/Sec)
        this->node->Motion.VelLimit = opts.VelLimit;                          //Set Velocity Limit (RPM)
        this->node->Limits.TrqGlobal = opts.TrqLimit;
        this->node->Motion.MovePosnStart(units, absolute);                       //Execute 10000 encoder count move
    }

    bool MoveIsDone() {
        return this->node->Motion.MoveIsDone();
    }

    void MoveStop(nodeStopCodes how) {
        this->node->Motion.NodeStop(how);
    }

    void Clear() {
        this->node->Motion.NodeStopClear();
    }

    void Disable() {
        this->node->EnableReq(false);
    }

    void Enable() {
        this->node->EnableReq(true);
    }

    bool IsReady() {
        return this->node->Motion.IsReady();
    }

    double Position() {
        return this->node->Motion.PosnMeasured;
    }
};


class Manager {
public:
    SysManager* sys;
    size_t portCount;
    std::vector<Motor *> nodes;

    Manager() {
        this->portCount = 0;
        this->sys = NULL;
    }

    Motor_t getMotor(unsigned i) {
        if ( i < this->nodes.size() ) {
            return (Motor_t) this->nodes[i];
        }
        return NULL;
    }
};

Manager_t NewManager() {
    size_t portCount;
    std::vector<std::string> comHubPorts;

    SysManager *mgr = SysManager::Instance();
    try {
        SysManager::FindComHubPorts(comHubPorts);
        printf("Found %d SC Hubs\n", comHubPorts.size());

        for (portCount = 0; portCount < comHubPorts.size() && portCount < NET_CONTROLLER_MAX; portCount++) {
            mgr->ComHubPort(portCount, comHubPorts[portCount].c_str());   //define the first SC Hub port (port 0) to be associated
        }

        if (portCount < 0) {
            printf("Unable to locate SC hub port\n");
            return NULL;
        }

        printf("Ports found: %d\n", portCount);
        mgr->PortsOpen(portCount);


        Manager *m = new Manager();
        m->sys = mgr;
        m->portCount = portCount;

        for (size_t i = 0; i < portCount; i++) {
            IPort &myPort = mgr->Ports(i);
            for (size_t iNode = 0; iNode < myPort.NodeCount(); iNode++) {
                INode &theNode = myPort.Nodes(iNode);
                theNode.EnableReq(false);
                theNode.Status.AlertsClear();                                   //Clear Alerts on node
                theNode.Motion.NodeStopClear(); //Clear Nodestops on Node
                theNode.EnableReq(true);                                        //Enable node
                printf("Port: %zi Node: \t%zi enabled\n", i, iNode);
                m->nodes.push_back( new Motor(&theNode));
            }
        }
        return (Manager_t) m;

    } catch(mnErr& theErr) {
        printf("Port Failed to open, Check to ensure correct Port number and that ClearView is not using the Port\n");
        //This statement will print the address of the error, the error code (defined by the mnErr class),
        //as well as the corresponding error message.
        printf("Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
        return NULL;
    }

    return NULL;
}

void mgrClose(Manager_t m) {
    auto mgr = (Manager*) m;


    try {
        printf("Disabling nodes, and closing port\n");
        for (auto iter = mgr->nodes.begin() ; iter != mgr->nodes.end(); ++iter ) {
            Motor *motor = *iter;
            motor->node->EnableReq(false);
        }
    } catch(mnErr& theErr) {
        printf("Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    }
    mgr->sys->PortsClose();

    delete mgr;
}

Motor_t mgrGetMotor(Manager_t m, unsigned i) {
    auto mgr = (Manager*) m;
    return mgr->getMotor(i);
}

void motorMove(Motor_t m, int32_t units, bool absolute, MoveOptions_t opts) {
    ((Motor*)m)->Move(units, absolute, opts);
}

bool motorMoveIsDone(Motor_t m) {
    return ((Motor*)m)->MoveIsDone();
}

void motorHalt(Motor_t m) {
    return ((Motor*)m)->MoveStop(STOP_TYPE_ABRUPT);
}

// Clears any stop conditions
void motorClear(Motor_t m) {
    return ((Motor*)m)->Clear();
}


void motorEnable(Motor_t m) {
    return ((Motor*)m)->Enable();
}

void motorDisable(Motor_t m) {
    return ((Motor*)m)->Disable();
}

bool motorReady(Motor_t m) {
    return ((Motor*)m)->IsReady();
}

double motorPosition(Motor_t m) {
    return ((Motor*)m)->Position();
}

