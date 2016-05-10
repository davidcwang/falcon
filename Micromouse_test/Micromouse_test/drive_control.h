#ifndef DRIVE_CONTROL_H
#define DRIVE_CONTROL_H

#include "pin_assignments.h"

//States from main
//bool DONE_MOVING; //Mouse is driving or turning if true, false otherwise
//bool UPDATE_POSITION; //True when just drove into next cell


//Methods for drive control
void drive_cell();


//Methods for turn control
void turn_left();
void turn_right();
void turn_around();


//Simply sets both motors to speed
void drive(float speed){
    leftMotor = speed;
    rightMotor = speed;
}

//Simply Sets both motors to speed in opposite directions. Turn right is positive.
void turn(float speed){
    leftMotor = speed;
    rightMotor = -speed;
}
//Simply sets both motors to 0
void stop(){
    leftMotor = 0;
    rightMotor = 0;
}

/*
Some rough math
Fastest exploration max_velocity = 23000/sec (2 cells/sec)
Try to keep it at around 11500/sec
 - need measure distance at steady velocity if possible

Each cell is 11550, less than double our mouse size - need actually be very, very slow in general

1/11550 ~= 8.66e-5

Realistic P for exploration: max_speed * 8.66e-5 (exactly 0 at full distance)

Realistic D for exploration:
keep speed < 23000/sec, or 23000*PID_SAMPLE_PERIOD per cycle (like 1150 (half is 575))
Speed should contribute less, so D must be less than max_speed. Lets say at 1150 I want to halve speed and it is at max_speed currently, D = max_speed /2300. But again, speed != velocity, so very rough guesses

say max_speed = 0.1,
P = 8.66e-6, likely more, float cant fit and definitely not frequency accuract
D = 4.35e-5, likely less, float barely fits
So divide final results by 10^6,
P = 8.66
D = 43.5



PID for position:
realistic position off values are -2 to 2
realistic speed adjust is maybe 20% max_speed, 0.02.
P = 0.01 , likely high

realisitic change per second is 2 if actually stable, so each cycle 0.01. Say at 0.01 I want to halve it (0.01/2), so factor is... 1/2?
D = 0.5 , likely high by quite a bit


*/
int print_count = 0;

//TODO!!!!! Add hit wall if should turn just turn.

//TODO!!!!: how is maze alg going to drive back to beginning?


//test fix, or try other fix (0.001f)TODO: its not going fast enough at end of drive if more than like 2 cells

//TODO: make sure center only has 1 entrance.
//TODO: verify_at_center()

//TODO: add lost checking while turning.


//Done. have maze implement it. TODO: avoid double reading ir - changed, now need maze to read given values
//done (most things tested). TODO: check and fix #defines.
//Done. Now make sure other code is changedTODO: maze cant use has_wall(), instead received passed in values
//Done.  Now there will only be one stop per turn (unless maze is slow), and no longer need to be super accurate! //TODO: if keep driving straight, as soon as it is determined, add the distance to the total distance so it doesnt stop. //TODO: carry over distance driven too far/too little. allows for much lower accuracy
//done. TODO: drive straignt time out


#define PID_SAMPLE_PERIOD               0.005//0.01 //every 5 ms

#define CELL_DISTANCE                   12000  //22566 // = 1024 * (40/8) * (18 / (3.1415*1.3))   encoder_units_per_revolution * gear_ratio * (cell_size / wheel_circumference)
#define CELL_DISTANCE_DIAGONAL          CELL_DISTANCE * 1.414214
#define MOUSE_LENGTH                    4300
#define DISTANCE_TO_NEXT_CELL           10600//CELL_DISTANCE/2 + MOUSE_LENGTH/2 //Distance to drive to ensure mouse is fully in next cell
#define DISTANCE_TO_NEXT_CELL_DIAGONAL  19413//CELL_DISTANCE_DIAGONAL/2 + MOUSE_LENGTH/2 //Distance to drive to ensure mouse is fully in next cell

