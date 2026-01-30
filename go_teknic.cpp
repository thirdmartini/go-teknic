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

// makeError converts an exception into a struct that can be returned as an error type
Error* makeError(mnErr& err) {
    Error *error = (Error *) malloc(sizeof(Error));
    // TODO: panic on malloc failure

    error->Code = err.ErrorCode;
    error->Msg = strdup(err.ErrorMsg);
    return error;
}

class Motor {
public:
    INode *node;

    Motor(INode *node) {
        this->node = node;

        printf("              type: %d\n", node->Info.NodeType());
        printf("            userID: %s\n", node->Info.UserID.Value());
        printf("        FW version: %s\n", node->Info.FirmwareVersion.Value());
        printf("          Serial #: %d\n", node->Info.SerialNumber.Value());
        printf("             Model: %s\n", node->Info.Model.Value());
        printf("TRQ: %d\n", this->node->Limits.TrqGlobal.Value());
        printf("Tracking: %u\n", this->node->Limits.PosnTrackingLimit.Value());
    }

    Error* Move(int32_t units, bool absolute, MoveOptions_t opts) {
        try {
            this->node->Motion.MoveWentDone();                // Clear the rising edge Move done register
            this->node->AccUnit(INode::RPM_PER_SEC);          // Set the units for Acceleration to RPM/SEC
            this->node->VelUnit(INode::RPM);                  // Set the units for Velocity to RPM
            this->node->TrqUnit(INode::PCT_MAX);

            this->node->Motion.AccLimit = opts.AccLimit;      // Set Acceleration Limit (RPM/Sec)
            this->node->Motion.VelLimit = opts.VelLimit;      // Set Velocity Limit (RPM)
            this->node->Motion.JrkLimit = 3;                  // Jerk Limit
            this->node->Motion.DwellMs = 0;                   // Dewll
            this->node->Limits.TrqGlobal = opts.TrqLimit;     // Torque Limit

            this->node->Motion.MovePosnStart(units, absolute);
       } catch(mnErr& theErr) {
            /// printf("Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
            //this->lastErrorCode = theErr.ErrorCode;
            //this->lastErrorMsg = theErr.ErrorMsg;
            return makeError(theErr);
       }

      return NULL;
    }

    void Dump() {
        printf("Trq: %f / %f\n", this->node->Motion.TrqMeasured.Value(), this->node->Motion.TrqCommanded.Value() );
        printf("Pos: %f\n", this->node->Motion.PosnMeasured.Value());
        printf("Vel: %f\n", this->node->Motion.VelMeasured.Value());
    }

    bool MoveIsDone() {
        bool done =  this->node->Motion.MoveIsDone();
        return done;
    }

    Error* MoveStop(nodeStopCodes how) {
        try {
            this->node->Motion.NodeStop(how);
        } catch(mnErr& theErr) {
            return makeError(theErr);
        }
        return NULL;
    }

    Error* Clear() {
        try {
            this->node->Motion.NodeStopClear();
        } catch(mnErr& theErr) {
            return makeError(theErr);
        }
        return NULL;
    }

    Error* Disable() {
        try {
            this->node->EnableReq(false);
        } catch(mnErr& theErr) {
            return makeError(theErr);
        }
        return NULL;
    }

    Error* Enable() {
        try {
            this->node->Status.AlertsClear();                                   //Clear Alerts on node
            this->node->Motion.NodeStopClear(); //Clear Nodestops on Node
            this->node->EnableReq(true);
        } catch(mnErr& theErr) {
            return makeError(theErr);
        }
        return NULL;
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

// ---------------------------------------------------------------------------------
// C++ to C stubs
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
                m->nodes.push_back( new Motor(&theNode));
            }
        }
        return (Manager_t) m;

    } catch(mnErr& theErr) {
        //printf("Port Failed to open, Check to ensure correct Port number and that ClearView is not using the Port\n");
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

Error* motorMove(Motor_t m, int32_t units, bool absolute, MoveOptions_t opts) {
    return ((Motor*)m)->Move(units, absolute, opts);
}

bool motorMoveIsDone(Motor_t m) {
    return ((Motor*)m)->MoveIsDone();
}

Error* motorMoveCancel(Motor_t m) {
    return ((Motor*)m)->MoveStop(STOP_TYPE_RAMP_AT_DECEL);
}

Error* motorHalt(Motor_t m) {
    return ((Motor*)m)->MoveStop(STOP_TYPE_ABRUPT);
}

Error* motorClear(Motor_t m) {
    return ((Motor*)m)->Clear();
}

Error* motorEnable(Motor_t m) {
    return ((Motor*)m)->Enable();
}

Error* motorDisable(Motor_t m) {
    return ((Motor*)m)->Disable();
}

bool motorReady(Motor_t m) {
    return ((Motor*)m)->IsReady();
}

double motorPosition(Motor_t m) {
    return ((Motor*)m)->Position();
}

// motorFreeError must be called to release any error returned by motor api
void motorFreeError(Error *error) {
    if( error != NULL ) {
        free(error->Msg);
        free(error);
    }
}
