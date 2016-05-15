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

#define CELL_DISTANCE                   11500 //34050 //22700 //11350  //22566 // = 1024 * (40/8) * (18 / (3.1415*1.3))   encoder_units_per_revolution * gear_ratio * (cell_size / wheel_circumference)
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
#define DRIVE_CELL_POSITION_D           0 //0.15f //0.10f //0 seems good

#define DRIVE_CELL_POSITION_DIAGONAL_P  0
#define DRIVE_CELL_POSITION_DIAGONAL_D  0
#define ENCODER_TO_IR_CONVERSION        3

#define LEFT_IR_MIDDLE_VALUE            4.66f//5.365f//4.387f
#define RIGHT_IR_MIDDLE_VALUE           5.75f//5.135f//4.634f
#define IR_OFFSET                       1.09f//-0.147f

//Left Dead Center is 0.82 --> 4.94
//Right Dead Center is 0.78 --> 5.84

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

bool has_wall_front(){
    float irfl = leftFrontIR.readIR();
    //float irfr = rightFrontIR.readIR(); 
    if(irfl < 12) {//|| irfr < 12){
        return true;
    }
    else{
        return false;
    }
}

void drive_cell(){
    //UPDATE_POSITION = false;
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
    float errorD = 0.0f;

    float irl = leftIR.readIR();
    float irr = rightIR.readIR();
    has_left_wall = irl < 12;
    has_right_wall = irr < 12;

    //Seeing both left and right wall
    if(has_left_wall && has_right_wall){
        errorP = irl - irr - IR_OFFSET;
        errorD = errorP - last_position_error;
    }
    
    //only sees left wall and not diagonal
    else if(has_left_wall){
    //    if(!(distance_left > 3700 && distance_left < 7200))
            errorP = (irl - LEFT_IR_MIDDLE_VALUE);
        errorD = errorP - last_position_error;
    }
    
    //only sees right wall and not diagonal
    else if(has_right_wall){
    //    if(!(distance_left > 3700 && distance_left < 7200))
            errorP = (RIGHT_IR_MIDDLE_VALUE - irr);
        
        errorD = errorP - last_position_error;
        
    }
        //no walls, use encoder
    else {
        errorP = 0; //(leftEncoder*1.05 - rightEncoder) * ENCODER_TO_IR_CONVERSION;
        errorD = 0;
    }

    float motorSpeedAdjust = (position_P * errorP) - position_D * (errorD);

    /*if (motorSpeedAdjust < -drive_top_speed / 2){
        motorSpeedAdjust = -drive_top_speed / 2;
    }
    else if (motorSpeedAdjust > drive_top_speed / 2 ){
        motorSpeedAdjust = drive_top_speed / 2;
    }*/
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

void turn_around() {
       //Turn_right requires roughly 4500 ticks from encoder. 
        int currentEnc = ((leftEncoder - rightEncoder) >> 1);
        //pc.printf("currentEnc is %d\r\n", currentEnc);
        if (leftEncoder < -8000)
        {
            leftMotor = 0;    
        }
        
        if (rightEncoder > 8000)
        {
            rightMotor = 0;
        }
        if (leftEncoder < -8000 && rightEncoder > 8000)
        {
            //Stop motors
            stop();  
            break_loop = true;
            return; 
        }
          rightMotor = 0.19695;
          leftMotor = -0.17255;

}

void turn_right() {
       //Turn_right requires roughly 4500 ticks from encoder. 
        int currentEnc = ((leftEncoder - rightEncoder) >> 1);

        if (rightEncoder < -4000)
        {
            rightMotor = 0;
        }
        
        if (leftEncoder > 4000)
        {
            leftMotor = 0;
        }
        
        if (rightEncoder < -4000 && leftEncoder > 4000)
        {
          //  Stop motors
         //   stop();
            break_loop = true;
            return; 
        }
          leftMotor = 0.19695;
          rightMotor = -0.17255;
}



void turn_left() {
       //Turn_left requires roughly 4500 ticks from encoder. 
        int currentEnc = ((leftEncoder - rightEncoder) >> 1);
        if (leftEncoder < -3250)
        {
            leftMotor = 0;    
        }
        
        if (rightEncoder > 3250)
        {
            rightMotor = 0;
        }
        if (leftEncoder < -3250 && rightEncoder > 3250)
        {
            //Stop motors
            //stop();  
            break_loop = true;
            return; 
        }
          rightMotor = 0.18695;
          leftMotor = -0.14255;
}



#endif