#define DRIVE_CELL_DISTANCE_P           22//8.66 //divided by 10e6
#define DRIVE_CELL_DISTANCE_D           43.5 //divided by 10e6
#define SPEED_DRIVE_CELL_DISTANCE_P     0 //divided by 10e6
#define SPEED_DRIVE_CELL_DISTANCE_D     0 //divided by 10e6
#define SPEED_DRIVE_CELL_DISTANCE_DIAGONAL_P 0 //divided by 10e6
#define SPEED_DRIVE_CELL_DISTANCE_DIAGONAL_D 0 //divided by 10e6
#define DRIVE_CELL_POSITION_P           0.02f //0.05 to 0.1
#define DRIVE_CELL_POSITION_D           0 //0 seems good
#define DRIVE_CELL_POSITION_DIAGONAL_P  0
#define DRIVE_CELL_POSITION_DIAGONAL_D  0
#define ENCODER_TO_IR_CONVERSION        0

#define LEFT_IR_MIDDLE_VALUE            5.365f//4.387f
#define RIGHT_IR_MIDDLE_VALUE           5.135f//4.634f
#define IR_OFFSET                       0.23f//-0.147f


bool not_diagonal;  //Is not driving diagonally
int total_distance; //Total distance to drive
float cell_distance; //Length of 1 cell; differs if diagonal
float distance_P;   //PID constant for distance
float distance_D;   //PID constant for distance
float position_P;   //PID constant for position
float position_D;   //PID constant for position
volatile int next_update_distance;  //Next distance to call update
volatile int last_distance_left;    //
volatile float last_position_error;
int same_count; //If last error == this error for this many cycles of pid, then is stuck right before distance so manually stop it


void drive_cell(){
    //UPDATE_POSITION = false;
    ledGreen = 1;
    // initialize distances for pid
    cell_distance = CELL_DISTANCE;
    distance_P = DRIVE_CELL_DISTANCE_P;
    distance_D = DRIVE_CELL_DISTANCE_D;
    position_P = DRIVE_CELL_POSITION_P;
    position_D = DRIVE_CELL_POSITION_D;
    total_distance = CELL_DISTANCE;

    if (print_count >= 200){
        print_count = 199;
    }
    

    //If update has finished before reaching stopping and the next action is to keep driving, just add more distance and keep driving
    if (UPDATE_FINISHED) {
        UPDATE_FINISHED = false;
        if (mouse_action == DRIVE){
            total_distance += CELL_DISTANCE;
            last_distance_left += CELL_DISTANCE;
            //pc.printf("keep going\r\n");
        }
    }

    int distance_left = total_distance - getEncoderDistance();

    //pc.printf("same_count: %d\r\n", same_count);
    //pc.printf("distance_left: %d\r\n", distance_left);

    
    //If passed distance or encoder remained still for 50ms (too slow to move), then done (no oversteer control)
    if (distance_left <= 50 || same_count >= 5){
        //drive_ticker.detach();

        stop();
        DONE_MOVING = true; //Signal to main

        ledYellow = 1;
        break_loop = true;
        return;
    }

    //P is for distance, D is for velocity. Less distance, lower the speed. Higher the velocity, lower the speed (actually acceleration)
    float motorSpeed = ((distance_left * distance_P) + ((distance_left - last_distance_left) * distance_D)) / 1000000;

    if (!(last_distance_left - distance_left)){
        same_count++;
    }
    else {
        same_count = 0;
    }


    if (motorSpeed > drive_top_speed){
        motorSpeed = drive_top_speed;
    }
    
    //PID for staying in middle
    float errorP = 0.0f;

    float irl = leftIR.readIR();
    float irr = rightIR.readIR();
    has_left_wall = irl < 12;
    has_right_wall = irr < 12;

    //Seeing both left and right wall
    if(has_left_wall && has_right_wall){
        errorP = irl - irr - IR_OFFSET;
    }
        //only sees left wall and not diagonal
    else if(has_left_wall){
        errorP = 0;//(irl - LEFT_IR_MIDDLE_VALUE);// * 2;
    }
        //only sees right wall and not diagonal
    else if(has_right_wall){
        errorP = 0;//(RIGHT_IR_MIDDLE_VALUE - irr);// * 2;
    }
        //no walls, use encoder
    else {
        errorP = 0;//(rightEncoder - leftEncoder) * ENCODER_TO_IR_CONVERSION;
    }

    float motorSpeedAdjust = position_P * errorP - position_D * (errorP - last_position_error);

    if (motorSpeedAdjust < -drive_top_speed / 2){
        motorSpeedAdjust = -drive_top_speed / 2;
    }
    else if (motorSpeedAdjust > drive_top_speed / 2 ){
        motorSpeedAdjust = drive_top_speed / 2;
    }
    //pc.printf("Current motor speed: ", motorSpeed);
    //Set speeds
    ledRed = 1;
    leftMotor = motorSpeed - motorSpeedAdjust;
    rightMotor = motorSpeed + motorSpeedAdjust;

    last_distance_left = distance_left;

    last_position_error = errorP;

    print_count++;


    //If in next cell, update position and start running maze algorithm
    if (leftEncoder > next_update_distance && rightEncoder > next_update_distance){
        next_update_distance += cell_distance;
        UPDATE_POSITION = true; //Signal to main that it is in next cell and update current position
    }
 //   return;
}

