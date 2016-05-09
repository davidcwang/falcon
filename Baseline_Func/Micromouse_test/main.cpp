#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NORTH_WEST 4
#define NORTH_EAST 5
#define SOUTH_WEST 6
#define SOUTH_EAST 7
#define IS_AT_BEGINNING 8
#define IS_AT_CENTER 9


//Internal for elsewhere
int next_cell_direction = 0; //TOP
int direction; //maze internal

//Signals from/to elsewhere:
int current_direction = NORTH;
int next_direction = -1; //A direction or AT_BEGINNING or AT_CENTER
int drive_distance = 1; //MAZE MUST SPECIFY DISTANCE TO DRIVE IN SPEED DRIVE MODE

int mouse_x = 0;
int mouse_y = 0;



float drive_top_speed = 0.1;
float turn_top_speed = 0.2;

bool has_left_wall = true;
bool has_right_wall = true;

bool UPDATE_POSITION = false;     //Needs position update when drive into new cell
bool DONE_MOVING = true;              //Driving or turning complete
bool UPDATING_INSTRUCTIONS = true;    //Control alg not yet saved an action to mouse_action
bool UPDATE_FINISHED = false;       //Contorl alg has saved next action to mouse_action. Will always be false unless just finshed updating and next action has not been used yet

bool LAST_ACTION_WAS_DRIVE = false;


#include "drive_control.h"
#include "io.h"
//#include "mouse_control.h"

Serial pc(PA_9, PA_10);  //THIS MUST GO IN MAIN
//Serial pc(D1, D0);


Ticker ticker;
void setup(){
    pc.baud(9600);
    print_battery();
    if (battery.read() < 0.73f){
        //ledRed = 1;
    }
    has_right_wall =  true;
    has_left_wall = true;
    ledYellow = 0;  
}
int main() {
    setup();
    
    while(!user_button){
    }
    wait(2);

    while (1) {
        // Right now, only has the explore mode.
        drive_cell();
        if(DONE_MOVING == true && !has_right_wall){
            turn_right(drive_cell);
        }
        // add another else if here if the front sensor works
        else if (DONE_MOVING == true && !has_left_wall){
            turn_left(drive_cell);
        }
        // else the mouse will keep driving forward
    }
}