//Initializes driving using preset constraints
void _drive_init(){
    DONE_MOVING = false;
    ledGreen = 1;
    same_count = 0;

    //Keeps using last drive encoder over/under values if drive again
    if (!LAST_ACTION_WAS_DRIVE){
        resetEncoders();
        LAST_ACTION_WAS_DRIVE = true;
    }

    next_update_distance = DISTANCE_TO_NEXT_CELL;
    last_distance_left = 0;
    last_position_error = 0;
    return;
}

//Drives one cell, updating current_position and calling update_maze when reached next cell, and calling get_next_move when drove full cell distance
void init_pid_consts(){
    cell_distance = CELL_DISTANCE;
    distance_P = DRIVE_CELL_DISTANCE_P;
    distance_D = DRIVE_CELL_DISTANCE_D;
    position_P = DRIVE_CELL_POSITION_P;
    position_D = DRIVE_CELL_POSITION_D;
    total_distance = CELL_DISTANCE;
    _drive_init();
    return;
}


//Speed run function to drive straight for given number of cells. Updates maze in every cell as usual
void speed_drive_cell(int cells){
    cell_distance = CELL_DISTANCE;
    distance_P = SPEED_DRIVE_CELL_DISTANCE_P;
    distance_D = SPEED_DRIVE_CELL_DISTANCE_D;
    position_P = DRIVE_CELL_POSITION_P;
    position_D = DRIVE_CELL_POSITION_D;
    total_distance = CELL_DISTANCE * cells;
    _drive_init();
}

/*
#define DEGREES_PER_COUNT       0.01875//or 0.019
#define TURN_P_CONSTANT          0.005 //0.005
#define TURN_D_CONSTANT          0//0.001
#define SPEED_TURN_P_CONSTANT          0
#define SPEED_TURN_D_CONSTANT          0
volatile float lastDiffP;
volatile float degrees;
float turn_P_constant;
float turn_D_constant;

// Controls the right turn. Uses the PID and the
// encoder to ensure precision.
void turn_right(){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = 90;
    
    DONE_MOVING = false;
    same_count = 0;
    resetEncoders();
    
    LAST_ACTION_WAS_DRIVE = false;
    
    // PID control code
    float diffP = degrees - 0.01875f  * ((leftEncoder - rightEncoder) >> 1);  //How many more degrees
    
    //If reached there within 5 degrees, stop
    if ((degrees > 0 && diffP < 0) || (degrees < 0 && diffP > 0) || same_count >= 5){
        
        //Updates direction mouse is facing
        current_direction = next_direction;
        //Callback (to drive) if not null

        DONE_MOVING = true;
        stop();
        
        //pc.printf("left_encoder_actual: %d\r\n", leftEncoder);
        //pc.printf("right_encoder_actual: %d\r\n", rightEncoder);
        break_loop = true;
        return;
    }
    
    float motorSpeed = diffP * turn_P_constant - (lastDiffP - diffP) * turn_D_constant;
    if (motorSpeed > turn_top_speed){
        motorSpeed = turn_top_speed;
    }
    else if (motorSpeed < -turn_top_speed){
        motorSpeed = -turn_top_speed;
    }
    turn(motorSpeed); 
    
    if (diffP == lastDiffP){
        same_count++;
    }
    lastDiffP = diffP;
}

// Controls the left turn. Uses the PID and the
// encoder to ensure precision.
void turn_left(){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = -90;
    
    DONE_MOVING = false;
    same_count = 0;
    if (LAST_ACTION_WAS_DRIVE){
        //wait(0.1);
    }
    LAST_ACTION_WAS_DRIVE = false;
    
    // PID control code
    float diffP = degrees - 0.01875f  * ((leftEncoder - rightEncoder) >> 1);  //How many more degrees

    //If reached there within 5 degrees, stop
    if ((degrees > 0 && diffP < 0) || (degrees < 0 && diffP > 0) || same_count >= 5){
        //Updates direction mouse is facing
        current_direction = next_direction;

        DONE_MOVING = true;
        stop();   
        //pc.printf("left_encoder_actual: %d\r\n", leftEncoder);
        //pc.printf("right_encoder_actual: %d\r\n", rightEncoder);
        return;
    }
    float motorSpeed = diffP * turn_P_constant - (lastDiffP - diffP) * turn_D_constant;
    if (motorSpeed > turn_top_speed){
        motorSpeed = turn_top_speed;
    }
    else if (motorSpeed < -turn_top_speed){
        motorSpeed = -turn_top_speed;
    }
    turn(motorSpeed);
    if (diffP == lastDiffP){
        same_count++;
    }
    lastDiffP = diffP;
}

// Controls the 180 turn. Uses the PID and the
// encoder to ensure precision.
*//*
void turn_around(void(*done_callback)(void) = NULL){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = 180;
    _turn_init(done_callback);
} */
#define DEGREES_PER_COUNT       0.01875//or 0.019
#define TURN_P_CONSTANT          0.005 //0.005
#define TURN_D_CONSTANT          0//0.001
#define SPEED_TURN_P_CONSTANT          0
#define SPEED_TURN_D_CONSTANT          0
volatile float lastDiffP;
volatile float degrees;
float turn_P_constant;
float turn_D_constant;
void(*_turn_callback)(void) = NULL;
Ticker drive_ticker;
//Internal turn function, called by turn_right/left/around
void _turn(){
    // PID control code
    float diffP = degrees - 0.01875f  * ((leftEncoder - rightEncoder) >> 1);  //How many more degrees

    //If reached there within 5 degrees, stop
    if ((degrees > 0 && diffP < 0) || (degrees < 0 && diffP > 0) || same_count >= 5){
        drive_ticker.detach();


        //Updates direction mouse is facing
        current_direction = next_direction;
        //Callback (to drive) if not null
        if (_turn_callback){
            stop();
            _turn_callback();
        }
            //Otherwise it can stop moving
        else {
            DONE_MOVING = true;
            stop();
        }

        //pc.printf("left_encoder_actual: %d\r\n", leftEncoder);
        //pc.printf("right_encoder_actual: %d\r\n", rightEncoder);

        return;
    }

    float motorSpeed = diffP * turn_P_constant - (lastDiffP - diffP) * turn_D_constant;
    if (motorSpeed > turn_top_speed){
        motorSpeed = turn_top_speed;
    }
    else if (motorSpeed < -turn_top_speed){
        motorSpeed = -turn_top_speed;
    }
    turn(motorSpeed);

    if (diffP == lastDiffP){
        same_count++;
    }
    lastDiffP = diffP;
}

//Initializes the turn using preset constraints
void _turn_init(void(*done_callback)(void)){
    DONE_MOVING = false;
    same_count = 0;
    resetEncoders();
    if (LAST_ACTION_WAS_DRIVE){
        //wait(0.1);
    }
    LAST_ACTION_WAS_DRIVE = false;

    drive_ticker.attach(&_turn, PID_SAMPLE_PERIOD);
    _turn_callback = done_callback;
}

// Controls the right turn. Uses the PID and the
// encoder to ensure precision.
void turn_right(void(*done_callback)(void) = NULL){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = 90;
    _turn_init(done_callback);
}

// Controls the left turn. Uses the PID and the
// encoder to ensure precision.
void turn_left(void(*done_callback)(void) = NULL){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = -90;
    _turn_init(done_callback);
}

// Controls the 180 turn. Uses the PID and the
// encoder to ensure precision.
void turn_around(void(*done_callback)(void) = NULL){
    turn_P_constant = TURN_P_CONSTANT;
    turn_D_constant = TURN_D_CONSTANT;
    degrees = 180;
    _turn_init(done_callback);
}

#endif